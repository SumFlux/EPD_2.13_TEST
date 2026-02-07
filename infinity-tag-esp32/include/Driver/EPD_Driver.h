#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include "PinConfig.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>

// Define the display class type for easier usage
typedef GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> EPD_Class;

class EPD_Driver {
public:
  EPD_Driver();
  void begin();

  // Refresh Modes
  void runFast(int num);
  void runFlicker(int num);
  void runLight(int num);
  void runDeep(int num);

  // Power Management
  void hibernate();

  // Draw Info (3 lines, Bottom Left)
  void drawInfo(const char *ver, int enc, int vib, bool useFlicker);

  // Draw static labels once during initialization
  void drawStaticLabels(const char *ver);

  // Draw bitmap data (212x104, 2808 bytes row-aligned)
  void drawBitmap(const uint8_t *bitmap, size_t size, bool useFlicker);

  // Low-level access if needed (or we can wrap drawing methods)
  EPD_Class &getDisplay() { return display; }

private:
  EPD_Class display;

  // Private Helpers
  void drawContent(int num, const char *label);

  // ╔════════════════════════════════════════════════════════════════════╗
  // ║  CRITICAL: HARDWARE DISPLAY OFFSETS - MUST USE IN ALL DRAW CALLS  ║
  // ╠════════════════════════════════════════════════════════════════════╣
  // ║  This 2.13" E-Paper panel has a physical offset of 18 pixels.     ║
  // ║  The GRAM is 250x122, but the visible area is only 212x104.       ║
  // ║  Drawing at (0, 0) will render ABOVE the visible screen!          ║
  // ║                                                                    ║
  // ║  ★ ALL drawing functions MUST add OFFSET_Y to Y coordinates ★     ║
  // ║  ★ Example: display.drawBitmap(OFFSET_X, OFFSET_Y, ...)     ★     ║
  // ╚════════════════════════════════════════════════════════════════════╝
  static const int OFFSET_X = 0;  // Horizontal offset (none needed)
  static const int OFFSET_Y = 18; // Vertical offset (MUST USE!)
  static const int VISIBLE_W = 212;
  static const int VISIBLE_H = 104;
};

#endif // EPD_DRIVER_H
