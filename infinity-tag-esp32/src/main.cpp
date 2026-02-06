#include "Driver/EPD_Driver.h"
#include "Input/InputManager.h"
#include "Version.h"
#include <Arduino.h>


// Objects
EPD_Driver epd;
InputManager input;

// State
int g_encCounter = 0; // 0-999
int g_vibCounter = 0; // 0-99

char g_versionStr[32];

// Refresh Counters
int g_fastCount = 0;
int g_flickerCount = 0;

void updateScreen(bool forceFlicker) {
  // Refresh Strategy Logic
  if (forceFlicker) {
    // 1. Light refresh clear (full screen)
    epd.runLight(0);

    // 2. Redraw info area
    epd.drawInfo(g_versionStr, g_encCounter, g_vibCounter, false);
    return;
  }

  g_fastCount++;

  if (g_fastCount > 5) {
    // Trigger FLICKER Partial
    g_fastCount = 0;
    epd.drawInfo(g_versionStr, g_encCounter, g_vibCounter, true);
  } else {
    // Trigger FAST Partial
    epd.drawInfo(g_versionStr, g_encCounter, g_vibCounter, false);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("--- INFINITY TAG V2 ---");

  // Format Version String
  snprintf(g_versionStr, sizeof(g_versionStr), "Firmware: v%d.%d.%d.%d",
           VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);
  Serial.println(g_versionStr);

  // Init Drivers
  epd.begin();
  input.begin();

  // Initial Clear & Draw static labels
  epd.runLight(0);  // Clear screen
  epd.drawStaticLabels(g_versionStr);  // Draw labels once

  // Draw initial values
  epd.drawInfo(g_versionStr, g_encCounter, g_vibCounter, false);

  Serial.println("--- READY ---");
}

void loop() {
  // 1. Check Input: Encoder
  int8_t delta = input.getEncoderDelta();
  bool vibe = input.isVibrationTriggered();
  bool btn = input.isButtonPressed();

  bool needUpdate = false;

  // 2. Encoder Logic (0-999, Wrap)
  if (delta != 0) {
    g_encCounter += delta;
    if (g_encCounter < 0)
      g_encCounter = 999; // Wrap 0->999
    if (g_encCounter > 999)
      g_encCounter = 0; // Wrap 999->0
    needUpdate = true;
    Serial.printf("[ENC] %d\n", g_encCounter);
  }

  // 3. Vibration Logic (0-99, Wrap, Increment Only)
  // "Only driven by vibration switch, can only add, not subtract"
  if (vibe) {
    g_vibCounter++;
    if (g_vibCounter > 99)
      g_vibCounter = 0; // Wrap 99->0
    needUpdate = true;
    Serial.printf("[VIB] %d\n", g_vibCounter);
  }

  // 4. Update Screen
  if (btn) {
    Serial.println("[BTN] Force Full Refresh");
    g_fastCount = 0; // Reset counters
    updateScreen(true);
  } else if (needUpdate) {
    updateScreen(false);
  }

  delay(10);
}