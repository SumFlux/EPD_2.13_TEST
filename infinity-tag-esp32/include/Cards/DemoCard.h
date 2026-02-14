#ifndef DEMO_CARD_H
#define DEMO_CARD_H

#include "Core/Card.h"
#include "Driver/EPD_Driver.h"

/**
 * @brief 演示卡片
 *
 * 展示如何使用新的 Framebuffer 系统：
 * - 显示多行中文文字
 * - 使用背景图片
 * - 使用图层系统
 */
class DemoCard : public Card {
public:
    DemoCard();
    ~DemoCard() override = default;

    // 生命周期
    void onEnter() override;
    void onExit() override;

    // 事件处理
    void onEvent(const Event& event) override;

    // 新接口：渲染到图层系统
    void renderToLayers(LayerManager& layerMgr) override;

    // 获取刷新模式
    RefreshMode getRefreshMode() const override;

    // 元数据
    String getName() const override { return "演示卡片"; }
    String getCategory() const override { return "示例"; }
    String getLogoPath() const override { return "/icons/card_demo.bin"; }
    int getOrder() const override { return 1; }

private:
    int _lineIndex;  // 当前显示到第几行
    bool _needsRefresh;  // 是否需要刷新
    bool _useBackground;  // 是否使用背景图片

    // 演示文本（多行）
    static const char* DEMO_TEXTS[];
    static const int DEMO_TEXT_COUNT;
};

#endif // DEMO_CARD_H
