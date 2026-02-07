#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "PinConfig.h"
#include "Core/Event.h"
#include "Core/EventQueue.h"
#include <Arduino.h>

/**
 * @brief 输入管理器
 *
 * 管理旋转编码器、按钮和振动传感器的输入
 * 支持长按检测（1秒）和三击检测
 * 产生Event对象并加入事件队列
 */
class InputManager {
public:
  void begin();

  /**
   * @brief 更新输入状态并产生事件
   *
   * 应该在主循环中调用
   * @param eventQueue 事件队列
   */
  void update(EventQueue& eventQueue);

  // ==========================================
  // 兼容旧接口（保留用于过渡）
  // ==========================================

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

  // 长按检测
  bool _buttonDown;
  uint32_t _buttonDownTime;
  bool _longPressTriggered;
  static const uint32_t LONG_PRESS_DURATION = 1000; // 1秒

  // 三击检测
  uint32_t _lastClickTime;
  uint8_t _clickCount;
  static const uint32_t CLICK_INTERVAL = 500; // 500ms内连续点击

  // 辅助方法
  void _checkLongPress(EventQueue& eventQueue);
  void _checkTripleClick(EventQueue& eventQueue);
};

#endif // INPUT_MANAGER_H
