#include "Lua/LuaBindings.h"
#include "Utils/ChineseFont.h"
#include <HTTPClient.h>
#include <Preferences.h>

// 静态成员初始化
EPD_Driver *LuaBindings::epd = nullptr;
ConfigManager *LuaBindings::config = nullptr;
GFXcanvas1 *LuaBindings::framebuffer = nullptr;

// HTTP白名单
const char *LuaBindings::allowedDomains[] = {"api.infinitytag.app",
                                             "192.168.", // 局域网
                                             nullptr};

// ========== 辅助函数 ==========

bool LuaBindings::isUrlAllowed(const String &url) {
  for (int i = 0; allowedDomains[i] != nullptr; i++) {
    if (url.indexOf(allowedDomains[i]) >= 0) {
      return true;
    }
  }
  return false;
}

void LuaBindings::registerNamespace(lua_State *L, const char *name,
                                    const luaL_Reg *funcs) {
  lua_newtable(L);
  luaL_setfuncs(L, funcs, 0);
  lua_setglobal(L, name);
}

// ========== eink.* API ==========

int LuaBindings::lua_eink_clear(lua_State *L) {
  if (!framebuffer) {
    return luaL_error(L, "Framebuffer not initialized");
  }
  framebuffer->fillScreen(1); // 1 = White
  return 0;
}

int LuaBindings::lua_eink_drawPixel(lua_State *L) {
  if (!framebuffer) {
    return luaL_error(L, "Framebuffer not initialized");
  }

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int color = luaL_optinteger(L, 3, 0); // 默认黑色

  // GFXCanvas1 内部处理边界检查，但显式检查更安全
  if (x < 0 || x >= 104 || y < 0 || y >= 212) {
    return 0; // 超出范围，忽略
  }

  // 1 = White, 0 = Black
  framebuffer->drawPixel(x, y, (color != 0) ? 1 : 0);

  return 0;
}

int LuaBindings::lua_eink_drawLine(lua_State *L) {
  int x1 = luaL_checkinteger(L, 1);
  int y1 = luaL_checkinteger(L, 2);
  int x2 = luaL_checkinteger(L, 3);
  int y2 = luaL_checkinteger(L, 4);

  // Bresenham算法
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = (x1 < x2) ? 1 : -1;
  int sy = (y1 < y2) ? 1 : -1;
  int err = dx - dy;

  while (true) {
    lua_pushinteger(L, x1);
    lua_pushinteger(L, y1);
    lua_pushinteger(L, 0);
    lua_eink_drawPixel(L);
    lua_pop(L, 1);

    if (x1 == x2 && y1 == y2)
      break;

    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x1 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y1 += sy;
    }
  }

  return 0;
}

int LuaBindings::lua_eink_drawRect(lua_State *L) {
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);

  // 画四条边
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, x + w - 1);
  lua_pushinteger(L, y);
  lua_eink_drawLine(L);

  lua_pushinteger(L, x + w - 1);
  lua_pushinteger(L, y);
  lua_pushinteger(L, x + w - 1);
  lua_pushinteger(L, y + h - 1);
  lua_eink_drawLine(L);

  lua_pushinteger(L, x + w - 1);
  lua_pushinteger(L, y + h - 1);
  lua_pushinteger(L, x);
  lua_pushinteger(L, y + h - 1);
  lua_eink_drawLine(L);

  lua_pushinteger(L, x);
  lua_pushinteger(L, y + h - 1);
  lua_pushinteger(L, x);
  lua_pushinteger(L, y);
  lua_eink_drawLine(L);

  return 0;
}

int LuaBindings::lua_eink_fillRect(lua_State *L) {
  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  int w = luaL_checkinteger(L, 3);
  int h = luaL_checkinteger(L, 4);

  // 使用 GFXCanvas1 的原生绘图函数
  framebuffer->fillRect(x, y, w, h, 0); // 0 = Black
  return 0;
}

int LuaBindings::lua_eink_drawStr(lua_State *L) {
  if (!epd) {
    return luaL_error(L, "EPD driver not initialized");
  }

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  const char *text = luaL_checkstring(L, 3);

  // 在framebuffer上绘制文本
  if (framebuffer) {
    framebuffer->setTextSize(1);
    framebuffer->setTextColor(0); // 0 = Black
    framebuffer->setCursor(x, y); // 无需硬件偏移，Canvas是逻辑坐标
    framebuffer->print(text);
  }

  return 0;
}

int LuaBindings::lua_eink_drawChinese(lua_State *L) {
  if (!epd) {
    return luaL_error(L, "EPD driver not initialized");
  }

  int x = luaL_checkinteger(L, 1);
  int y = luaL_checkinteger(L, 2);
  const char *text = luaL_checkstring(L, 3);

  // 在framebuffer上绘制中文
  if (framebuffer) {
    ChineseFont::drawString(*framebuffer, x, y, String(text), 0); // 0 = Black
  }
  return 0;
}

int LuaBindings::lua_eink_refresh(lua_State *L) {
  if (!epd) {
    return luaL_error(L, "EPD driver not initialized");
  }

  // 使用全屏刷新
  // 使用全屏刷新
  epd->refreshFull([](EPD_Class &display) {
    // 将framebuffer的内容复制到display
    if (LuaBindings::framebuffer) {
      // Canvas是212x104，硬件可视区从Y=18开始
      // 1 = White, 0 = Black
      display.drawBitmap(0, 18, LuaBindings::framebuffer->getBuffer(), 212, 104, GxEPD_WHITE,
                         GxEPD_BLACK);
    }
  });
  return 0;
}

int LuaBindings::lua_eink_refreshPartial(lua_State *L) {
  if (!epd) {
    return luaL_error(L, "EPD driver not initialized");
  }

  // 使用局部刷新
  // 使用局部刷新
  // 使用局部刷新
  epd->refreshPartial([](EPD_Class &display) {
    // 将framebuffer的内容复制到display
    if (LuaBindings::framebuffer) {
      // Canvas是212x104，硬件可视区从Y=18开始
      display.drawBitmap(0, 18, LuaBindings::framebuffer->getBuffer(), 212, 104, GxEPD_WHITE,
                         GxEPD_BLACK);
    }
  });
  return 0;
}

int LuaBindings::lua_eink_refreshDeep(lua_State *L) {
  if (!epd) {
    return luaL_error(L, "EPD driver not initialized");
  }

  Serial.println("[Lua] eink.refreshDeep() called");

  if (!LuaBindings::framebuffer) {
    Serial.println("[Lua] ERROR: framebuffer is null!");
    return luaL_error(L, "Framebuffer not initialized");
  }

  Serial.printf("[Lua] Framebuffer address: %p\n", LuaBindings::framebuffer);
  Serial.printf("[Lua] Framebuffer buffer address: %p\n", LuaBindings::framebuffer->getBuffer());

  // 使用深度刷新（DEEP模式，闪烁3次，最慢但最彻底）
  epd->refreshDeep([](EPD_Class &display) {
    Serial.println("[Lua] Inside refreshDeep callback");
    // 将framebuffer的内容复制到display
    if (LuaBindings::framebuffer) {
      Serial.println("[Lua] Drawing bitmap to display");
      // Canvas是212x104，硬件可视区从Y=18开始
      display.drawBitmap(0, 18, LuaBindings::framebuffer->getBuffer(), 212, 104, GxEPD_WHITE,
                         GxEPD_BLACK);
      Serial.println("[Lua] Bitmap drawn successfully");
    } else {
      Serial.println("[Lua] ERROR: framebuffer is null in callback!");
    }
  });

  Serial.println("[Lua] eink.refreshDeep() completed");
  return 0;
}

int LuaBindings::lua_eink_getWidth(lua_State *L) {
  lua_pushinteger(L, 104);  // 目标可见宽度
  return 1;
}

int LuaBindings::lua_eink_getHeight(lua_State *L) {
  lua_pushinteger(L, 212);  // 目标可见高度
  return 1;
}

// ========== nvs.* API ==========

int LuaBindings::lua_nvs_set(lua_State *L) {
  const char *key = luaL_checkstring(L, 1);
  const char *value = luaL_checkstring(L, 2);

  Preferences prefs;
  prefs.begin("lua_cards", false);
  prefs.putString(key, value);
  prefs.end();

  return 0;
}

int LuaBindings::lua_nvs_get(lua_State *L) {
  const char *key = luaL_checkstring(L, 1);

  Preferences prefs;
  prefs.begin("lua_cards", true);
  String value = prefs.getString(key, "");
  prefs.end();

  lua_pushstring(L, value.c_str());
  return 1;
}

int LuaBindings::lua_nvs_remove(lua_State *L) {
  const char *key = luaL_checkstring(L, 1);

  Preferences prefs;
  prefs.begin("lua_cards", false);
  prefs.remove(key);
  prefs.end();

  return 0;
}

// ========== http.* API ==========

int LuaBindings::lua_http_get(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);

  if (!isUrlAllowed(url)) {
    return luaL_error(L, "URL not in whitelist: %s", url);
  }

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    lua_pushstring(L, payload.c_str());
    http.end();
    return 1;
  } else {
    http.end();
    return luaL_error(L, "HTTP GET failed: %d", httpCode);
  }
}

int LuaBindings::lua_http_post(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);
  const char *body = luaL_checkstring(L, 2);

  if (!isUrlAllowed(url)) {
    return luaL_error(L, "URL not in whitelist: %s", url);
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(body);

  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    lua_pushstring(L, payload.c_str());
    http.end();
    return 1;
  } else {
    http.end();
    return luaL_error(L, "HTTP POST failed: %d", httpCode);
  }
}

int LuaBindings::lua_http_downloadBitmap(lua_State *L) {
  const char *url = luaL_checkstring(L, 1);

  if (!isUrlAllowed(url)) {
    return luaL_error(L, "URL not in whitelist: %s", url);
  }

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int len = http.getSize();
    if (len != FRAMEBUFFER_SIZE) {
      http.end();
      return luaL_error(L, "Invalid bitmap size: %d (expected %d)", len,
                        FRAMEBUFFER_SIZE);
    }

    WiFiClient *stream = http.getStreamPtr();
    stream->readBytes(framebuffer->getBuffer(), FRAMEBUFFER_SIZE);
    http.end();

    lua_pushboolean(L, 1);
    return 1;
  } else {
    http.end();
    lua_pushboolean(L, 0);
    return 1;
  }
}

// ========== hw.* API ==========

int LuaBindings::lua_hw_vibrate(lua_State *L) {
  int duration = luaL_checkinteger(L, 1);
  // TODO: 实现震动功能（需要硬件支持）
  Serial.printf("[hw.vibrate] %d ms\n", duration);
  return 0;
}

int LuaBindings::lua_hw_sleep(lua_State *L) {
  // TODO: 实现深度睡眠功能
  Serial.println("[hw.sleep] Deep sleep requested");
  return 0;
}

int LuaBindings::lua_hw_getBattery(lua_State *L) {
  // TODO: 实现电池电量读取
  lua_pushinteger(L, 100); // 临时返回100%
  return 1;
}

// ========== sys.* API ==========

int LuaBindings::lua_sys_log(lua_State *L) {
  const char *message = luaL_checkstring(L, 1);
  Serial.printf("[Lua] %s\n", message);
  return 0;
}

int LuaBindings::lua_sys_millis(lua_State *L) {
  lua_pushinteger(L, millis());
  return 1;
}

int LuaBindings::lua_sys_delay(lua_State *L) {
  int ms = luaL_checkinteger(L, 1);
  delay(ms);
  return 0;
}

// ========== 注册所有API ==========

void LuaBindings::registerAll(EPD_Driver &epdRef, ConfigManager &configRef) {
  epd = &epdRef;
  config = &configRef;

  // 分配framebuffer（使用PSRAM）
  // 仅在PSRAM可用且强制使用时使用? GFXCanvas1 内部使用malloc
  // 我们可以通过宏重载 new 操作符? 不容易
  // 这里直接 new GFXCanvas1(104, 212)
  // 如果需要PSRAM，需要在 platformio.ini 中配置 -DBOARD_HAS_PSRAM
  // 并且 esp32-hal-psram.c 会自动接管 malloc

  framebuffer = new GFXcanvas1(104, 212);  // 目标可见区域尺寸
  if (framebuffer) {
    Serial.println("[LuaBindings] GFXcanvas1 allocated");
  } else {
    Serial.println("[LuaBindings] Failed to allocate GFXcanvas1");
  }

  if (!framebuffer) {
    Serial.println("[LuaBindings] ERROR: Failed to allocate framebuffer");
    return;
  }

  framebuffer->fillScreen(1); // 1 = White

  lua_State *L = LuaEngine::getInstance().getState();
  if (!L) {
    Serial.println("[LuaBindings] ERROR: Lua engine not initialized");
    return;
  }

  // 注册 eink.* API
  static const luaL_Reg eink_funcs[] = {
      {"clear", lua_eink_clear},
      {"drawPixel", lua_eink_drawPixel},
      {"drawLine", lua_eink_drawLine},
      {"drawRect", lua_eink_drawRect},
      {"fillRect", lua_eink_fillRect},
      {"drawStr", lua_eink_drawStr},
      {"drawChinese", lua_eink_drawChinese},
      {"refresh", lua_eink_refresh},
      {"refreshPartial", lua_eink_refreshPartial},
      {"refreshDeep", lua_eink_refreshDeep},
      {"getWidth", lua_eink_getWidth},
      {"getHeight", lua_eink_getHeight},
      {nullptr, nullptr}};
  registerNamespace(L, "eink", eink_funcs);

  // 注册 nvs.* API
  static const luaL_Reg nvs_funcs[] = {{"set", lua_nvs_set},
                                       {"get", lua_nvs_get},
                                       {"remove", lua_nvs_remove},
                                       {nullptr, nullptr}};
  registerNamespace(L, "nvs", nvs_funcs);

  // 注册 http.* API
  static const luaL_Reg http_funcs[] = {
      {"get", lua_http_get},
      {"post", lua_http_post},
      {"downloadBitmap", lua_http_downloadBitmap},
      {nullptr, nullptr}};
  registerNamespace(L, "http", http_funcs);

  // 注册 hw.* API
  static const luaL_Reg hw_funcs[] = {{"vibrate", lua_hw_vibrate},
                                      {"sleep", lua_hw_sleep},
                                      {"getBattery", lua_hw_getBattery},
                                      {nullptr, nullptr}};
  registerNamespace(L, "hw", hw_funcs);

  // 注册 sys.* API
  static const luaL_Reg sys_funcs[] = {{"log", lua_sys_log},
                                       {"millis", lua_sys_millis},
                                       {"delay", lua_sys_delay},
                                       {nullptr, nullptr}};
  registerNamespace(L, "sys", sys_funcs);

  Serial.println("[LuaBindings] All APIs registered");
}
