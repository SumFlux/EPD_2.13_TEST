#ifndef CHINESE_FONT_H
#define CHINESE_FONT_H

#include "Fonts/HuiwenFangsong.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>

/**
 * @brief 中文字体渲染工具类
 *
 * 使用匯文仿宋 12px 字体渲染中文字符
 */
class ChineseFont {
public:
  /**
   * @brief 在指定位置绘制中文字符串
   *
   * @param gfx Adafruit_GFX 显示对象
   * @param x X坐标
   * @param y Y坐标（左上角）
   * @param text UTF-8 编码的中文字符串
   * @param color 颜色（GxEPD_BLACK 或 GxEPD_WHITE）
   */
  static void drawString(Adafruit_GFX &gfx, int16_t x, int16_t y,
                         const String &text, uint16_t color) {
    int16_t cursorX = x;
    int16_t cursorY = y;

    const char *str = text.c_str();
    size_t len = text.length();
    size_t i = 0;

    while (i < len) {
      // 检查是否是 UTF-8 多字节字符
      if ((str[i] & 0x80) == 0) {
        // ASCII 字符，使用默认字体 (6x8)，强制大小为 1
        // 中文字符 12px 高，ASCII 8px 高，为了垂直居中，ASCII 向下偏移 2px
        // 使用 drawChar(x, y, c, color, bg, size)，当 bg == color 时背景透明
        gfx.drawChar(cursorX, cursorY + 2, str[i], color, color, 1);

        cursorX += 6; // ASCII 字符宽度 (6px)
        i++;
      } else if ((str[i] & 0xE0) == 0xC0) {
        // 2字节 UTF-8
        char utf8Char[3] = {str[i], str[i + 1], 0};
        drawChineseChar(gfx, cursorX, cursorY, utf8Char, color);
        cursorX += 12; // 中文字符宽度
        i += 2;
      } else if ((str[i] & 0xF0) == 0xE0) {
        // 3字节 UTF-8（中文常用）
        char utf8Char[4] = {str[i], str[i + 1], str[i + 2], 0};
        drawChineseChar(gfx, cursorX, cursorY, utf8Char, color);
        cursorX += 12; // 中文字符宽度
        i += 3;
      } else if ((str[i] & 0xF8) == 0xF0) {
        // 4字节 UTF-8
        char utf8Char[5] = {str[i], str[i + 1], str[i + 2], str[i + 3], 0};
        drawChineseChar(gfx, cursorX, cursorY, utf8Char, color);
        cursorX += 12; // 中文字符宽度
        i += 4;
      } else {
        // 无效字符，跳过
        i++;
      }
    }
  }

  /**
   * @brief 计算中文字符串的显示宽度
   *
   * @param text UTF-8 编码的字符串
   * @return 显示宽度（像素）
   */
  static int16_t getStringWidth(const String &text) {
    int16_t width = 0;
    const char *str = text.c_str();
    size_t len = text.length();
    size_t i = 0;

    while (i < len) {
      if ((str[i] & 0x80) == 0) {
        // ASCII 字符
        width += 6;
        i++;
      } else if ((str[i] & 0xE0) == 0xC0) {
        // 2字节 UTF-8
        width += 12;
        i += 2;
      } else if ((str[i] & 0xF0) == 0xE0) {
        // 3字节 UTF-8
        width += 12;
        i += 3;
      } else if ((str[i] & 0xF8) == 0xF0) {
        // 4字节 UTF-8
        width += 12;
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
   * @param x X坐标
   * @param y Y坐标（左上角）
   * @param utf8Char UTF-8 编码的字符
   * @param color 颜色
   */
  static void drawChineseChar(Adafruit_GFX &gfx, int16_t x, int16_t y,
                              const char *utf8Char, uint16_t color) {
    // 查找字符的位图数据
    const uint8_t *bitmap = findChineseBitmap(utf8Char);

    if (bitmap == nullptr) {
      // 字符不存在，绘制方框
      gfx.drawRect(x, y, 12, 12, color);
      return;
    }

    // 绘制 12x12 位图
    // 格式：每行 2 字节（16位），只用前 12 位，共 12 行 = 24 字节
    for (int dy = 0; dy < 12; dy++) {
      uint16_t rowData = (bitmap[dy * 2] << 8) | bitmap[dy * 2 + 1];

      for (int dx = 0; dx < 12; dx++) {
        // MSB first: bit 15 是最左边的像素
        if (rowData & (0x8000 >> dx)) {
          gfx.drawPixel(x + dx, y + dy, color);
        }
      }
    }
  }
};

#endif // CHINESE_FONT_H
