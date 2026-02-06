#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "PinConfig.h"
#include <Arduino.h>


class InputManager {
public:
  void begin();

  // Polling methods to be called in loop()
  // Returns the delta change (-1, 0, 1) since last call
  int8_t getEncoderDelta();

  // Returns true if button was pressed since last call
  bool isButtonPressed();

  // Returns true if vibration action (3 shakes/1s) triggered
  bool isVibrationTriggered();

private:
  static void IRAM_ATTR encoderISR();
  static void IRAM_ATTR buttonISR();
  static void IRAM_ATTR vibrationISR();
};

#endif // INPUT_MANAGER_H
