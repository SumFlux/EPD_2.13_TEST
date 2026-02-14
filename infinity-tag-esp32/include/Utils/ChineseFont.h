#ifndef CHINESE_FONT_H
#define CHINESE_FONT_H

#include "Fonts/HuiwenFangsong.h"
#include "Driver/Framebuffer.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>

/**
 * @brief 中文字体渲染工具类
 *
 * 使用匯文仿宋 16px 字体渲染中文字符
 *
 * 注意：此类自动处理硬件偏移（OFFSET_Y=18），UI代码使用0-104坐标范围即可
 */
class ChineseFont {
public:
  // 硬件偏移常量（2.13" E-Paper GRAM偏移）
  static const int16_t OFFSET_X = 0;
  static const int16_t OFFSET_Y = 18;

  /**
   * @brief 在指定位置绘制中文字符串（String版本）
   *
   * @param gfx Adafruit_GFX 显示对象
   * @param x X坐标（相对于可见区域0-212）
   * @param y Y坐标（相对于可见区域0-104，自动加上OFFSET_Y=18）
   * @param text UTF-8 编码的中文字符串
   * @param color 颜色（GxEPD_BLACK 或 GxEPD_WHITE）
   */
  static void drawString(Adafruit_GFX &gfx, int16_t x, int16_t y,
                         const String &text, uint16_t color) {
    drawString(gfx, x, y, text.c_str(), color);
  }

  /**
   * @brief 在指定位置绘制中文字符串（const char*版本）
   *
   * @param gfx Adafruit_GFX 显示对象
   * @param x X坐标（相对于可见区域0-212）
   * @param y Y坐标（相对于可见区域0-104，自动加上OFFSET_Y=18）
   * @param text UTF-8 编码的C字符串
   * @param color 颜色（GxEPD_BLACK 或 GxEPD_WHITE）
   */
  static void drawString(Adafruit_GFX &gfx, int16_t x, int16_t y,
                         const char *text, uint16_t color) {
    if (text == nullptr || !text[0]) return;

    // 自动处理硬件偏移
    int16_t cursorX = x + OFFSET_X;
    int16_t cursorY = y + OFFSET_Y;

    const char *str = text;
    size_t len = strlen(text);
    size_t i = 0;

    while (i < len) {
      // 检查是否是 UTF-8 多字节字符
      if ((str[i] & 0x80) == 0) {
        // ASCII 字符，使用半宽（8px）
        char asciiChar[2] = {str[i], 0};
        drawChineseChar(gfx, cursorX, cursorY, asciiChar, color, true);
        cursorX += 8; // ASCII字符半宽8px
        i++;
      } else if ((str[i] & 0xE0) == 0xC0) {
        // 2字节 UTF-8
        char utf8Char[3] = {str[i], str[i + 1], 0};
        drawChineseChar(gfx, cursorX, cursorY, utf8Char, color, false);
        cursorX += 16; // 中文字符全宽16px
        i += 2;
      } else if ((str[i] & 0xF0) == 0xE0) {
        // 3字节 UTF-8（中文常用）
        char utf8Char[4] = {str[i], str[i + 1], str[i + 2], 0};
        drawChineseChar(gfx, cursorX, cursorY, utf8Char, color, false);
        cursorX += 16; // 中文字符全宽16px
        i += 3;
      } else if ((str[i] & 0xF8) == 0xF0) {
        // 4字节 UTF-8
        char utf8Char[5] = {str[i], str[i + 1], str[i + 2], str[i + 3], 0};
        drawChineseChar(gfx, cursorX, cursorY, utf8Char, color, false);
        cursorX += 16; // 中文字符全宽16px
        i += 4;
      } else {
        // 跳过
        i++;
      }
    }
  }

  /**
   * @brief 绘制字符串到 Framebuffer（String版本）
   *
   * @param fb Framebuffer 对象
   * @param x X坐标（相对于可见区域0-212）
   * @param y Y坐标（相对于可见区域0-104，不需要加偏移）
   * @param text UTF-8 编码的字符串
   * @param color 颜色（Framebuffer::BLACK 或 Framebuffer::WHITE）
   */
  static void drawStringToFramebuffer(Framebuffer &fb, int16_t x, int16_t y,
                                      const String &text, uint16_t color) {
    drawStringToFramebuffer(fb, x, y, text.c_str(), color);
  }

  /**
   * @brief 绘制字符串到 Framebuffer（const char*版本）
   *
   * @param fb Framebuffer 对象
   * @param x X坐标（相对于可见区域0-212）
   * @param y Y坐标（相对于可见区域0-104，不需要加偏移）
   * @param text UTF-8 编码的C字符串
   * @param color 颜色（Framebuffer::BLACK 或 Framebuffer::WHITE）
   */
  static void drawStringToFramebuffer(Framebuffer &fb, int16_t x, int16_t y,
                                      const char *text, uint16_t color) {
    if (text == nullptr || !text[0]) return;

    // Framebuffer 不需要硬件偏移（偏移在传输到 EPD 时处理）
    int16_t cursorX = x;
    int16_t cursorY = y;

    const char *str = text;
    size_t len = strlen(text);
    size_t i = 0;

    while (i < len) {
      // 检查是否是 UTF-8 多字节字符
      if ((str[i] & 0x80) == 0) {
        // ASCII 字符，使用半宽（8px）
        char asciiChar[2] = {str[i], 0};
        drawChineseCharToFramebuffer(fb, cursorX, cursorY, asciiChar, color, true);
        cursorX += 8; // ASCII字符半宽8px
        i++;
      } else if ((str[i] & 0xE0) == 0xC0) {
        // 2字节 UTF-8
        char utf8Char[3] = {str[i], str[i + 1], 0};
        drawChineseCharToFramebuffer(fb, cursorX, cursorY, utf8Char, color, false);
        cursorX += 16; // 中文字符全宽16px
        i += 2;
      } else if ((str[i] & 0xF0) == 0xE0) {
        // 3字节 UTF-8（中文常用）
        char utf8Char[4] = {str[i], str[i + 1], str[i + 2], 0};
        drawChineseCharToFramebuffer(fb, cursorX, cursorY, utf8Char, color, false);
        cursorX += 16; // 中文字符全宽16px
        i += 3;
      } else if ((str[i] & 0xF8) == 0xF0) {
        // 4字节 UTF-8
        char utf8Char[5] = {str[i], str[i + 1], str[i + 2], str[i + 3], 0};
        drawChineseCharToFramebuffer(fb, cursorX, cursorY, utf8Char, color, false);
        cursorX += 16; // 中文字符全宽16px
        i += 4;
      } else {
        // 跳过
        i++;
      }
    }
  }

  /**
   * @brief 计算中文字符串的显示宽度（String版本）
   *
   * @param text UTF-8 编码的字符串
   * @return 显示宽度（像素）
   */
  static int16_t getStringWidth(const String &text) {
    return getStringWidth(text.c_str());
  }

  /**
   * @brief 计算中文字符串的显示宽度（const char*版本）
   *
   * @param text UTF-8 编码的C字符串
   * @return 显示宽度（像素）
   */
  static int16_t getStringWidth(const char *text) {
    if (text == nullptr) return 0;

    int16_t width = 0;
    const char *str = text;
    size_t len = strlen(text);
    size_t i = 0;

    while (i < len) {
      if ((str[i] & 0x80) == 0) {
        // ASCII 字符，半宽8px
        width += 8;
        i++;
      } else if ((str[i] & 0xE0) == 0xC0) {
        // 2字节 UTF-8
        width += 16;
        i += 2;
      } else if ((str[i] & 0xF0) == 0xE0) {
        // 3字节 UTF-8
        width += 16;
        i += 3;
      } else if ((str[i] & 0xF8) == 0xF0) {
        // 4字节 UTF-8
        width += 16;
        i += 4;
      } else {
        i++;
      }
    }

    return width;
  }

private:
  /**
   * @brief 绘制单个中文字符
   *
   * @param gfx Adafruit_GFX 显示对象
   * @param x X坐标（已包含硬件偏移）
   * @param y Y坐标（已包含硬件偏移）
   * @param utf8Char UTF-8 编码的字符
   * @param color 颜色
   * @param isASCII 是否为ASCII字符（true=半宽8px，false=全宽16px）
   */
  static void drawChineseChar(Adafruit_GFX &gfx, int16_t x, int16_t y,
                              const char *utf8Char, uint16_t color, bool isASCII = false) {
    // 查找字符的位图数据
    const uint8_t *bitmap = findChineseBitmap(utf8Char);

    if (bitmap == nullptr) {
      // 字符不存在，绘制方框
      if (isASCII) {
        gfx.drawRect(x, y, 8, 16, color);  // ASCII字符方框8x16
      } else {
        gfx.drawRect(x, y, 16, 16, color); // 中文字符方框16x16
      }
      return;
    }

    // 绘制 16x16 位图
    // 格式：每行 2 字节（16位），共 16 行 = 32 字节
    if (isASCII) {
      // ASCII字符：只绘制中间8列（居中显示）
      // 从第4列开始，绘制8列（bit 11 到 bit 4）
      for (int dy = 0; dy < 16; dy++) {
        uint16_t rowData = (bitmap[dy * 2] << 8) | bitmap[dy * 2 + 1];

        for (int dx = 0; dx < 8; dx++) {
          // 从bit 11开始（0x0800），向右移动
          if (rowData & (0x0800 >> dx)) {
            gfx.drawPixel(x + dx, y + dy, color);
          }
        }
      }
    } else {
      // 中文字符：绘制完整16列
      for (int dy = 0; dy < 16; dy++) {
        uint16_t rowData = (bitmap[dy * 2] << 8) | bitmap[dy * 2 + 1];

        for (int dx = 0; dx < 16; dx++) {
          // MSB first: bit 15 是最左边的像素
          if (rowData & (0x8000 >> dx)) {
            gfx.drawPixel(x + dx, y + dy, color);
          }
        }
      }
    }
  }

  /**
   * @brief 绘制单个字符到 Framebuffer
   *
   * @param fb Framebuffer 对象
   * @param x X坐标
   * @param y Y坐标
   * @param utf8Char UTF-8 编码的字符
   * @param color 颜色
   * @param isASCII 是否为ASCII字符（true=半宽8px，false=全宽16px）
   */
  static void drawChineseCharToFramebuffer(Framebuffer &fb, int16_t x, int16_t y,
                                           const char *utf8Char, uint16_t color, bool isASCII = false) {
    // 查找字符的位图数据
    const uint8_t *bitmap = findChineseBitmap(utf8Char);

    if (bitmap == nullptr) {
      // 字符不存在，绘制方框
      int16_t width = isASCII ? 8 : 16;
      // 绘制方框边框
      for (int16_t dx = 0; dx < width; dx++) {
        fb.setPixel(x + dx, y, color);           // 上边
        fb.setPixel(x + dx, y + 15, color);      // 下边
      }
      for (int16_t dy = 0; dy < 16; dy++) {
        fb.setPixel(x, y + dy, color);           // 左边
        fb.setPixel(x + width - 1, y + dy, color); // 右边
      }
      return;
    }

    // 绘制 16x16 位图
    if (isASCII) {
      // ASCII字符：只绘制中间8列（居中显示）
      for (int dy = 0; dy < 16; dy++) {
        uint16_t rowData = (bitmap[dy * 2] << 8) | bitmap[dy * 2 + 1];

        for (int dx = 0; dx < 8; dx++) {
          // 从bit 11开始（0x0800），向右移动
          if (rowData & (0x0800 >> dx)) {
            fb.setPixel(x + dx, y + dy, color);
          }
        }
      }
    } else {
      // 中文字符：绘制完整16列
      for (int dy = 0; dy < 16; dy++) {
        uint16_t rowData = (bitmap[dy * 2] << 8) | bitmap[dy * 2 + 1];

        for (int dx = 0; dx < 16; dx++) {
          // MSB first: bit 15 是最左边的像素
          if (rowData & (0x8000 >> dx)) {
            fb.setPixel(x + dx, y + dy, color);
          }
        }
      }
    }
  }
};

#endif // CHINESE_FONT_H
