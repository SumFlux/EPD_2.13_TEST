#ifndef LUA_BINDINGS_H
#define LUA_BINDINGS_H

#include "Core/ConfigManager.h"
#include "Driver/EPD_Driver.h"
#include "Lua/LuaEngine.h"

#include <Adafruit_GFX.h>

/**
 * @brief Lua API绑定类
 *
 * 提供以下API命名空间：
 * - eink.*  : 墨水屏绘图API
 * - nvs.*   : 永久存储API
 * - http.*  : 网络请求API
 * - hw.*    : 硬件控制API
 * - sys.*   : 系统工具API
 */
class LuaBindings {
public:
  /**
   * @brief 注册所有API到Lua虚拟机
   * @param epd 墨水屏驱动引用
   * @param config 配置管理器引用
   */
  static void registerAll(EPD_Driver &epd, ConfigManager &config);

  /**
   * @brief 获取framebuffer指针（供LuaCard使用）
   */
  static uint8_t *getFramebuffer() {
    return framebuffer ? framebuffer->getBuffer() : nullptr;
  }

  /**
   * @brief 获取framebuffer大小
   */
  static size_t getFramebufferSize() { return FRAMEBUFFER_SIZE; }

private:
  // ========== eink.* API ==========
  static int lua_eink_clear(lua_State *L);
  static int lua_eink_drawPixel(lua_State *L);
  static int lua_eink_drawLine(lua_State *L);
  static int lua_eink_drawRect(lua_State *L);
  static int lua_eink_fillRect(lua_State *L);
  static int lua_eink_drawStr(lua_State *L);
  static int lua_eink_drawChinese(lua_State *L);
  static int lua_eink_refresh(lua_State *L);
  static int lua_eink_refreshPartial(lua_State *L);
  static int lua_eink_refreshDeep(lua_State *L);
  static int lua_eink_getWidth(lua_State *L);
  static int lua_eink_getHeight(lua_State *L);

  // ========== nvs.* API ==========
  static int lua_nvs_set(lua_State *L);
  static int lua_nvs_get(lua_State *L);
  static int lua_nvs_remove(lua_State *L);

  // ========== http.* API ==========
  static int lua_http_get(lua_State *L);
  static int lua_http_post(lua_State *L);
  static int lua_http_downloadBitmap(lua_State *L);

  // ========== hw.* API ==========
  static int lua_hw_vibrate(lua_State *L);
  static int lua_hw_sleep(lua_State *L);
  static int lua_hw_getBattery(lua_State *L);

  // ========== sys.* API ==========
  static int lua_sys_log(lua_State *L);
  static int lua_sys_millis(lua_State *L);
  static int lua_sys_delay(lua_State *L);

  // ========== 辅助函数 ==========
  static bool isUrlAllowed(const String &url);
  static void registerNamespace(lua_State *L, const char *name,
                                const luaL_Reg *funcs);

  // ========== 全局引用 ==========
  static EPD_Driver *epd;
  static ConfigManager *config;
  static GFXcanvas1 *framebuffer;
  static const size_t FRAMEBUFFER_SIZE = 2756; // 104 * 212 / 8 = 2756

  // ========== HTTP白名单 ==========
  static const char *allowedDomains[];
};

#endif // LUA_BINDINGS_H
