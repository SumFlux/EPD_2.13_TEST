#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

/*
 * IL3897 (B72) 长寿命高画质版
 * ============================
 * 优化策略：
 * 1. [平滑波形] 更长的脉冲时间，更少的脉冲次数
 * 2. [VCOM协同] 添加VCOM驱动，提升对比度稳定性
 * 3. [刷新节流] 最小刷新间隔保护
 * 4. [智能恢复] 更智能的对比度恢复策略
 */

// ==========================================
// 硬件引脚配置
// ==========================================
#define PIN_MOSI 5
#define PIN_SCK 6
#define PIN_CS 7
#define PIN_DC 17
#define PIN_RST 18
#define PIN_BUSY 8

// ==========================================
// 性能参数配置
// ==========================================
#define SPI_FREQUENCY 10000000
#define BUSY_MARGIN_MS 10
#define CONTRAST_RECOVERY_N 20      // 每20次深度恢复（比之前更频繁）
#define FLASH_WHITE_N 3             // 每3次轻度恢复（更频繁）
#define MIN_REFRESH_INTERVAL_MS 100 // 最小刷新间隔，保护屏幕
#define DEMO_LOOP_COUNT 50

// ==========================================
// 刷新区域定义
// ==========================================
struct RefreshZone {
  int16_t x, y, w, h;
};

const RefreshZone ZONE_COUNTER = {80, 40, 60, 50};

// ==========================================
// 驱动实例
// ==========================================
GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT>
    display(GxEPD2_213_B72(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

// ==========================================
// ★ 长寿命高画质 LUT ★
// ==========================================
/*
 * 设计理念：
 * - 使用更长的脉冲时间（0x28=40周期）让粒子充分移动
 * - 只用2个脉冲相位，减少电极应力
 * - 添加VCOM驱动，协同增强对比度
 * - 重复次数适中(1次)，平衡效果和寿命
 */
const uint8_t LUT_LONGEVITY_HQ[] PROGMEM = {
    // LUT0: BB 黑→黑 (轻微加强保持)
    0x00,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,

    // LUT1: BW 黑→白 (双相位平滑变白)
    0x80,
    0x80,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,

    // LUT2: WB 白→黑 ★ 双相位平滑变黑 ★
    0x40,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,

    // LUT3: WW 白→白 (轻微加强保持)
    0x00,
    0x80,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,

    // LUT4: VCOM ★ 协同驱动 ★
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,

    // 时序参数: [TP_A, TP_B, TP_C, TP_D, RP]
    // 关键: 使用更长的脉冲时间(0x28=40周期)，而非更多脉冲
    0x28,
    0x28,
    0x00,
    0x00,
    0x01, // TP0: 40周期×2相位, 重复1次
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP1
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP2
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP3
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP4
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP5
    0x00,
    0x00,
    0x00,
    0x00,
    0x00, // TP6
};

// ★★ 极致画质 LUT（最慢但效果最好）★★
const uint8_t LUT_BEST_QUALITY[] PROGMEM = {
    // LUT0: BB 黑→黑 (加强保持)
    0x00,
    0x40,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00,

    // LUT1: BW 黑→白 (三相位渐进变白)
    0x80,
    0x80,
    0x80,
    0x00,
    0x00,
    0x00,
    0x00,

    // LUT2: WB 白→黑 ★ 三相位渐进变黑 ★
    0x40,
    0x40,
    0x40,
    0x00,
    0x00,
    0x00,
    0x00,

    // LUT3: WW 白→白 (加强保持)
    0x00,
    0x80,
    0x80,
    0x00,
    0x00,
    0x00,
    0x00,

    // LUT4: VCOM (跟随变化)
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,

    // 时序: 更长脉冲(0x30=48周期)，重复2次
    0x30,
    0x30,
    0x30,
    0x00,
    0x02, // TP0: 48周期×3相位, 重复2次
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

// ==========================================
// 全局状态
// ==========================================
static uint16_t g_partialCount = 0;
static unsigned long g_lastRefreshTime = 0;

// 选择当前使用的 LUT（可切换测试）
#define USE_BEST_QUALITY 1 // 1=极致画质, 0=长寿命平衡

// ==========================================
// 底层命令函数
// ==========================================
void writeCommand(uint8_t cmd) {
  digitalWrite(PIN_DC, LOW);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(cmd);
  digitalWrite(PIN_CS, HIGH);
}

void writeData(uint8_t data) {
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(data);
  digitalWrite(PIN_CS, HIGH);
}

void writeDataArray(const uint8_t *data, size_t len) {
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  for (size_t i = 0; i < len; i++) {
    SPI.transfer(pgm_read_byte(&data[i]));
  }
  digitalWrite(PIN_CS, HIGH);
}

void loadCustomLUT() {
  writeCommand(0x32);
#if USE_BEST_QUALITY
  writeDataArray(LUT_BEST_QUALITY, 70);
  Serial.println(">> LUT_BEST_QUALITY loaded");
#else
  writeDataArray(LUT_LONGEVITY_HQ, 70);
  Serial.println(">> LUT_LONGEVITY_HQ loaded");
#endif
}

// 设置优化的驱动电压
void setOptimizedVoltage() {
  // VCOM 电压 (0x2C) - 调整可影响对比度
  writeCommand(0x2C);
  writeData(0x55); // 略微提高VCOM，增强对比度 (原值0x26)

  // Gate 驱动电压 (0x03)
  writeCommand(0x03);
  writeData(0x17); // 20V (原值0x15=19V)

  // Source 驱动电压 (0x04)
  writeCommand(0x04);
  writeData(0x41); // VSH1 15V
  writeData(0xA8); // VSH2 5V
  writeData(0x32); // VSL -15V

  Serial.println(">> Optimized voltage set");
}

void waitBusyWithMargin() {
  while (digitalRead(PIN_BUSY) == HIGH) {
    delayMicroseconds(100);
  }
  delay(BUSY_MARGIN_MS);
}

// ==========================================
// 刷新节流（保护屏幕寿命）
// ==========================================
void enforceRefreshInterval() {
  unsigned long now = millis();
  unsigned long elapsed = now - g_lastRefreshTime;

  if (elapsed < MIN_REFRESH_INTERVAL_MS) {
    delay(MIN_REFRESH_INTERVAL_MS - elapsed);
  }

  g_lastRefreshTime = millis();
}

// ==========================================
// 对比度恢复（温和版）
// ==========================================
void contrastRecovery() {
  Serial.println(">> [Recovery] Gentle contrast recovery...");
  display.setFullWindow();

  // 温和的黑白交替（只1次循环，减少应力）
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
  } while (display.nextPage());
  delay(100); // 更长的稳定时间

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  delay(100);

  // 重新加载 LUT 和电压设置
  setOptimizedVoltage();
  loadCustomLUT();

  Serial.println(">> [Recovery] Done.");
}

// 轻度恢复（只闪白，不做全黑）
void lightRecovery() {
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  waitBusyWithMargin();
}

void checkRecovery() {
  g_partialCount++;

  // 深度恢复（黑白交替）
  if (g_partialCount >= CONTRAST_RECOVERY_N) {
    contrastRecovery();
    g_partialCount = 0;
    return;
  }

  // 轻度恢复（闪白）
  if ((g_partialCount % FLASH_WHITE_N) == 0) {
    lightRecovery();
  }
}

// ==========================================
// 高画质局部刷新
// ==========================================
void hqPartialRefresh(const RefreshZone &zone, int value) {
  // 刷新节流
  enforceRefreshInterval();

  // 检查恢复
  checkRecovery();

  // 设置局部窗口
  display.setPartialWindow(zone.x, zone.y, zone.w, zone.h);

  // 绘制内容
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
// 初始化
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n========================================");
  Serial.println("   IL3897 Longevity + HQ Demo");
  Serial.println("========================================\n");

  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);

  SPI.end();
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);

  display.init(115200, true, 2, false);
  display.epd2.selectSPI(SPI, SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
  display.setRotation(1);

  // 初始全刷清屏
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  delay(300);

  // 设置优化电压和加载自定义LUT
  setOptimizedVoltage();
  loadCustomLUT();

  // 高画质局刷演示
  Serial.println("\n>> High Quality + Longevity Demo...\n");
  Serial.printf("   Min refresh interval: %d ms\n", MIN_REFRESH_INTERVAL_MS);
  Serial.printf("   Light recovery every: %d frames\n", FLASH_WHITE_N);
  Serial.printf("   Deep recovery every:  %d frames\n\n", CONTRAST_RECOVERY_N);

  unsigned long totalStart = millis();

  for (int i = 0; i < DEMO_LOOP_COUNT; i++) {
    unsigned long frameStart = millis();
    hqPartialRefresh(ZONE_COUNTER, i);
    Serial.printf("Frame %02d | Time: %3lu ms\n", i, millis() - frameStart);
  }

  unsigned long totalTime = millis() - totalStart;
  Serial.printf("\n>> Total: %lu ms, Avg: %.1f ms/frame\n", totalTime,
                (float)totalTime / DEMO_LOOP_COUNT);

  display.hibernate();
  Serial.println(">> Done. Display hibernating.");
}

void loop() { delay(10000); }
