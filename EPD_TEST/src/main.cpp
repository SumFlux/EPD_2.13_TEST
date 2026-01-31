#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

/*
 * IL3897 (B72) 优化版驱动
 * ========================
 * 优化内容：
 * 1. [智能等待] 动态检测 BUSY + 最小保护延时
 * 2. [SPI提速] 使用 10MHz SPI 时钟
 * 3. [对比度恢复] 定期黑白交替全刷，防止粒子疲劳
 * 4. [结构化代码] 刷新区域定义、可配置参数
 * 5. [性能统计] 输出每帧耗时和平均速度
 *
 * 硬件连接 (ESP32-S3):
 *   MOSI  -> GPIO5
 *   SCLK  -> GPIO6
 *   CS    -> GPIO7
 *   DC    -> GPIO17
 *   RES   -> GPIO18
 *   BUSY  -> GPIO8
 */

// ==========================================
// 1. 硬件引脚配置
// ==========================================
#define PIN_MOSI 5
#define PIN_SCK 6
#define PIN_CS 7
#define PIN_DC 17
#define PIN_RST 18
#define PIN_BUSY 8

// 可选：电源使能引脚（如果硬件支持）
// #define PIN_POWER_EN  47

// ==========================================
// 2. 性能参数配置
// ==========================================
#define SPI_FREQUENCY 10000000 // SPI 频率：10MHz
#define BUSY_MARGIN_MS 10      // BUSY 释放后的最小保护延时
#define CONTRAST_RECOVERY_N 30 // 每 N 次局刷后执行对比度恢复（黑白交替）
#define FLASH_WHITE_N 5        // 每 N 次局刷后闪白一次（保持对比度）
#define DEMO_LOOP_COUNT 50     // 演示循环次数

// ==========================================
// 3. 刷新区域定义
// ==========================================
struct RefreshZone {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

// 预定义刷新区域（可根据实际需求修改）
const RefreshZone ZONE_COUNTER = {80, 40, 60, 50}; // 数字显示区域
const RefreshZone ZONE_STATUS = {10, 10, 200, 30}; // 状态栏区域（备用）

// ==========================================
// 4. 驱动实例
// ==========================================
GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT>
    display(GxEPD2_213_B72(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

// ==========================================
// 5. 全局状态
// ==========================================
static uint16_t g_partialCount = 0; // 局刷计数器

// ==========================================
// 工具函数：智能等待 BUSY 释放
// ==========================================
void waitBusyWithMargin() {
  // 动态轮询 BUSY 引脚（微秒级精度）
  while (digitalRead(PIN_BUSY) == HIGH) {
    delayMicroseconds(100);
  }
  // 额外保护延时，确保电压稳定
  delay(BUSY_MARGIN_MS);
}

// ==========================================
// 工具函数：对比度恢复（黑白交替全刷）
// ==========================================
void contrastRecovery() {
  Serial.println(">> [Recovery] Contrast recovery started...");

  display.setFullWindow();

  // 黑→白→黑→白 交替全刷，彻底激活墨水粒子
  for (int cycle = 0; cycle < 2; cycle++) {
    // 全黑
    display.firstPage();
    do {
      display.fillScreen(GxEPD_BLACK);
    } while (display.nextPage());
    delay(50);

    // 全白
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
    delay(50);
  }

  Serial.println(">> [Recovery] Contrast recovery done.");
}

// ==========================================
// 工具函数：检查并执行对比度恢复
// ==========================================
void checkContrastRecovery() {
  g_partialCount++;

  if (g_partialCount >= CONTRAST_RECOVERY_N) {
    contrastRecovery();
    g_partialCount = 0;
  }
}

// ==========================================
// 核心函数：高速局部刷新（每5次闪白保持对比度）
// ==========================================
void fastPartialRefresh(const RefreshZone &zone, int value) {
  // 检查是否需要对比度恢复（黑白交替全刷）
  checkContrastRecovery();

  // 设置局部刷新窗口
  display.setPartialWindow(zone.x, zone.y, zone.w, zone.h);

  // 每 FLASH_WHITE_N 次执行一次闪白，保持对比度
  if ((g_partialCount % FLASH_WHITE_N) == 0) {
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
    waitBusyWithMargin();
  }

  // 写入新内容
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawRect(zone.x, zone.y, zone.w, zone.h, GxEPD_BLACK);
    display.setCursor(zone.x + 15, zone.y + 10);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(4);
    display.print(value % 10);
  } while (display.nextPage());
  waitBusyWithMargin();
}

// ==========================================
// 初始化函数
// ==========================================
void initDisplay() {
// 可选：电源使能
#ifdef PIN_POWER_EN
  pinMode(PIN_POWER_EN, OUTPUT);
  digitalWrite(PIN_POWER_EN, HIGH);
  delay(100);
#endif

  // SPI 初始化
  SPI.end();
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);

  // 墨水屏初始化
  display.init(115200, true, 2, false);

  // 设置 SPI 速度
  display.epd2.selectSPI(SPI, SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));

  // 设置屏幕方向（横屏）
  display.setRotation(1);

  Serial.printf(">> Display initialized. SPI: %d MHz\n",
                SPI_FREQUENCY / 1000000);
}

// ==========================================
// 初始清屏
// ==========================================
void initialClear() {
  Serial.println(">> Initial clear screen...");

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  delay(300);
  Serial.println(">> Clear done.");
}

// ==========================================
// 主程序
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n========================================");
  Serial.println("   IL3897 (B72) Optimized EPD Demo");
  Serial.println("========================================\n");

  // 初始化显示
  initDisplay();

  // 初始清屏
  initialClear();

  // ==========================================
  // 极速局刷演示
  // ==========================================
  Serial.println("\n>> Turbo Partial Refresh Demo Start...\n");

  unsigned long totalStart = millis();
  unsigned long frameStart;

  for (int i = 0; i < DEMO_LOOP_COUNT; i++) {
    frameStart = millis();

    fastPartialRefresh(ZONE_COUNTER, i);

    unsigned long frameTime = millis() - frameStart;
    Serial.printf("Frame %02d | Value: %d | Time: %3lu ms\n", i, i % 10,
                  frameTime);
  }

  // 性能统计
  unsigned long totalTime = millis() - totalStart;
  float avgTime = (float)totalTime / DEMO_LOOP_COUNT;
  float fps = 1000.0 / avgTime;

  Serial.println("\n========================================");
  Serial.println("   Performance Summary");
  Serial.println("========================================");
  Serial.printf("   Total frames:     %d\n", DEMO_LOOP_COUNT);
  Serial.printf("   Total time:       %lu ms\n", totalTime);
  Serial.printf("   Average:          %.1f ms/frame\n", avgTime);
  Serial.printf("   Effective FPS:    %.2f\n", fps);
  Serial.printf("   Recovery cycles:  %d\n",
                DEMO_LOOP_COUNT / CONTRAST_RECOVERY_N);
  Serial.println("========================================\n");

  // 进入休眠
  Serial.println(">> Done. Entering hibernate mode...");
  display.hibernate();
}

void loop() {
  // 主循环空闲，可添加低功耗逻辑
  delay(10000);
}
