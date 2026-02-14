#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <Arduino.h>
#include <cstdint>
#include <cstddef>

/**
 * @brief 1位单色 Framebuffer 类
 *
 * 用于 EPD 显存管理，支持像素操作、图层合成、差异检测
 * 内存分配到 PSRAM 以节省内部 RAM
 */
class Framebuffer {
public:
    // 显存尺寸（竖屏）
    static const uint16_t WIDTH = 104;
    static const uint16_t HEIGHT = 212;
    static const size_t BUFFER_SIZE = (WIDTH * HEIGHT + 7) / 8;  // 2756 字节

    // 颜色定义（与 GxEPD 兼容）
    static const uint16_t BLACK = 0x0000;
    static const uint16_t WHITE = 0xFFFF;

    /**
     * @brief 构造函数（PSRAM 分配）
     */
    Framebuffer();

    /**
     * @brief 析构函数
     */
    ~Framebuffer();

    /**
     * @brief 禁用拷贝构造和赋值（避免意外的深拷贝）
     */
    Framebuffer(const Framebuffer&) = delete;
    Framebuffer& operator=(const Framebuffer&) = delete;

    /**
     * @brief 设置像素
     * @param x X 坐标 (0-103)
     * @param y Y 坐标 (0-211)
     * @param color 颜色 (BLACK 或 WHITE)
     */
    void setPixel(int16_t x, int16_t y, uint16_t color);

    /**
     * @brief 获取像素
     * @param x X 坐标 (0-103)
     * @param y Y 坐标 (0-211)
     * @return 颜色 (BLACK 或 WHITE)
     */
    uint16_t getPixel(int16_t x, int16_t y) const;

    /**
     * @brief 清空 framebuffer
     * @param color 填充颜色 (默认 WHITE)
     */
    void clear(uint16_t color = WHITE);

    /**
     * @brief 填充矩形区域
     * @param x 起始 X 坐标
     * @param y 起始 Y 坐标
     * @param w 宽度
     * @param h 高度
     * @param color 填充颜色
     */
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);

    /**
     * @brief 复制矩形区域
     * @param srcX 源 X 坐标
     * @param srcY 源 Y 坐标
     * @param w 宽度
     * @param h 高度
     * @param dstX 目标 X 坐标
     * @param dstY 目标 Y 坐标
     */
    void copyRect(int16_t srcX, int16_t srcY, int16_t w, int16_t h,
                  int16_t dstX, int16_t dstY);

    /**
     * @brief 绘制另一个图层到当前 framebuffer
     * @param layer 源图层
     * @param alpha 透明度 (0-255，当前仅支持 0 或 255)
     */
    void drawLayer(const Framebuffer& layer, uint8_t alpha = 255);

    /**
     * @brief 使用遮罩绘制图层
     * @param layer 源图层
     * @param mask 遮罩（BLACK 像素表示绘制，WHITE 像素表示跳过）
     */
    void drawLayerWithMask(const Framebuffer& layer, const Framebuffer& mask);

    /**
     * @brief 检测与另一个 framebuffer 的差异
     * @param other 对比的 framebuffer
     * @param minX 输出：差异区域最小 X
     * @param minY 输出：差异区域最小 Y
     * @param maxX 输出：差异区域最大 X
     * @param maxY 输出：差异区域最大 Y
     * @return true 如果有差异，false 如果完全相同
     */
    bool isDifferent(const Framebuffer& other,
                     int16_t& minX, int16_t& minY,
                     int16_t& maxX, int16_t& maxY) const;

    /**
     * @brief 获取原始缓冲区（用于传输到 EPD）
     * @return 缓冲区指针
     */
    const uint8_t* getBuffer() const { return _buffer; }
    uint8_t* getBuffer() { return _buffer; }

    /**
     * @brief 检查 framebuffer 是否有效分配
     * @return true 如果缓冲区已分配
     */
    bool isValid() const { return _buffer != nullptr; }

private:
    uint8_t* _buffer;  // PSRAM 分配的缓冲区

    /**
     * @brief 获取字节索引
     */
    inline size_t getByteIndex(int16_t x, int16_t y) const {
        return (y * WIDTH + x) / 8;
    }

    /**
     * @brief 获取位掩码
     */
    inline uint8_t getBitMask(int16_t x) const {
        return 0x80 >> (x & 7);
    }

    /**
     * @brief 边界检查
     */
    inline bool isInBounds(int16_t x, int16_t y) const {
        return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT;
    }
};

#endif // FRAMEBUFFER_H
