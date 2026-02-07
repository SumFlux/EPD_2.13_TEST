#ifndef CARD_H
#define CARD_H

#include "Event.h"
#include <Arduino.h>

/**
 * @brief 卡片抽象基类
 *
 * 所有卡片（C++卡片和Lua卡片）的基类
 * 提供生命周期管理、事件处理和渲染接口
 */
class Card {
public:
    virtual ~Card() = default;

    // ==========================================
    // 生命周期方法
    // ==========================================

    /**
     * @brief 卡片进入时调用
     *
     * 用于初始化卡片状态、加载资源等
     */
    virtual void onEnter() {}

    /**
     * @brief 卡片退出时调用
     *
     * 用于清理资源、保存状态等
     */
    virtual void onExit() {}

    // ==========================================
    // 事件处理
    // ==========================================

    /**
     * @brief 处理事件
     * @param event 要处理的事件
     */
    virtual void onEvent(const Event& event) = 0;

    // ==========================================
    // 渲染
    // ==========================================

    /**
     * @brief 渲染卡片内容
     *
     * 卡片应该将内容渲染到framebuffer中
     * StatusBar会在之后叠加状态栏图标
     *
     * @param framebuffer 帧缓冲区指针
     * @param size 帧缓冲区大小（字节）
     */
    virtual void render(uint8_t* framebuffer, size_t size) = 0;

    // ==========================================
    // 元数据
    // ==========================================

    /**
     * @brief 获取卡片名称
     * @return 卡片名称（如"黄历"）
     */
    virtual String getName() const = 0;

    /**
     * @brief 获取卡片分类
     * @return 卡片分类（如"决策站"）
     */
    virtual String getCategory() const = 0;

    /**
     * @brief 获取卡片Logo路径
     * @return Logo文件路径（如"/icons/card_calendar.bin"）
     */
    virtual String getLogoPath() const {
        return "/icons/card_default.bin";
    }

    /**
     * @brief 获取卡片显示顺序
     * @return 显示顺序（数字越小越靠前）
     */
    virtual int getOrder() const {
        return 100;
    }

    /**
     * @brief 检查卡片是否启用
     * @return true 启用，false 禁用
     */
    virtual bool isEnabled() const {
        return true;
    }
};

#endif // CARD_H
