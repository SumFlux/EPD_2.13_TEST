#ifndef LAYER_H
#define LAYER_H

#include "Driver/Framebuffer.h"
#include <Arduino.h>
#include <memory>
#include <vector>

/**
 * @brief 图层类型枚举
 */
enum class LayerType {
    BACKGROUND,  // 背景层（最底层）
    CONTENT,     // 内容层（图标、图形）
    TEXT,        // 文字层
    STATUSBAR    // 状态栏层（最顶层）
};

/**
 * @brief 图层基类
 *
 * 每个图层拥有独立的 framebuffer，支持分层绘制和合成
 */
class Layer {
public:
    /**
     * @brief 构造函数
     * @param type 图层类型
     * @param zIndex Z轴顺序（数字越大越靠上）
     */
    Layer(LayerType type, uint8_t zIndex = 0);

    /**
     * @brief 虚析构函数
     */
    virtual ~Layer() = default;

    /**
     * @brief 获取图层类型
     */
    LayerType getType() const { return _type; }

    /**
     * @brief 获取 Z 轴顺序
     */
    uint8_t getZIndex() const { return _zIndex; }

    /**
     * @brief 设置 Z 轴顺序
     */
    void setZIndex(uint8_t zIndex) { _zIndex = zIndex; }

    /**
     * @brief 检查图层是否可见
     */
    bool isVisible() const { return _visible; }

    /**
     * @brief 设置图层可见性
     */
    void setVisible(bool visible) { _visible = visible; }

    /**
     * @brief 检查图层是否需要重绘
     */
    bool isDirty() const { return _dirty; }

    /**
     * @brief 标记图层为脏（需要重绘）
     */
    void markDirty() { _dirty = true; }

    /**
     * @brief 清除脏标记
     */
    void clearDirty() { _dirty = false; }

    /**
     * @brief 获取图层 framebuffer
     */
    Framebuffer& getFramebuffer() { return _framebuffer; }
    const Framebuffer& getFramebuffer() const { return _framebuffer; }

    /**
     * @brief 渲染图层内容（子类实现）
     */
    virtual void render() = 0;

protected:
    LayerType _type;
    uint8_t _zIndex;
    bool _visible;
    bool _dirty;
    Framebuffer _framebuffer;
};

/**
 * @brief 图层管理器
 *
 * 管理多个图层，负责排序和合成
 */
class LayerManager {
public:
    LayerManager();
    ~LayerManager();

    /**
     * @brief 添加图层
     */
    void addLayer(std::shared_ptr<Layer> layer);

    /**
     * @brief 移除图层
     */
    void removeLayer(std::shared_ptr<Layer> layer);

    /**
     * @brief 清空所有图层
     */
    void clearLayers();

    /**
     * @brief 合成所有图层到目标 framebuffer
     * @param target 目标 framebuffer
     */
    void composite(Framebuffer& target);

    /**
     * @brief 标记所有图层为脏
     */
    void markAllDirty();

    /**
     * @brief 获取图层数量
     */
    size_t getLayerCount() const { return _layers.size(); }

private:
    std::vector<std::shared_ptr<Layer>> _layers;

    /**
     * @brief 按 zIndex 排序图层
     */
    void sortLayers();
};

// ============================================================================
// 预定义图层类
// ============================================================================

/**
 * @brief 背景图层（纯色或图片）
 */
class BackgroundLayer : public Layer {
public:
    BackgroundLayer(uint16_t color = Framebuffer::WHITE);

    /**
     * @brief 设置背景颜色
     */
    void setColor(uint16_t color);

    /**
     * @brief 设置背景图片
     * @param imageData 图片数据（1位格式）
     * @param w 图片宽度
     * @param h 图片高度
     */
    void setImage(const uint8_t* imageData, uint16_t w, uint16_t h);

    void render() override;

private:
    uint16_t _color;
    const uint8_t* _imageData;
    uint16_t _imageWidth;
    uint16_t _imageHeight;
};

/**
 * @brief 文字图层（使用 ChineseFont）
 */
class TextLayer : public Layer {
public:
    TextLayer();

    /**
     * @brief 添加文字
     * @param x X 坐标
     * @param y Y 坐标
     * @param text 文字内容
     * @param color 颜色
     */
    void addText(int16_t x, int16_t y, const String& text, uint16_t color = Framebuffer::BLACK);

    /**
     * @brief 清空所有文字
     */
    void clearTexts();

    void render() override;

private:
    struct TextItem {
        int16_t x, y;
        String text;
        uint16_t color;
    };
    std::vector<TextItem> _texts;
};

/**
 * @brief 状态栏图层
 */
class StatusBarLayer : public Layer {
public:
    /**
     * @brief 构造函数
     * @param statusBar StatusBar 对象指针
     */
    StatusBarLayer(class StatusBar* statusBar);

    void render() override;

private:
    class StatusBar* _statusBar;
};

#endif // LAYER_H
