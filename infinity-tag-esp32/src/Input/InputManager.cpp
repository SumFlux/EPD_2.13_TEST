#include "Input/InputManager.h"

// ==========================================
// Static State Variables (for ISRs)
// ==========================================
static volatile int8_t _isr_enc_delta = 0;
static volatile uint8_t _isr_enc_last_state = 0;
static volatile int8_t _isr_enc_steps = 0;

static volatile bool _isr_btn_pressed = false;
static unsigned long _isr_btn_last_time = 0;

static volatile bool _isr_vib_triggered = false;
static unsigned long _isr_vib_last_time = 0;

// Configuration (times in microseconds for ISR)
#define BTN_ISR_DEBOUNCE_US 300000  // 300ms in microseconds
#define VIB_ISR_DEBOUNCE_US 50000   // 50ms in microseconds
#define VIB_WINDOW_MS 1000
#define VIB_THRESHOLD_COUNT 3

// ==========================================
// ISR Implementations
// ==========================================

void IRAM_ATTR InputManager::encoderISR() {
  uint8_t a = digitalRead(PIN_ENC_A);
  uint8_t b = digitalRead(PIN_ENC_B);
  uint8_t state = (a << 1) | b;

  // Standard Quadrature Lookup Table
  static const int8_t encTable[16] = {0,  -1, 1, 0, 1, 0, 0,  -1,
                                      -1, 0,  0, 1, 0, 1, -1, 0};

  int8_t delta = encTable[(_isr_enc_last_state << 2) | state];
  _isr_enc_last_state = state;

  // Accumulate 4 steps for 1 detent (optional, depends on hardware)
  // Reference code used 4 steps, adopting that behavior
  _isr_enc_steps += delta;

  if (_isr_enc_steps >= 4) {
    _isr_enc_delta--; // CCW -> -1
    _isr_enc_steps = 0;
  } else if (_isr_enc_steps <= -4) {
    _isr_enc_delta++; // CW -> +1
    _isr_enc_steps = 0;
  }
}

void IRAM_ATTR InputManager::buttonISR() {
  unsigned long now = micros();
  if (now - _isr_btn_last_time < BTN_ISR_DEBOUNCE_US)
    return;
  _isr_btn_last_time = now;

  if (digitalRead(PIN_ENC_BTN) == LOW) {
    _isr_btn_pressed = true;
  }
}

void IRAM_ATTR InputManager::vibrationISR() {
  unsigned long now = micros();
  if (now - _isr_vib_last_time < VIB_ISR_DEBOUNCE_US)
    return;
  _isr_vib_last_time = now;

  _isr_vib_triggered = true;
}

// ==========================================
// Class Implementation
// ==========================================

void InputManager::begin() {
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_BTN, INPUT_PULLUP);
  pinMode(PIN_SW_KEY, INPUT_PULLUP);

  // Initial state
  _isr_enc_last_state = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);

  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_BTN), buttonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(PIN_SW_KEY), vibrationISR, FALLING);
}

int8_t InputManager::getEncoderDelta() {
  int8_t val = 0;
  if (_isr_enc_delta != 0) {
    noInterrupts();
    val = _isr_enc_delta;
    _isr_enc_delta = 0;
    interrupts();
  }
  return val;
}

bool InputManager::isButtonPressed() {
  if (_isr_btn_pressed) {
    _isr_btn_pressed = false;
    return true;
  }
  return false;
}

bool InputManager::isVibrationTriggered() {
  // 1. Check raw trigger from ISR
  bool raw_trigger = false;
  if (_isr_vib_triggered) {
    _isr_vib_triggered = false;
    raw_trigger = true;
  }

  if (!raw_trigger)
    return false;

  // 2. Logic: 3 triggers in 1 second
  static unsigned long win_start = 0;
  static int count = 0;
  unsigned long now = millis();

  // Check window expiration
  if (count > 0 && (now - win_start > VIB_WINDOW_MS)) {
    count = 0; // Reset if too slow
  }

  if (count == 0) {
    win_start = now;
    count = 1;
  } else {
    count++;
  }

  if (count >= VIB_THRESHOLD_COUNT) {
    count = 0; // Reset after successful action
    return true;
  }

  return false;
}
