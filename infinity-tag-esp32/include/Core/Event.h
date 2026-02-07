#ifndef EVENT_H
#define EVENT_H

#include <Arduino.h>

/**
 * @brief 事件类型枚举
 *
 * 定义系统中所有可能的事件类型
 */
enum EventType {
    EVENT_NONE = 0,              // 无事件
    EVENT_ENCODER_ROTATE,        // 旋转编码器（value: -1=逆时针, 1=顺时针）
    EVENT_BUTTON_PRESS,          // 短按
    EVENT_BUTTON_DOUBLE_CLICK,   // 双击（预留）
    EVENT_BUTTON_TRIPLE_CLICK,   // 三击（触发检查更新）
    EVENT_BUTTON_LONG_PRESS,     // 长按1秒（进入卡片切换模式）
    EVENT_BUTTON_RELEASE,        // 松开按钮
    EVENT_VIBRATION,             // 振动开关触发
    EVENT_NETWORK_READY,         // 网络就绪
    EVENT_NETWORK_DISCONNECTED,  // 网络断开
    EVENT_UPDATE_AVAILABLE,      // 有固件更新可用
    EVENT_CARD_SWITCH_REQUEST    // 请求切换卡片
};

/**
 * @brief 事件结构体
 *
 * 封装事件的所有信息
 */
struct Event {
    EventType type;      // 事件类型
    int value;           // 事件值（如编码器增量 -1/0/1，或其他数据）
    uint32_t timestamp;  // 事件时间戳（毫秒）

    Event() : type(EVENT_NONE), value(0), timestamp(0) {}

    Event(EventType t, int v = 0)
        : type(t), value(v), timestamp(millis()) {}

    Event(EventType t, int v, uint32_t ts)
        : type(t), value(v), timestamp(ts) {}
};

#endif // EVENT_H
