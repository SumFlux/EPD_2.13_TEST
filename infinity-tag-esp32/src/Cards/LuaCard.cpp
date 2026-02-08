#include "Cards/LuaCard.h"

LuaCard::LuaCard(const String& scriptPath, EPD_Driver& epd)
    : scriptPath(scriptPath),
      epd(epd),
      lua(LuaEngine::getInstance()),
      cardName("Unknown"),
      cardCategory("Other"),
      cardLogo("/icons/card_default.bin"),
      cardOrder(100),
      cardEnabled(true),
      loaded(false),
      lastRenderTime(0) {
  loaded = loadScriptAndMetadata();
}

bool LuaCard::loadScriptAndMetadata() {
  // 加载脚本
  if (!lua.loadScript(scriptPath)) {
    errorMessage = "Failed to load script: " + lua.getLastError();
    Serial.printf("[LuaCard] ERROR: %s\n", errorMessage.c_str());
    return false;
  }

  // 读取元数据
  cardName = lua.getGlobalString("CARD_NAME");
  if (cardName.isEmpty()) {
    cardName = "Lua Card";
  }

  cardCategory = lua.getGlobalString("CARD_CATEGORY");
  if (cardCategory.isEmpty()) {
    cardCategory = "Other";
  }

  cardLogo = lua.getGlobalString("CARD_LOGO");
  if (cardLogo.isEmpty()) {
    cardLogo = "/icons/card_default.bin";
  }

  cardOrder = lua.getGlobalInt("CARD_ORDER");
  if (cardOrder == 0) {
    cardOrder = 100;
  }

  cardEnabled = lua.getGlobalInt("CARD_ENABLED") != 0;

  Serial.printf("[LuaCard] Loaded: %s (Category: %s, Order: %d)\n",
                cardName.c_str(), cardCategory.c_str(), cardOrder);

  return true;
}

void LuaCard::onEnter() {
  if (!loaded) {
    Serial.printf("[LuaCard] Cannot enter: script not loaded\n");
    return;
  }

  Serial.printf("[LuaCard] Entering: %s\n", cardName.c_str());

  // 调用Lua的onInit函数
  Serial.println("[LuaCard] Calling Lua onInit()");
  if (!lua.callFunction("onInit")) {
    errorMessage = "onInit failed: " + lua.getLastError();
    Serial.printf("[LuaCard] ERROR: %s\n", errorMessage.c_str());
    loaded = false;
  } else {
    Serial.println("[LuaCard] Lua onInit() completed successfully");
  }
}

void LuaCard::onExit() {
  if (!loaded) {
    return;
  }

  Serial.printf("[LuaCard] Exiting: %s\n", cardName.c_str());

  // 调用Lua的onExit函数
  if (!lua.callFunction("onExit")) {
    Serial.printf("[LuaCard] WARNING: onExit failed: %s\n",
                  lua.getLastError().c_str());
  }
}

void LuaCard::onEvent(const Event& event) {
  if (!loaded) {
    return;
  }

  // 映射事件到Lua回调函数
  const char* callbackName = nullptr;

  switch (event.type) {
  case EVENT_BUTTON_PRESS:
    callbackName = "onBtnPress";
    break;
  case EVENT_BUTTON_LONG_PRESS:
    callbackName = "onBtnLong";
    break;
  case EVENT_ENCODER_ROTATE:
    if (event.value > 0) {
      callbackName = "onEncoderCW";
    } else if (event.value < 0) {
      callbackName = "onEncoderCCW";
    }
    break;
  case EVENT_VIBRATION:
    callbackName = "onShake";
    break;
  default:
    return;
  }

  // 调用Lua回调函数
  if (!lua.callFunction(callbackName)) {
    Serial.printf("[LuaCard] WARNING: %s failed: %s\n", callbackName,
                  lua.getLastError().c_str());
  }
}

void LuaCard::render(uint8_t* framebuffer, size_t size) {
  if (!loaded) {
    renderErrorCard(framebuffer, size);
    return;
  }

  // 限制渲染频率为10Hz
  uint32_t now = millis();
  if (now - lastRenderTime < RENDER_INTERVAL_MS) {
    return;
  }
  lastRenderTime = now;

  // 调用Lua的onLoop函数
  if (!lua.callFunction("onLoop")) {
    errorMessage = "onLoop failed: " + lua.getLastError();
    Serial.printf("[LuaCard] ERROR: %s\n", errorMessage.c_str());
    loaded = false;
    renderErrorCard(framebuffer, size);
    return;
  }

  // 将Lua绘制的framebuffer复制到输出
  uint8_t* luaFramebuffer = LuaBindings::getFramebuffer();
  if (luaFramebuffer && size >= LuaBindings::getFramebufferSize()) {
    memcpy(framebuffer, luaFramebuffer, LuaBindings::getFramebufferSize());
  }
}

void LuaCard::renderErrorCard(uint8_t* framebuffer, size_t size) {
  // 清空framebuffer
  memset(framebuffer, 0xFF, size);

  // 使用EPD_Driver的回调方式绘制错误信息
  epd.refreshFull([this](EPD_Class& display) {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(10, 28);  // 18 + 10
    display.print("Lua Card Error");
    display.setCursor(10, 48);  // 18 + 30
    display.print(cardName.c_str());
    display.setCursor(10, 68);  // 18 + 50
    display.print(errorMessage.c_str());

    // 绘制边框
    display.drawRect(0, 18, 212, 104, GxEPD_BLACK);
  });
}
