#include "Driver/EPD_Driver.h"

// Constructor using the pin definitions from PinConfig.h
EPD_Driver::EPD_Driver()
    : display(GxEPD2_213_B72(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY)) {}

void EPD_Driver::begin() {
  // Power enable
  pinMode(PIN_POWER_EN, OUTPUT);
  digitalWrite(PIN_POWER_EN, HIGH);
  delay(50);

  // SPI initialization
  SPI.end();
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);

  // Display initialization
  display.init(115200, true, 2, false);
  display.setRotation(1);
  display.setPartialWindow(0, 0, display.width(), display.height());
}

void EPD_Driver::hibernate() { display.hibernate(); }

// Draw 3 lines of system info: firmware version, encoder count, vibration count
void EPD_Driver::drawInfo(const char *ver, int enc, int vib, bool useFlicker) {
  // Text size 1: each char is 6x8 pixels

  // Line 2: Encoder value area (only refresh the number)
  int16_t enc_label_x = 0;
  int16_t enc_value_x = 54; // After "Encoder: " (9 chars * 6px)
  int16_t enc_y = 102;
  int16_t enc_value_w = 24; // 3 digits max (999) * 6px + padding
  int16_t enc_h = 10;

  // Line 3: Vibration value area (only refresh the number)
  int16_t vib_label_x = 0;
  int16_t vib_value_x = 66; // After "Vibration: " (11 chars * 6px)
  int16_t vib_y = 114;
  int16_t vib_value_w = 18; // 2 digits max (99) * 6px + padding
  int16_t vib_h = 10;

  // Refresh encoder number area
  display.setPartialWindow(enc_value_x, enc_y, enc_value_w, enc_h);

  if (useFlicker) {
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
  }

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
    display.setCursor(enc_value_x, enc_y + 2);
    display.print(enc);
  } while (display.nextPage());

  // Refresh vibration number area
  display.setPartialWindow(vib_value_x, vib_y, vib_value_w, vib_h);

  if (useFlicker) {
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
  }

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
    display.setCursor(vib_value_x, vib_y + 2);
    display.print(vib);
  } while (display.nextPage());
}

// Draw static labels (call once during initialization)
void EPD_Driver::drawStaticLabels(const char *ver) {
  display.setPartialWindow(0, 90, 120, 32);

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);

    // Line 1: Firmware
    display.setCursor(0, 92);
    display.print(ver);

    // Line 2: Encoder label
    display.setCursor(0, 104);
    display.print("Encoder:");

    // Line 3: Vibration label
    display.setCursor(0, 116);
    display.print("Vibration:");
  } while (display.nextPage());
}

// ==========================================
// Refresh Modes
// ==========================================

void EPD_Driver::runFast(int num) {
  display.setPartialWindow(OFFSET_X, OFFSET_Y, VISIBLE_W, VISIBLE_H);
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    drawContent(num, "FAST");
  } while (display.nextPage());
}

// Full refresh: clear screen then draw content
void EPD_Driver::runFlicker(int num) {
  // Full screen wipe
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  // Draw content with placeholder values (legacy compatibility)
  drawInfo("v?.?.?.?", num, 0, false);
}

void EPD_Driver::runLight(int num) {
  // Light refresh: simple full screen clear
  display.setFullWindow();

  // Clear screen to white
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
}

void EPD_Driver::runDeep(int num) {
  // Deep refresh: strong black-white clear only
  display.setFullWindow();

  // Black clear
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
  } while (display.nextPage());

  // White clear
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
}

// ==========================================
// Private Helpers
// ==========================================

void EPD_Driver::drawContent(int num, const char *label) {
  display.drawRect(OFFSET_X, OFFSET_Y, VISIBLE_W, VISIBLE_H, GxEPD_BLACK);

  display.setTextColor(GxEPD_BLACK);
  display.setCursor(OFFSET_X + 5, OFFSET_Y + 15);
  display.setTextSize(2);
  display.print(label);

  display.setCursor(OFFSET_X + 10, OFFSET_Y + 55);
  display.setTextSize(5);
  display.print(num);

  display.fillRect(OFFSET_X, OFFSET_Y + 100, VISIBLE_W, 3, GxEPD_BLACK);
}

// ==========================================
// Bitmap Display
// ==========================================

void EPD_Driver::drawBitmap(const uint8_t *bitmap, size_t size,
                            bool useFlicker) {
  // 验证位图大小
  // 行对齐格式: (212 + 7) / 8 * 104 = 27 * 104 = 2808 字节
  if (size != 2808) {
    Serial.print("Error: Invalid bitmap size: ");
    Serial.print(size);
    Serial.println(" (expected 2808 bytes for row-aligned format)");
    return;
  }

  // 设置全屏窗口
  display.setFullWindow();

  // 如果使用闪烁刷新，先清屏
  if (useFlicker) {
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
  }

  // 绘制位图
  // 注意：
  // 1. 位图中 1=白色, 0=黑色，所以用 GxEPD_WHITE 作为前景色
  // 2. 需要加上 OFFSET_Y=18 的硬件偏移
  // 3. 背景填充黑色，然后用白色画位图中的 "1" 像素
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK); // 背景黑色
    display.drawBitmap(OFFSET_X, OFFSET_Y, bitmap, 212, 104,
                       GxEPD_WHITE); // 前景白色 + 偏移
  } while (display.nextPage());

  Serial.println("Bitmap displayed");
}
