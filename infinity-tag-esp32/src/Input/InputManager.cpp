#include "Input/InputManager.h"
#include "Utils/Logger.h"

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
#define BTN_ISR_DEBOUNCE_US 300000 // 300ms in microseconds
#define VIB_ISR_DEBOUNCE_US 50000  // 50ms in microseconds
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

  // 初始化长按和三击检测
  _buttonDown = false;
  _buttonDownTime = 0;
  _longPressTriggered = false;
  _lastClickTime = 0;
  _clickCount = 0;
}

void InputManager::update(EventQueue &eventQueue) {
  // 1. 检测编码器旋转
  int8_t delta = getEncoderDelta();
  if (delta != 0) {
    Event event(EVENT_ENCODER_ROTATE, delta);
    eventQueue.push(event);
  }

  // 2. 检测按钮状态
  bool btnPressed = isButtonPressed();
  bool btnDown = (digitalRead(PIN_ENC_BTN) == LOW);

  if (btnPressed) {
    // 按钮按下
    _buttonDown = true;
    _buttonDownTime = millis();
    _longPressTriggered = false;

    Event event(EVENT_BUTTON_PRESS);
    eventQueue.push(event);

    // 检测三击
    _checkTripleClick(eventQueue);
  }

  if (_buttonDown && !btnDown) {
    // 按钮松开
    _buttonDown = false;

    Event event(EVENT_BUTTON_RELEASE);
    eventQueue.push(event);
  }

  // 3. 检测长按
  if (_buttonDown && !_longPressTriggered) {
    _checkLongPress(eventQueue);
  }

  // 4. 检测振动
  if (isVibrationTriggered()) {
    Event event(EVENT_VIBRATION);
    eventQueue.push(event);
  }
}

void InputManager::_checkLongPress(EventQueue &eventQueue) {
  if (millis() - _buttonDownTime >= LONG_PRESS_DURATION) {
    _longPressTriggered = true;

    Event event(EVENT_BUTTON_LONG_PRESS);
    eventQueue.push(event);

    LOG_DEBUG("[InputManager] Long press detected");
  }
}

void InputManager::_checkTripleClick(EventQueue &eventQueue) {
  uint32_t now = millis();

  if (now - _lastClickTime < CLICK_INTERVAL) {
    _clickCount++;
    if (_clickCount >= 3) {
      // 三击触发
      Event event(EVENT_BUTTON_TRIPLE_CLICK);
      eventQueue.push(event);

      LOG_DEBUG("[InputManager] Triple click detected");

      _clickCount = 0;
    }
  } else {
    _clickCount = 1;
  }

  _lastClickTime = now;
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
