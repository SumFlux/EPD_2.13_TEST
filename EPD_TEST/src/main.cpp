#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

/*
 * IL3897 (B72) 编码器控制版
 * 版本: V1.0.0.2
 */

// ==========================================
// 版本号定义
// ==========================================
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_BUILD 2

// ==========================================
// 硬件引脚配置
// ==========================================
#define PIN_MOSI 5
#define PIN_SCK 6
#define PIN_CS 7
#define PIN_DC 17
#define PIN_RST 18
#define PIN_BUSY 8

#define PIN_ENC_A 40
#define PIN_ENC_B 39
#define PIN_ENC_BTN 38

// ==========================================
// 性能参数配置
// ==========================================
#define SPI_FREQUENCY 10000000
#define BUSY_MARGIN_MS 10
#define CONTRAST_RECOVERY_N 20
#define FLASH_WHITE_N 3
#define MIN_REFRESH_INTERVAL_MS 50

#define SCREEN_WIDTH 250
#define SCREEN_HEIGHT 122

// ==========================================
// 刷新区域定义
// ==========================================
struct RefreshZone {
  int16_t x, y, w, h;
};

const RefreshZone ZONE_COUNTER = {60, 20, 130, 70};

// ==========================================
// 驱动实例
// ==========================================
GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT>
    display(GxEPD2_213_B72(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

// ==========================================
// 全局状态
// ==========================================
static int16_t g_displayValue = 0;
static uint16_t g_partialCount = 0;
static unsigned long g_lastRefreshTime = 0;
static volatile int8_t g_encoderDelta = 0;
static volatile bool g_buttonPressed = false;
static uint8_t g_lastEncState = 0;
static volatile int8_t g_encoderSteps = 0;

// ==========================================
// 函数前向声明
// ==========================================
void updateDisplay();
void drawVersion();

// ==========================================
// 工具函数
// ==========================================
void waitBusyWithMargin() {
  while (digitalRead(PIN_BUSY) == HIGH) {
    delayMicroseconds(100);
  }
  delay(BUSY_MARGIN_MS);
}

// ==========================================
// 编码器中断处理
// ==========================================
void IRAM_ATTR encoderISR() {
  uint8_t a = digitalRead(PIN_ENC_A);
  uint8_t b = digitalRead(PIN_ENC_B);
  uint8_t state = (a << 1) | b;

  static const int8_t encTable[16] = {0,  -1, 1, 0, 1, 0, 0,  -1,
                                      -1, 0,  0, 1, 0, 1, -1, 0};

  int8_t delta = encTable[(g_lastEncState << 2) | state];
  g_lastEncState = state;
  g_encoderSteps += delta;

  if (g_encoderSteps >= 4) {
    g_encoderDelta--;
    g_encoderSteps = 0;
  } else if (g_encoderSteps <= -4) {
    g_encoderDelta++;
    g_encoderSteps = 0;
  }
}

void IRAM_ATTR buttonISR() {
  static unsigned long lastTime = 0;
  unsigned long now = millis();
  if (now - lastTime < 300)
    return;
  lastTime = now;
  if (digitalRead(PIN_ENC_BTN) == LOW) {
    g_buttonPressed = true;
  }
}

// ==========================================
// 刷新控制
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
// 绘制内容（只绘制，不处理恢复逻辑）
// ==========================================
void drawContent() {
  display.setPartialWindow(ZONE_COUNTER.x, ZONE_COUNTER.y, ZONE_COUNTER.w,
                           ZONE_COUNTER.h);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.drawRect(ZONE_COUNTER.x, ZONE_COUNTER.y, ZONE_COUNTER.w,
                     ZONE_COUNTER.h, GxEPD_BLACK);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(5);
    display.setCursor(ZONE_COUNTER.x + 20, ZONE_COUNTER.y + 15);
    display.printf("%3d", g_displayValue);
  } while (display.nextPage());
  waitBusyWithMargin();
}

// ==========================================
// 绘制版本号
// ==========================================
void drawVersion() {
  display.setPartialWindow(160, 100, 90, 20);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
    display.setCursor(162, 106);
    display.printf("V%d.%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
                   VERSION_BUILD);
  } while (display.nextPage());
  waitBusyWithMargin();
}

// ==========================================
// 轻度恢复（闪白）
// ==========================================
void lightRecovery() {
  display.setPartialWindow(ZONE_COUNTER.x, ZONE_COUNTER.y, ZONE_COUNTER.w,
                           ZONE_COUNTER.h);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  waitBusyWithMargin();
}

// ==========================================
// 深度恢复（单次黑白）
// ==========================================
void contrastRecovery() {
  Serial.println(">> [Recovery] Contrast recovery");

  // 使用局刷方式做全屏黑白（避免触发库的多闪LUT）
  display.setPartialWindow(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
  } while (display.nextPage());
  waitBusyWithMargin();

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  waitBusyWithMargin();

  Serial.println(">> [Recovery] Done");
}

// ==========================================
// 强制全刷（按键触发）
// ==========================================
void forceFullRefresh() {
  Serial.println(">> [Button] Force full refresh!");

  contrastRecovery();
  g_partialCount = 0;

  drawContent();
  drawVersion();

  Serial.printf(">> Full refresh done. Value: %d\n", g_displayValue);
}

// ==========================================
// 检查恢复
// ==========================================
void checkRecovery() {
  g_partialCount++;

  if (g_partialCount >= CONTRAST_RECOVERY_N) {
    contrastRecovery();
    g_partialCount = 0;
    drawContent();
    drawVersion();
    return;
  }

  if ((g_partialCount % FLASH_WHITE_N) == 0) {
    lightRecovery();
  }
}

// ==========================================
// 显示更新
// ==========================================
void updateDisplay() {
  enforceRefreshInterval();
  checkRecovery();
  drawContent();

  Serial.printf(">> Display: %d (partial #%d)\n", g_displayValue,
                g_partialCount);
}

// ==========================================
// 初始化
// ==========================================
void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n========================================");
  Serial.printf("   IL3897 EPD V%d.%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR,
                VERSION_PATCH, VERSION_BUILD);
  Serial.println("========================================\n");

  // 墨水屏引脚
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);

  // 编码器引脚
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_BTN, INPUT_PULLUP);

  g_lastEncState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);

  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BTN), buttonISR, FALLING);

  // SPI 初始化
  SPI.end();
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);

  // 墨水屏初始化
  display.init(115200, true, 2, false);
  display.epd2.selectSPI(SPI, SPISettings(SPI_FREQUENCY, MSBFIRST, SPI_MODE0));
  display.setRotation(1);

  // 初始清屏（使用库的标准方法）
  Serial.println(">> Initial clear...");
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  // 显示初始内容
  drawContent();
  drawVersion();

  Serial.println("\n>> Ready! Rotate encoder to change value.");
  Serial.println(">> Press button for full refresh.\n");
}

// ==========================================
// 主循环
// ==========================================
void loop() {
  static unsigned long lastUpdateTime = 0;

  // 检查按键
  if (g_buttonPressed) {
    g_buttonPressed = false;
    forceFullRefresh();
    lastUpdateTime = millis();
  }

  // 检查编码器
  if (g_encoderDelta != 0) {
    unsigned long now = millis();
    if (now - lastUpdateTime >= 200) {
      noInterrupts();
      int8_t delta = g_encoderDelta;
      g_encoderDelta = 0;
      interrupts();

      g_displayValue += delta;
      if (g_displayValue < 0)
        g_displayValue = 0;
      if (g_displayValue > 999)
        g_displayValue = 999;

      updateDisplay();
      lastUpdateTime = now;
    }
  }

  delay(10);
}
