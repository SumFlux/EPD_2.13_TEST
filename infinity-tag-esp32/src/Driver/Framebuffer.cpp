#include "Driver/Framebuffer.h"
#include <esp_heap_caps.h>
#include <cstring>

Framebuffer::Framebuffer() : _buffer(nullptr) {
    // 尝试从 PSRAM 分配
    _buffer = (uint8_t*)heap_caps_malloc(BUFFER_SIZE, MALLOC_CAP_SPIRAM);

    if (!_buffer) {
        // PSRAM 分配失败，回退到内部 RAM
        Serial.println("[Framebuffer] PSRAM allocation failed, using internal RAM");
        _buffer = (uint8_t*)malloc(BUFFER_SIZE);
    } else {
        Serial.printf("[Framebuffer] Allocated %d bytes in PSRAM\n", BUFFER_SIZE);
    }

    if (_buffer) {
        clear(WHITE);
    } else {
        Serial.println("[Framebuffer] ERROR: Memory allocation failed!");
    }
}

Framebuffer::~Framebuffer() {
    if (_buffer) {
        free(_buffer);
        _buffer = nullptr;
    }
}

void Framebuffer::setPixel(int16_t x, int16_t y, uint16_t color) {
    if (!isInBounds(x, y) || !_buffer) return;

    size_t byteIndex = getByteIndex(x, y);
    uint8_t bitMask = getBitMask(x);

    if (color == BLACK) {
        _buffer[byteIndex] &= ~bitMask;  // 清除位（黑色）
    } else {
        _buffer[byteIndex] |= bitMask;   // 设置位（白色）
    }
}

uint16_t Framebuffer::getPixel(int16_t x, int16_t y) const {
    if (!isInBounds(x, y) || !_buffer) return WHITE;

    size_t byteIndex = getByteIndex(x, y);
    uint8_t bitMask = getBitMask(x);

    return (_buffer[byteIndex] & bitMask) ? WHITE : BLACK;
}

void Framebuffer::clear(uint16_t color) {
    if (!_buffer) return;

    uint8_t fillValue = (color == BLACK) ? 0x00 : 0xFF;
    memset(_buffer, fillValue, BUFFER_SIZE);
}

void Framebuffer::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    if (!_buffer) return;

    // 裁剪到边界
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > WIDTH) w = WIDTH - x;
    if (y + h > HEIGHT) h = HEIGHT - y;

    if (w <= 0 || h <= 0) return;

    // 逐像素填充（简单实现，可优化）
    for (int16_t dy = 0; dy < h; dy++) {
        for (int16_t dx = 0; dx < w; dx++) {
            setPixel(x + dx, y + dy, color);
        }
    }
}

void Framebuffer::copyRect(int16_t srcX, int16_t srcY, int16_t w, int16_t h,
                           int16_t dstX, int16_t dstY) {
    if (!_buffer) return;

    // 简单实现：逐像素复制
    // TODO: 优化为按字节复制
    for (int16_t dy = 0; dy < h; dy++) {
        for (int16_t dx = 0; dx < w; dx++) {
            int16_t sx = srcX + dx;
            int16_t sy = srcY + dy;
            int16_t tx = dstX + dx;
            int16_t ty = dstY + dy;

            if (isInBounds(sx, sy) && isInBounds(tx, ty)) {
                uint16_t pixel = getPixel(sx, sy);
                setPixel(tx, ty, pixel);
            }
        }
    }
}

void Framebuffer::drawLayer(const Framebuffer& layer, uint8_t alpha) {
    if (!_buffer || !layer._buffer) return;

    if (alpha == 0) return;  // 完全透明，不绘制

    if (alpha == 255) {
        // 完全不透明：直接按位或合成（黑色像素覆盖）
        for (size_t i = 0; i < BUFFER_SIZE; i++) {
            _buffer[i] &= layer._buffer[i];  // AND 操作：黑色（0）覆盖白色（1）
        }
    } else {
        // 半透明：逐像素处理（简单实现）
        for (int16_t y = 0; y < HEIGHT; y++) {
            for (int16_t x = 0; x < WIDTH; x++) {
                uint16_t layerPixel = layer.getPixel(x, y);
                if (layerPixel == BLACK) {
                    // 根据 alpha 决定是否绘制黑色像素
                    if (random(256) < alpha) {
                        setPixel(x, y, BLACK);
                    }
                }
            }
        }
    }
}

void Framebuffer::drawLayerWithMask(const Framebuffer& layer, const Framebuffer& mask) {
    if (!_buffer || !layer._buffer || !mask._buffer) return;

    // 逐像素处理：只在 mask 为 BLACK 的位置绘制
    for (int16_t y = 0; y < HEIGHT; y++) {
        for (int16_t x = 0; x < WIDTH; x++) {
            if (mask.getPixel(x, y) == BLACK) {
                setPixel(x, y, layer.getPixel(x, y));
            }
        }
    }
}

bool Framebuffer::isDifferent(const Framebuffer& other,
                              int16_t& minX, int16_t& minY,
                              int16_t& maxX, int16_t& maxY) const {
    if (!_buffer || !other._buffer) return false;

    // 初始化边界
    minX = WIDTH;
    minY = HEIGHT;
    maxX = -1;
    maxY = -1;

    bool hasDifference = false;

    // 按字节比较（快速检测）
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        if (_buffer[i] != other._buffer[i]) {
            hasDifference = true;

            // 计算该字节对应的像素坐标
            size_t pixelIndex = i * 8;
            int16_t y = pixelIndex / WIDTH;
            int16_t x = pixelIndex % WIDTH;

            // 更新边界
            if (x < minX) minX = x;
            if (y < minY) minY = y;

            // 该字节包含 8 个像素
            int16_t endX = x + 7;
            if (endX >= WIDTH) endX = WIDTH - 1;

            if (endX > maxX) maxX = endX;
            if (y > maxY) maxY = y;
        }
    }

    // 如果有差异，确保边界有效
    if (hasDifference) {
        if (minX < 0) minX = 0;
        if (minY < 0) minY = 0;
        if (maxX >= WIDTH) maxX = WIDTH - 1;
        if (maxY >= HEIGHT) maxY = HEIGHT - 1;
    }

    return hasDifference;
}
