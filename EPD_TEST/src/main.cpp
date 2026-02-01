#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

/*
 * IL3897 (B72) 编码器控制版
 * ==========================
 * 功能：
 * - 编码器顺时针旋转：数字 +1
 * - 编码器逆时针旋转：数字 -1
 * - 编码器按下：强制全刷
 * - 保持局刷策略：每N次闪白，每M次深度恢复
 */

// ==========================================
// 硬件引脚配置
// ==========================================
// 墨水屏
#define PIN_MOSI 5
#define PIN_SCK 6
#define PIN_CS 7
#define PIN_DC 17
#define PIN_RST 18
#define PIN_BUSY 8

// 编码器
#define PIN_ENC_A 40   // 编码器 A 相
#define PIN_ENC_B 39   // 编码器 B 相
#define PIN_ENC_BTN 38 // 编码器按键

// ==========================================
// 性能参数配置
// ==========================================
#define SPI_FREQUENCY 10000000
#define BUSY_MARGIN_MS 10
#define CONTRAST_RECOVERY_N 20     // 每20次深度恢复
#define FLASH_WHITE_N 3            // 每3次轻度恢复
#define MIN_REFRESH_INTERVAL_MS 50 // 最小刷新间隔
#define DEBOUNCE_MS 5              // 编码器防抖

// ==========================================
// 刷新区域定义
// ==========================================
struct RefreshZone {
  int16_t x, y, w, h;
};

const RefreshZone ZONE_COUNTER = {60, 30, 130, 70}; // 更大的显示区域

// ==========================================
// 驱动实例
// ==========================================
GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT>
    display(GxEPD2_213_B72(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

// ==========================================
// 极致画质 LUT
// ==========================================
const uint8_t LUT_BEST_QUALITY[] PROGMEM = {
    0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, // LUT0: BB
    0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, // LUT1: BW
    0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, // LUT2: WB
    0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, // LUT3: WW
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // LUT4: VCOM
    0x30, 0x30, 0x30, 0x00, 0x02,             // TP0
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

// ==========================================
// 全局状态
// ==========================================
static int16_t g_displayValue = 0;  // 当前显示数字
static uint16_t g_partialCount = 0; // 局刷计数器
static unsigned long g_lastRefreshTime = 0;
static volatile int8_t g_encoderDelta = 0; // 编码器变化量
static volatile bool g_buttonPressed = false;
static uint8_t g_lastEncState = 0;

// 函数前向声明
void updateDisplay();

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
  writeDataArray(LUT_BEST_QUALITY, 70);
}

void setOptimizedVoltage() {
  writeCommand(0x2C);
  writeData(0x55);
  writeCommand(0x03);
  writeData(0x17);
  writeCommand(0x04);
  writeData(0x41);
  writeData(0xA8);
  writeData(0x32);
}

void waitBusyWithMargin() {
  while (digitalRead(PIN_BUSY) == HIGH) {
    delayMicroseconds(100);
  }
  delay(BUSY_MARGIN_MS);
}

// ==========================================
// 编码器中断处理（增强防抖）
// ==========================================
static volatile int8_t g_encoderSteps = 0; // 累积步数

void IRAM_ATTR encoderISR() {
  uint8_t a = digitalRead(PIN_ENC_A);
  uint8_t b = digitalRead(PIN_ENC_B);
  uint8_t state = (a << 1) | b;

  // 状态机检测旋转方向
  static const int8_t encTable[16] = {0,  -1, 1, 0, 1, 0, 0,  -1,
                                      -1, 0,  0, 1, 0, 1, -1, 0};

  int8_t delta = encTable[(g_lastEncState << 2) | state];
  g_lastEncState = state;

  // 累积步数
  g_encoderSteps += delta;

  // 只有累积到完整脉冲（4步=1格）才触发
  if (g_encoderSteps >= 4) {
    g_encoderDelta--; // 逆时针加（反转）
    g_encoderSteps = 0;
  } else if (g_encoderSteps <= -4) {
    g_encoderDelta++; // 顺时针减（反转）
    g_encoderSteps = 0;
  }
}

void IRAM_ATTR buttonISR() {
  static unsigned long lastTime = 0;
  unsigned long now = millis();

  if (now - lastTime < 200)
    return; // 按键防抖 200ms
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

// 深度恢复（黑白交替）
void contrastRecovery() {
  Serial.println(">> [Recovery] Deep recovery...");
  display.setFullWindow();

  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
  } while (display.nextPage());
  delay(100);

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  delay(100);

  setOptimizedVoltage();
  loadCustomLUT();
  Serial.println(">> [Recovery] Done.");
}

// 轻度恢复（闪白）
void lightRecovery() {
  display.setPartialWindow(ZONE_COUNTER.x, ZONE_COUNTER.y, ZONE_COUNTER.w,
                           ZONE_COUNTER.h);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  waitBusyWithMargin();
}

// 强制全刷（按键触发）
void forceFullRefresh() {
  Serial.println(">> [Button] Force full refresh!");

  display.setFullWindow();

  // 黑白交替清屏
  for (int i = 0; i < 2; i++) {
    display.firstPage();
    do {
      display.fillScreen(GxEPD_BLACK);
    } while (display.nextPage());
    delay(50);
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
    delay(50);
  }

  // 重新显示当前数字
  setOptimizedVoltage();
  loadCustomLUT();
  g_partialCount = 0; // 重置计数器

  // 显示当前值
  updateDisplay();
}

void checkRecovery() {
  g_partialCount++;

  if (g_partialCount >= CONTRAST_RECOVERY_N) {
    contrastRecovery();
    g_partialCount = 0;
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

  display.setPartialWindow(ZONE_COUNTER.x, ZONE_COUNTER.y, ZONE_COUNTER.w,
                           ZONE_COUNTER.h);

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // 绘制边框
    display.drawRect(ZONE_COUNTER.x, ZONE_COUNTER.y, ZONE_COUNTER.w,
                     ZONE_COUNTER.h, GxEPD_BLACK);

    // 显示数字（居中）
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(5);

    // 计算文本位置（简单居中）
    int16_t textX = ZONE_COUNTER.x + 20;
    int16_t textY = ZONE_COUNTER.y + 15;

    display.setCursor(textX, textY);
    display.printf("%3d", g_displayValue);

  } while (display.nextPage());
  waitBusyWithMargin();

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
  Serial.println("   IL3897 Encoder Control Demo");
  Serial.println("========================================\n");

  // 初始化墨水屏引脚
  pinMode(PIN_DC, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);

  // 初始化编码器引脚
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_BTN, INPUT_PULLUP);

  // 读取初始状态
  g_lastEncState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);

  // 注册中断
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

  // 初始全刷清屏
  Serial.println(">> Initial clear...");
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  delay(300);

  // 加载优化设置
  setOptimizedVoltage();
  loadCustomLUT();

  // 显示初始值
  updateDisplay();

  Serial.println("\n>> Ready! Rotate encoder to change value.");
  Serial.println(">> Press encoder button for full refresh.\n");
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

  // 检查编码器旋转（带防抖间隔）
  if (g_encoderDelta != 0) {
    unsigned long now = millis();

    // 防抖：距离上次刷新至少间隔 200ms
    if (now - lastUpdateTime >= 200) {
      noInterrupts();
      int8_t delta = g_encoderDelta;
      g_encoderDelta = 0;
      interrupts();

      g_displayValue += delta;

      // 限制范围 0-999
      if (g_displayValue < 0)
        g_displayValue = 0;
      if (g_displayValue > 999)
        g_displayValue = 999;

      updateDisplay();
      lastUpdateTime = now;
    }
  }

  delay(10); // 降低 CPU 占用
}
