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

  // Low-level access if needed (or we can wrap drawing methods)
  EPD_Class &getDisplay() { return display; }

private:
  EPD_Class display;

  // Private Helpers
  void drawContent(int num, const char *label);

  const int OFFSET_X = 0;
  const int OFFSET_Y = 18;
  const int VISIBLE_W = 212;
  const int VISIBLE_H = 104;
};

#endif // EPD_DRIVER_H
