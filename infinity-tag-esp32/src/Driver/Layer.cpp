#include "Driver/Layer.h"
#include "Core/StatusBar.h"
#include "Utils/ChineseFont.h"
#include <algorithm>

// ============================================================================
// Layer 基类实现
// ============================================================================

Layer::Layer(LayerType type, uint8_t zIndex)
    : _type(type), _zIndex(zIndex), _visible(true), _dirty(true) {
    // Framebuffer 在构造时自动分配
}

// ============================================================================
// LayerManager 实现
// ============================================================================

LayerManager::LayerManager() {
}

LayerManager::~LayerManager() {
    clearLayers();
}

void LayerManager::addLayer(std::shared_ptr<Layer> layer) {
    if (!layer) return;
    _layers.push_back(layer);
    sortLayers();
}

void LayerManager::removeLayer(std::shared_ptr<Layer> layer) {
    if (!layer) return;
    _layers.erase(
        std::remove(_layers.begin(), _layers.end(), layer),
        _layers.end()
    );
}

void LayerManager::clearLayers() {
    _layers.clear();
}

void LayerManager::composite(Framebuffer& target) {
    // 1. 清空目标 framebuffer
    target.clear(Framebuffer::WHITE);

    // 2. 按 zIndex 顺序合成图层（从底到顶）
    for (auto& layer : _layers) {
        if (!layer->isVisible()) continue;

        // 如果图层是脏的，先渲染
        if (layer->isDirty()) {
            layer->render();
            layer->clearDirty();
        }

        // 合成到目标 framebuffer
        target.drawLayer(layer->getFramebuffer());
    }
}

void LayerManager::markAllDirty() {
    for (auto& layer : _layers) {
        layer->markDirty();
    }
}

void LayerManager::sortLayers() {
    std::sort(_layers.begin(), _layers.end(),
        [](const std::shared_ptr<Layer>& a, const std::shared_ptr<Layer>& b) {
            return a->getZIndex() < b->getZIndex();
        }
    );
}

// ============================================================================
// BackgroundLayer 实现
// ============================================================================

BackgroundLayer::BackgroundLayer(uint16_t color)
    : Layer(LayerType::BACKGROUND, 0),
      _color(color),
      _imageData(nullptr),
      _imageWidth(0),
      _imageHeight(0) {
}

void BackgroundLayer::setColor(uint16_t color) {
    if (_color != color) {
        _color = color;
        _imageData = nullptr;  // 清除图片
        markDirty();
    }
}

void BackgroundLayer::setImage(const uint8_t* imageData, uint16_t w, uint16_t h) {
    _imageData = imageData;
    _imageWidth = w;
    _imageHeight = h;
    markDirty();
}

void BackgroundLayer::render() {
    if (_imageData) {
        // 绘制图片（简单实现：直接复制数据）
        // TODO: 支持缩放和居中
        uint8_t* buffer = _framebuffer.getBuffer();
        size_t copySize = std::min(
            (size_t)((_imageWidth * _imageHeight + 7) / 8),
            Framebuffer::BUFFER_SIZE
        );
        memcpy(buffer, _imageData, copySize);
    } else {
        // 纯色背景
        _framebuffer.clear(_color);
    }
}

// ============================================================================
// TextLayer 实现
// ============================================================================

TextLayer::TextLayer() : Layer(LayerType::TEXT, 10) {
}

void TextLayer::addText(int16_t x, int16_t y, const String& text, uint16_t color) {
    _texts.push_back({x, y, text, color});
    markDirty();
}

void TextLayer::clearTexts() {
    _texts.clear();
    markDirty();
}

void TextLayer::render() {
    // 清空图层
    _framebuffer.clear(Framebuffer::WHITE);

    // 绘制所有文字到 framebuffer
    for (const auto& item : _texts) {
        ChineseFont::drawStringToFramebuffer(
            _framebuffer,
            item.x,
            item.y,
            item.text,
            item.color
        );
    }
}

// ============================================================================
// StatusBarLayer 实现
// ============================================================================

StatusBarLayer::StatusBarLayer(StatusBar* statusBar)
    : Layer(LayerType::STATUSBAR, 100),  // 最高 zIndex
      _statusBar(statusBar) {
}

void StatusBarLayer::render() {
    if (!_statusBar) {
        _framebuffer.clear(Framebuffer::WHITE);
        return;
    }

    // 让 StatusBar 渲染到 framebuffer
    _statusBar->renderToFramebuffer(_framebuffer);
}
