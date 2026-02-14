#ifndef CARD_H
#define CARD_H

#include "Event.h"
#include "Driver/Layer.h"
#include "Driver/Framebuffer.h"
#include "Driver/EPD_Driver.h"
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
     * @brief 渲染卡片内容到图层系统（新接口）
     *
     * 卡片应该创建并添加图层到 LayerManager
     * 例如：背景层、内容层、文字层等
     *
     * @param layerMgr 图层管理器
     */
    virtual void renderToLayers(LayerManager& layerMgr) {
        // 默认实现：使用旧的 render() 方法
        // 子类应该重写此方法以使用新的图层系统
        Framebuffer fb;
        render(fb.getBuffer(), Framebuffer::BUFFER_SIZE);

        // 将 framebuffer 包装为图层
        auto contentLayer = std::make_shared<BackgroundLayer>(Framebuffer::WHITE);
        memcpy(contentLayer->getFramebuffer().getBuffer(), fb.getBuffer(), Framebuffer::BUFFER_SIZE);
        layerMgr.addLayer(contentLayer);
    }

    /**
     * @brief 获取刷新模式（新接口）
     *
     * 卡片可以根据当前状态返回合适的刷新模式
     * 例如：菜单选择变化用 PARTIAL，切换场景用 FULL
     *
     * @return 刷新模式
     */
    virtual RefreshMode getRefreshMode() const {
        return RefreshMode::FULL;  // 默认全屏刷新
    }

    /**
     * @brief 渲染卡片内容（旧接口，保留兼容性）
     * @deprecated 请使用 renderToLayers() 代替
     *
     * 卡片应该将内容渲染到framebuffer中
     * StatusBar会在之后叠加状态栏图标
     *
     * @param framebuffer 帧缓冲区指针
     * @param size 帧缓冲区大小（字节）
     */
    virtual void render(uint8_t* framebuffer, size_t size) {
        // 默认实现：清空为白色
        if (framebuffer && size >= Framebuffer::BUFFER_SIZE) {
            memset(framebuffer, 0xFF, Framebuffer::BUFFER_SIZE);
        }
    }

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
