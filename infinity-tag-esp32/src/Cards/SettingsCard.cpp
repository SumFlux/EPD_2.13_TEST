#include "Cards/SettingsCard.h"
#include "Utils/ChineseFont.h"

SettingsCard::SettingsCard(EPD_Driver &epd, ConfigManager &config,
                           WiFiProvisioning &wifiProv, OTAManager &otaManager)
    : _epd(epd), _config(config), _wifiProv(wifiProv), _otaManager(otaManager),
      _state(STATE_MENU), _selectedIndex(0), _needsRender(true),
      _partialRefreshCount(0) {}

SettingsCard::~SettingsCard() {}

void SettingsCard::onEnter() {
  Serial.println("[SettingsCard] Entering settings");

  _state = STATE_MENU;
  _selectedIndex = 0;
  _needsRender = true;
  _partialRefreshCount = 0; // 重置计数器

  // 初始化缓存
  _soundEnabled = _config.getSoundEnabled();

  // 如果是首次启动且没有WiFi配置，自动进入配网模式
  if (_config.isFirstBoot() || !_config.hasWiFiConfig()) {
    Serial.println("[SettingsCard] First boot, entering WiFi provisioning");
    _enterWiFiProvisioning();
  } else {
    _renderMenu(true); // 首次进入使用深度刷新
  }
}

void SettingsCard::onExit() {
  Serial.println("[SettingsCard] Exiting settings");

  // 如果正在配网，停止配网
  if (_state == STATE_WIFI_PROVISIONING) {
    _exitWiFiProvisioning();
  }
}

void SettingsCard::onEvent(const Event &event) {
  if (_state == STATE_WIFI_PROVISIONING) {
    // 配网模式下，只处理长按退出
    if (event.type == EVENT_BUTTON_LONG_PRESS) {
      _exitWiFiProvisioning();
    }
    return;
  }

  if (_state == STATE_INFO_DISPLAY) {
    // 信息显示模式下，根据子状态处理不同的按键操作
    switch (_infoDisplayType) {
    case INFO_FIRMWARE:
    case INFO_NO_UPDATE:
      // 固件信息/无更新：短按返回菜单
      if (event.type == EVENT_BUTTON_RELEASE) {
        _state = STATE_MENU;
        _partialRefreshCount = 0;
        _renderMenu(true);
      }
      break;

    case INFO_UPDATE_AVAILABLE:
      // 有更新可用：短按开始更新，长按取消
      if (event.type == EVENT_BUTTON_RELEASE) {
        Serial.println("[SettingsCard] User confirmed update");
        _otaManager.performUpdate();
      } else if (event.type == EVENT_BUTTON_LONG_PRESS) {
        Serial.println("[SettingsCard] User cancelled update");
        _state = STATE_MENU;
        _partialRefreshCount = 0;
        _renderMenu(true);
      }
      break;

    case INFO_FACTORY_RESET:
      // 恢复出厂设置：长按确认，短按取消
      if (event.type == EVENT_BUTTON_LONG_PRESS) {
        Serial.println("[SettingsCard] User confirmed factory reset");
        // TODO: 实现恢复出厂设置逻辑
        // _config.clearAll();
        // ESP.restart();
        _state = STATE_MENU;
        _partialRefreshCount = 0;
        _renderMenu(true);
      } else if (event.type == EVENT_BUTTON_RELEASE) {
        Serial.println("[SettingsCard] User cancelled factory reset");
        _state = STATE_MENU;
        _partialRefreshCount = 0;
        _renderMenu(true);
      }
      break;
    }
    return;
  }

  // 菜单模式
  switch (event.type) {
  case EVENT_ENCODER_ROTATE:
    // 上下滚动菜单
    _selectedIndex += event.value;
    if (_selectedIndex < 0) {
      _selectedIndex = MENU_COUNT - 1;
    } else if (_selectedIndex >= MENU_COUNT) {
      _selectedIndex = 0;
    }
    _needsRender = true;
    _renderMenu(false); // 滚动使用局部刷新
    break;

  case EVENT_BUTTON_RELEASE:
    // 执行选中的菜单项
    _executeMenuItem(_selectedIndex);
    break;

  default:
    break;
  }
}

void SettingsCard::render(uint8_t *framebuffer, size_t size) {
  // SettingsCard 使用直接渲染，不使用framebuffer
}

void SettingsCard::update() {
  // 如果在配网模式，处理DNS和Web请求
  if (_state == STATE_WIFI_PROVISIONING) {
    _wifiProv.update();

    // 检查配网是否完成
    if (_wifiProv.isConfigured()) {
      // 配网完成，设备会自动重启
      // 这里不需要额外处理
    }
  }
}

void SettingsCard::_renderMenu(bool forceDeep) {
  Serial.println("[SettingsCard] Rendering menu");

  // 绘制逻辑封装为 lambda
  auto drawMenu = [this](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题（使用中文字体，左上角坐标）
    ChineseFont::drawString(d, 10, 28, "设置", GxEPD_BLACK);

    // 菜单项
    int startY = 50;
    int itemHeight = 12;

    for (int i = 0; i < MENU_COUNT; i++) {
      int y = startY + i * itemHeight;

      d.setTextSize(1);

      // 选中标记
      if (i == _selectedIndex) {
        d.setCursor(5, y);
        d.print(">");
      }

      // 菜单项文本（使用中文字体，左上角坐标）
      String itemText = _getMenuItemText(i);
      ChineseFont::drawString(d, 15, y, itemText, GxEPD_BLACK);

      // 菜单项值（使用中文字体）
      String value = _getMenuItemValue(i);
      if (!value.isEmpty()) {
        ChineseFont::drawString(d, 150, y, value, GxEPD_BLACK);
      }
    }
  };

  // 使用新 API - 刷新策略由 EPD_Driver 自动管理
  if (forceDeep) {
    _epd.refreshDeep(drawMenu);
  } else {
    _epd.refreshPartial(drawMenu); // 自动每5次触发全刷
  }

  _needsRender = false;
}

void SettingsCard::_renderMenuItem(int index, int y, bool selected) {
  _epd.getDisplay().setTextSize(1);

  // 选中标记
  if (selected) {
    _epd.getDisplay().setCursor(5, y);
    _epd.getDisplay().print(">");
  }

  // 菜单项文本
  _epd.getDisplay().setCursor(15, y);
  _epd.getDisplay().print(_getMenuItemText(index));

  // 菜单项值（如开关状态）
  String value = _getMenuItemValue(index);
  if (!value.isEmpty()) {
    _epd.getDisplay().setCursor(150, y);
    _epd.getDisplay().print(value);
  }
}

String SettingsCard::_getMenuItemText(int index) {
  switch (index) {
  case MENU_WIFI_CONFIG:
    return "网络设置";
  case MENU_SOUND_TOGGLE:
    return "声音";
  case MENU_FIRMWARE_INFO:
    return "固件信息";
  case MENU_CHECK_UPDATE:
    return "检查更新";
  case MENU_FACTORY_RESET:
    return "恢复出厂";
  default:
    return "未知";
  }
}

String SettingsCard::_getMenuItemValue(int index) {
  switch (index) {
  case MENU_SOUND_TOGGLE:
    return _soundEnabled ? "开" : "关";
  case MENU_FIRMWARE_INFO: {
    char version[32];
    snprintf(version, sizeof(version), "v%d.%d.%d.%d", VERSION_MAJOR,
             VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);
    return String(version);
  }
  default:
    return "";
  }
}

void SettingsCard::_executeMenuItem(int index) {
  Serial.printf("[SettingsCard] Execute menu item: %d\n", index);

  switch (index) {
  case MENU_WIFI_CONFIG:
    _enterWiFiProvisioning();
    break;

  case MENU_SOUND_TOGGLE:
    _toggleSound();
    break;

  case MENU_FIRMWARE_INFO:
    _showFirmwareInfo();
    break;

  case MENU_CHECK_UPDATE:
    _checkUpdate();
    break;

  case MENU_FACTORY_RESET:
    _factoryReset();
    break;
  }
}

void SettingsCard::_enterWiFiProvisioning() {
  Serial.println("[SettingsCard] Entering WiFi provisioning");

  _state = STATE_WIFI_PROVISIONING;

  // 启动配网
  if (_wifiProv.start()) {
    Serial.println("[SettingsCard] WiFi provisioning started");
  } else {
    Serial.println("[SettingsCard] ERROR: Failed to start WiFi provisioning");
    _state = STATE_MENU;
    _renderMenu();
  }
}

void SettingsCard::_exitWiFiProvisioning() {
  Serial.println("[SettingsCard] Exiting WiFi provisioning");

  // 停止配网
  _wifiProv.stop();

  _state = STATE_MENU;
  _needsRender = true;
  _partialRefreshCount = 0; // 重置计数器
  _renderMenu(true);        // 退出配网使用深度刷新
}

void SettingsCard::_toggleSound() {
  _soundEnabled = !_soundEnabled;
  _config.setSoundEnabled(_soundEnabled);

  Serial.printf("[SettingsCard] Sound toggled: %s\n",
                _soundEnabled ? "ON" : "OFF");

  _needsRender = true;
  _renderMenu(false); // 切换开关使用局部刷新
}

void SettingsCard::_showFirmwareInfo() {
  Serial.println("[SettingsCard] Showing firmware info");

  char version[32];
  snprintf(version, sizeof(version), "v%d.%d.%d.%d", VERSION_MAJOR,
           VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);

  _epd.refreshFull([&](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题（使用中文字体）
    ChineseFont::drawString(d, 10, 28, "固件信息", GxEPD_BLACK);

    d.setTextSize(1);
    ChineseFont::drawString(d, 10, 50, "版本:", GxEPD_BLACK);
    d.setCursor(50, 52);
    d.print(version);

    ChineseFont::drawString(d, 10, 68, "构建日期:", GxEPD_BLACK);
    d.setCursor(80, 70);
    d.print(__DATE__);
    d.print(" ");
    d.print(__TIME__);

    ChineseFont::drawString(d, 10, 90, "按键返回", GxEPD_BLACK);
  });

  // 进入信息显示状态，等待用户按键返回
  _state = STATE_INFO_DISPLAY;
  _infoDisplayType = INFO_FIRMWARE;
}

void SettingsCard::_checkUpdate() {
  Serial.println("[SettingsCard] Checking for updates");

  if (_otaManager.checkUpdate()) {
    String version = _otaManager.getLatestVersion();
    size_t size = _otaManager.getUpdateSize();

    _epd.refreshFull([&](EPD_Class &d) {
      d.fillScreen(GxEPD_WHITE);
      d.setTextColor(GxEPD_BLACK);

      // 标题（使用中文字体）
      ChineseFont::drawString(d, 10, 28, "发现更新", GxEPD_BLACK);

      d.setTextSize(1);
      ChineseFont::drawString(d, 10, 50, "版本: ", GxEPD_BLACK);
      d.setCursor(50, 52);
      d.print(version);

      ChineseFont::drawString(d, 10, 66, "大小: ", GxEPD_BLACK);
      d.setCursor(50, 68);
      d.print(size / 1024);
      d.print(" KB");

      ChineseFont::drawString(d, 10, 86, "按键更新", GxEPD_BLACK);
      ChineseFont::drawString(d, 10, 100, "长按取消", GxEPD_BLACK);
    });

    // 进入信息显示状态，等待用户确认
    _state = STATE_INFO_DISPLAY;
    _infoDisplayType = INFO_UPDATE_AVAILABLE;
  } else {
    // 没有更新，显示提示后返回菜单
    _epd.refreshFull([](EPD_Class &d) {
      d.fillScreen(GxEPD_WHITE);
      d.setTextColor(GxEPD_BLACK);

      // 使用中文字体
      ChineseFont::drawString(d, 10, 38, "无更新", GxEPD_BLACK);
      ChineseFont::drawString(d, 10, 58, "已是最新版本", GxEPD_BLACK);
      ChineseFont::drawString(d, 10, 80, "按键返回", GxEPD_BLACK);
    });

    _state = STATE_INFO_DISPLAY;
    _infoDisplayType = INFO_NO_UPDATE;
  }
}

void SettingsCard::_factoryReset() {
  Serial.println("[SettingsCard] Factory reset requested");

  _epd.refreshFull([](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 使用中文字体
    ChineseFont::drawString(d, 10, 38, "恢复出厂?", GxEPD_BLACK);
    ChineseFont::drawString(d, 10, 58, "所有数据将被", GxEPD_BLACK);
    ChineseFont::drawString(d, 10, 72, "清除!", GxEPD_BLACK);
    ChineseFont::drawString(d, 10, 96, "长按确认", GxEPD_BLACK);
  });

  // 进入信息显示状态，等待用户确认
  _state = STATE_INFO_DISPLAY;
  _infoDisplayType = INFO_FACTORY_RESET;
}

bool SettingsCard::_showConfirmDialog(const String &message) {
  // TODO: 实现确认对话框
  return false;
}
