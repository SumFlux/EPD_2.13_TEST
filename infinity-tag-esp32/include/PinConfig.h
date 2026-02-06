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
#define PIN_POWER_EN 47

// Input Devices
#define PIN_ENC_A 40
#define PIN_ENC_B 39
#define PIN_ENC_BTN 38
#define PIN_SW_KEY 48

// Display Constants
#define EPD_WIDTH 212
#define EPD_HEIGHT 104

#endif // PIN_CONFIG_H
