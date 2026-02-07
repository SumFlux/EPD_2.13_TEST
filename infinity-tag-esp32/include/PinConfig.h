#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// ==========================================
// 1. ESP32-S3 Hardware Pins for E-Paper
// ==========================================

// SPI Interface
#define PIN_MOSI 5
#define PIN_SCK 6
#define PIN_CS 7
#define PIN_DC 17
#define PIN_RST 18
#define PIN_BUSY 8

// Power Control
#define PIN_POWER_EN 47 // EPD 电源使能
#define PIN_PWR_IO 47   // 系统电源保持（HOLD POWER）- 必须保持 HIGH

// Input Devices
#define PIN_ENC_A 40
#define PIN_ENC_B 39
#define PIN_ENC_BTN 38
#define PIN_SW_KEY 48

// Display Constants
#define EPD_WIDTH 212
#define EPD_HEIGHT 104

// ══════════════════════════════════════════════════════════════════════════
// IMPORTANT: E-Paper Display Hardware Offset
// ══════════════════════════════════════════════════════════════════════════
// This 2.13" SSD1680 panel has a 250x122 GRAM, but only 212x104 is visible.
// There is an 18-pixel vertical offset from GRAM origin to visible area.
//
// EPD_Driver.h defines: OFFSET_X = 0, OFFSET_Y = 18
//
// ★★★ CRITICAL: When implementing ANY display function, ALWAYS add     ★★★
// ★★★ OFFSET_Y to all Y coordinates! Example:                          ★★★
// ★★★   display.setCursor(x, y + OFFSET_Y);                             ★★★
// ★★★   display.drawBitmap(OFFSET_X, OFFSET_Y, bitmap, w, h, color);    ★★★
// ══════════════════════════════════════════════════════════════════════════

#endif // PIN_CONFIG_H
