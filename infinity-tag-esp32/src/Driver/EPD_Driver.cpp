#include "Driver/EPD_Driver.h"
#include "Utils/Logger.h"

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
// 刷新 API 实现 (参考 il3879epdfullrefresh.txt)
// ==========================================

/**
 * refreshPartial - 极速直出 (对应 FAST)
 * 无闪烁，最快，适合数字跳变、菜单选择
 * 每 5 次自动触发 refreshFlicker 消除残影
 */
void EPD_Driver::refreshPartial(DrawCallback drawFunc, int16_t x, int16_t y,
                                int16_t w, int16_t h) {
  _partialRefreshCount++;

  if (_partialRefreshCount >= PARTIAL_REFRESH_THRESHOLD) {
    LOG_DEBUG("[EPD] Auto flicker (5 partial reached)");
    _partialRefreshCount = 0;

    // 自动切换到 Flicker 模式消除残影
    refreshFlicker(drawFunc);
  } else {
    LOG_PRINTF("[EPD] Partial (%d/%d)\n", _partialRefreshCount,
               PARTIAL_REFRESH_THRESHOLD);

    // 直接局刷
    LOG_DEBUG("[EPD] setPartialWindow start");
    display.setPartialWindow(x, y, w, h);
    LOG_DEBUG("[EPD] firstPage start");
    display.firstPage();
    LOG_DEBUG("[EPD] Page loop start");
    do {
      drawFunc(display);
    } while (display.nextPage());
    LOG_DEBUG("[EPD] Page loop done");
  }
}

/**
 * refreshFlicker - 局部擦白+写入 (对应 FLICKER)
 * 闪1次，消除一般残影，仍使用 setPartialWindow
 */
void EPD_Driver::refreshFlicker(DrawCallback drawFunc) {
  _partialRefreshCount = 0;
  LOG_DEBUG("[EPD] Flicker");

  display.setPartialWindow(OFFSET_X, OFFSET_Y, VISIBLE_W, VISIBLE_H);

  // 1. 擦白 (Wipe)
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  // 2. 写入内容 (Draw)
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    drawFunc(display);
  } while (display.nextPage());
}

/**
 * refreshFull - 全屏黑白清洗 (对应 LIGHT)
 * 闪2次 (黑→白)，适合切换卡片、场景切换
 */
void EPD_Driver::refreshFull(DrawCallback drawFunc) {
  _partialRefreshCount = 0;
  LOG_DEBUG("[EPD] Full");

  // 使用全屏局刷窗口 (不触发硬件全刷命令)
  display.setPartialWindow(0, 0, display.width(), display.height());

  // 1. 闪黑
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
  } while (display.nextPage());

  // 2. 闪白
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  // 3. 绘制内容
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    drawFunc(display);
  } while (display.nextPage());
}

/**
 * refreshDeep - 深度强力清洗 (对应 DEEP)
 * 闪3次 (白→黑→白)，最慢，用于开机初始化、刷图片、休眠前
 */
void EPD_Driver::refreshDeep(DrawCallback drawFunc) {
  _partialRefreshCount = 0;
  LOG_DEBUG("[EPD] Deep");

  display.setPartialWindow(0, 0, display.width(), display.height());

  // 1. 白
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  // 2. 黑
  display.firstPage();
  do {
    display.fillScreen(GxEPD_BLACK);
  } while (display.nextPage());

  // 3. 白
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  // 4. 绘制内容
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    drawFunc(display);
  } while (display.nextPage());

  // 5. 加固一次
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    drawFunc(display);
  } while (display.nextPage());

  // 6. 断电 (保护屏幕)
  display.powerOff();
}

// ==========================================
// 旧 API: 保留兼容性
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
