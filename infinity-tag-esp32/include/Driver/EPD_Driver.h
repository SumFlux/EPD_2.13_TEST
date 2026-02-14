#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include "PinConfig.h"
#include "Driver/Framebuffer.h"
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

/**
 * @brief 刷新模式枚举
 */
enum class RefreshMode {
    PARTIAL,   // 局部刷新（无闪烁）
    FLICKER,   // 局部刷新（闪烁1次，消除残影）
    FULL,      // 全屏刷新（闪烁2次）
    DEEP       // 深度刷新（闪烁3次，最强力）
};

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
   * @param x 刷新区域X坐标 (默认14)
   * @param y 刷新区域Y坐标 (默认34)
   * @param w 刷新区域宽度 (默认104)
   * @param h 刷新区域高度 (默认212)
   * @note 无闪烁，最快速，适合连续更新
   * @note 每 5 次自动触发一次 refreshFlicker 消除残影
   */
  void refreshPartial(DrawCallback drawFunc, int16_t x = 14, int16_t y = 34,
                      int16_t w = 104, int16_t h = 212);

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
  // 新 API: Framebuffer 支持
  // ==========================================

  /**
   * @brief 从 Framebuffer 刷新到屏幕
   * @param fb Framebuffer 对象
   * @param mode 刷新模式
   */
  void refreshFromFramebuffer(const Framebuffer& fb, RefreshMode mode);

  /**
   * @brief 自动差异检测的局部刷新
   * @param oldFb 旧的 framebuffer（当前屏幕内容）
   * @param newFb 新的 framebuffer（要显示的内容）
   */
  void refreshPartialAuto(const Framebuffer& oldFb, const Framebuffer& newFb);

  /**
   * @brief 获取当前屏幕内容的 framebuffer（用于差异对比）
   */
  const Framebuffer& getCurrentFramebuffer() const { return _currentFb; }

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

  // Draw bitmap data (104x212, 2756 bytes row-aligned)
  void drawBitmap(const uint8_t *bitmap, size_t size, bool useFlicker);

  // Low-level access (仅用于需要直接操作的特殊场景)
  EPD_Class &getDisplay() { return display; }

private:
  EPD_Class display;
  Framebuffer _currentFb;  // 记录当前屏幕内容（用于差异对比）

  // Private Helpers
  void drawContent(int num, const char *label);

  /**
   * @brief 传输 Framebuffer 到 EPD 硬件
   * @param fb Framebuffer 对象
   */
  void transferFramebuffer(const Framebuffer& fb);

  // ╔════════════════════════════════════════════════════════════════════╗
  // ║  CRITICAL: HARDWARE DISPLAY OFFSETS - MUST USE IN ALL DRAW CALLS  ║
  // ╠════════════════════════════════════════════════════════════════════╣
  // ║  This 2.13" E-Paper panel has a physical offset.                  ║
  // ║  Based on actual testing with setRotation(2):                     ║
  // ║  - Test block disappeared at X=13, visible at X=14                 ║
  // ║  - Test block disappeared at Y=33, visible at Y=34                 ║
  // ║  - Target visible area: 104x212 pixels                             ║
  // ║                                                                    ║
  // ║  ★ ALL drawing functions MUST add offsets to coordinates ★        ║
  // ╚════════════════════════════════════════════════════════════════════╝
  static const int OFFSET_X = 14;   // 13 + 1 (left boundary test result)
  static const int OFFSET_Y = 34;   // 33 + 1 (top boundary test result)
  static const int VISIBLE_W = 104; // Target width
  static const int VISIBLE_H = 212; // Target height

  // 刷新策略：每 5 次局刷自动触发一次全刷
  static const int PARTIAL_REFRESH_THRESHOLD = 5;
  int _partialRefreshCount = 0;
};

#endif // EPD_DRIVER_H
