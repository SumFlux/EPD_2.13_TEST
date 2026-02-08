#ifndef LUA_CARD_H
#define LUA_CARD_H

#include "Core/Card.h"
#include "Lua/LuaEngine.h"
#include "Lua/LuaBindings.h"

/**
 * @brief Lua脚本卡片类
 *
 * 功能：
 * - 从LittleFS加载.lua脚本
 * - 读取元数据（CARD_NAME, CARD_CATEGORY, CARD_LOGO, CARD_ORDER）
 * - 事件映射（C++ Event -> Lua回调）
 * - 渲染（调用onLoop，限制10Hz）
 * - 错误处理（显示错误卡片）
 */
class LuaCard : public Card {
public:
  /**
   * @brief 构造函数
   * @param scriptPath Lua脚本路径（如 "/cards/image.lua"）
   * @param epd 墨水屏驱动引用
   */
  LuaCard(const String& scriptPath, EPD_Driver& epd);

  /**
   * @brief 析构函数
   */
  ~LuaCard() override = default;

  /**
   * @brief 卡片进入时调用
   */
  void onEnter() override;

  /**
   * @brief 卡片退出时调用
   */
  void onExit() override;

  /**
   * @brief 处理事件
   */
  void onEvent(const Event& event) override;

  /**
   * @brief 渲染卡片
   */
  void render(uint8_t* framebuffer, size_t size) override;

  /**
   * @brief 获取卡片名称
   */
  String getName() const override { return cardName; }

  /**
   * @brief 获取卡片分类
   */
  String getCategory() const override { return cardCategory; }

  /**
   * @brief 获取卡片图标路径
   */
  String getLogoPath() const override { return cardLogo; }

  /**
   * @brief 获取卡片排序权重
   */
  int getOrder() const override { return cardOrder; }

  /**
   * @brief 检查脚本是否加载成功
   */
  bool isLoaded() const { return loaded; }

  /**
   * @brief 获取错误信息
   */
  String getError() const { return errorMessage; }

private:
  /**
   * @brief 加载脚本并读取元数据
   */
  bool loadScriptAndMetadata();

  /**
   * @brief 渲染错误卡片
   */
  void renderErrorCard(uint8_t* framebuffer, size_t size);

  String scriptPath;
  EPD_Driver& epd;
  LuaEngine& lua;

  // 元数据
  String cardName;
  String cardCategory;
  String cardLogo;
  int cardOrder;
  bool cardEnabled;

  // 状态
  bool loaded;
  String errorMessage;
  uint32_t lastRenderTime;
  static const uint32_t RENDER_INTERVAL_MS = 100;  // 10Hz
};

#endif // LUA_CARD_H
