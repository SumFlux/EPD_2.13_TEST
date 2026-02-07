#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include "PinConfig.h"
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>
#include <functional>

// Define the display class type for easier usage
typedef GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> EPD_Class;

/**
 * @brief 绘制回调类型 - 接收 Display 引用
 */
using DrawCallback = std::function<void(EPD_Class &)>;

class EPD_Driver {
public:
  EPD_Driver();
  void begin();

  // ╔════════════════════════════════════════════════════════════════════════╗
  // ║  刷新模式对照表                                                         ║
  // ╠════════════════════════════════════════════════════════════════════════╣
  // ║  模式           │ 刷新区域 │ 闪烁   │ 速度 │ 用途                      ║
  // ╠════════════════════════════════════════════════════════════════════════╣
  // ║  refreshPartial │ 局部     │ 无     │ 快   │ 数字跳变、菜单选择        ║
  // ║  refreshFlicker │ 局部     │ 有(1次)│ 中   │ 每N次局刷后消除残影       ║
  // ║  refreshFull    │ 全屏     │ 有(2次)│ 中慢 │ 切换卡片、场景切换        ║
  // ║  refreshDeep    │ 全屏     │ 有(3次)│ 慢   │ 开机初始化、刷图片、休眠前║
  // ╚════════════════════════════════════════════════════════════════════════╝

  /**
   * @brief 局部刷新 - 极速直出 (参考 FAST)
   * @param drawFunc 绘制回调
   * @param x 刷新区域X坐标 (默认0)
   * @param y 刷新区域Y坐标 (默认18，硬件偏移)
   * @param w 刷新区域宽度 (默认212)
   * @param h 刷新区域高度 (默认104)
   * @note 无闪烁，最快速，适合连续更新
   * @note 每 5 次自动触发一次 refreshFlicker 消除残影
   */
  void refreshPartial(DrawCallback drawFunc, int16_t x = 0, int16_t y = 18,
                      int16_t w = 212, int16_t h = 104);

  /**
   * @brief 快速擦除刷新 - 局部擦白+写入 (参考 FLICKER)
   * @param drawFunc 绘制回调
   * @note 会闪一次（擦白），用于消除一般残影
   * @note 仍使用 setPartialWindow，不触发硬件全刷
   */
  void refreshFlicker(DrawCallback drawFunc);

  /**
   * @brief 全屏刷新 - 黑白清洗 (参考 LIGHT)
   * @param drawFunc 绘制回调
   * @note 闪烁2次（黑→白），适合切换卡片、场景切换
   */
  void refreshFull(DrawCallback drawFunc);

  /**
   * @brief 深度刷新 - 强力清洗 (参考 DEEP)
   * @param drawFunc 绘制回调
   * @note 闪烁3次（白→黑→白），最慢，用于开机初始化、刷图片、休眠前
   */
  void refreshDeep(DrawCallback drawFunc);

  /**
   * @brief 重置局部刷新计数器
   */
  void resetPartialCounter() { _partialRefreshCount = 0; }

  // ==========================================
  // 旧 API (保留兼容性)
  // ==========================================
  void runFast(int num);
  void runFlicker(int num);
  void runLight(int num);
  void runDeep(int num);

  // Power Management
  void hibernate();

  // Draw Info (3 lines, Bottom Left)
  void drawInfo(const char *ver, int enc, int vib, bool useFlicker);

  // Draw static labels once during initialization
  void drawStaticLabels(const char *ver);

  // Draw bitmap data (212x104, 2808 bytes row-aligned)
  void drawBitmap(const uint8_t *bitmap, size_t size, bool useFlicker);

  // Low-level access (仅用于需要直接操作的特殊场景)
  EPD_Class &getDisplay() { return display; }

private:
  EPD_Class display;

  // Private Helpers
  void drawContent(int num, const char *label);

  // ╔════════════════════════════════════════════════════════════════════╗
  // ║  CRITICAL: HARDWARE DISPLAY OFFSETS - MUST USE IN ALL DRAW CALLS  ║
  // ╠════════════════════════════════════════════════════════════════════╣
  // ║  This 2.13" E-Paper panel has a physical offset of 18 pixels.     ║
  // ║  The GRAM is 250x122, but the visible area is only 212x104.       ║
  // ║  Drawing at (0, 0) will render ABOVE the visible screen!          ║
  // ║                                                                    ║
  // ║  ★ ALL drawing functions MUST add OFFSET_Y to Y coordinates ★     ║
  // ║  ★ Example: display.drawBitmap(OFFSET_X, OFFSET_Y, ...)     ★     ║
  // ╚════════════════════════════════════════════════════════════════════╝
  static const int OFFSET_X = 0;  // Horizontal offset (none needed)
  static const int OFFSET_Y = 18; // Vertical offset (MUST USE!)
  static const int VISIBLE_W = 212;
  static const int VISIBLE_H = 104;

  // 刷新策略：每 5 次局刷自动触发一次全刷
  static const int PARTIAL_REFRESH_THRESHOLD = 5;
  int _partialRefreshCount = 0;
};

#endif // EPD_DRIVER_H
