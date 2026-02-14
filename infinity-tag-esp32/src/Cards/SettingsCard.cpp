#include "Cards/SettingsCard.h"
#include "Utils/ChineseFont.h"
#include "Utils/Logger.h"

SettingsCard::SettingsCard(EPD_Driver &epd, ConfigManager &config,
                           WiFiProvisioning &wifiProv, OTAManager &otaManager)
    : _epd(epd), _config(config), _wifiProv(wifiProv), _otaManager(otaManager),
      _state(STATE_MENU), _selectedIndex(0), _needsRender(true),
      _partialRefreshCount(0) {}

SettingsCard::~SettingsCard() {}

void SettingsCard::onEnter() {
  LOG_DEBUG("[SettingsCard] Entering settings");

  _state = STATE_MENU;
  _selectedIndex = 0;
  _needsRender = true;
  _partialRefreshCount = 0; // 重置计数器

  // 初始化缓存
  _soundEnabled = _config.getSoundEnabled();

  // 如果是首次启动且没有WiFi配置，自动进入配网模式
  if (_config.isFirstBoot() || !_config.hasWiFiConfig()) {
    LOG_DEBUG("[SettingsCard] First boot, entering WiFi provisioning");
    _enterWiFiProvisioning();
  } else {
    _renderMenu(true); // 首次进入使用深度刷新
  }
}

void SettingsCard::onExit() {
  LOG_DEBUG("[SettingsCard] Exiting settings");

  // 如果正在配网，停止配网
  if (_state == STATE_WIFI_PROVISIONING) {
    _exitWiFiProvisioning();
  }
}

void SettingsCard::onEvent(const Event &event) {
  if (_state == STATE_WIFI_PROVISIONING) {
    // 配网模式下，短按返回设置界面，长按退出配网
    if (event.type == EVENT_BUTTON_RELEASE) {
      LOG_DEBUG("[SettingsCard] Button release in provisioning mode, returning to menu");
      _exitWiFiProvisioning();
      // _exitWiFiProvisioning() 已经调用了 _renderMenu(true)，不需要重复调用
    } else if (event.type == EVENT_BUTTON_LONG_PRESS) {
      LOG_DEBUG("[SettingsCard] Long press in provisioning mode, exiting");
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
        LOG_DEBUG("[SettingsCard] User confirmed update");
        _otaManager.performUpdate();
      } else if (event.type == EVENT_BUTTON_LONG_PRESS) {
        LOG_DEBUG("[SettingsCard] User cancelled update");
        _state = STATE_MENU;
        _partialRefreshCount = 0;
        _renderMenu(true);
      }
      break;

    case INFO_FACTORY_RESET:
      // 恢复出厂设置：长按确认，短按取消
      if (event.type == EVENT_BUTTON_LONG_PRESS) {
        LOG_DEBUG("[SettingsCard] User confirmed factory reset");
        // TODO: 实现恢复出厂设置逻辑
        // _config.clearAll();
        // ESP.restart();
        _state = STATE_MENU;
        _partialRefreshCount = 0;
        _renderMenu(true);
      } else if (event.type == EVENT_BUTTON_RELEASE) {
        LOG_DEBUG("[SettingsCard] User cancelled factory reset");
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
    {
      int oldSelectedIndex = _selectedIndex;
      _selectedIndex += event.value;
      if (_selectedIndex < 0) {
        _selectedIndex = MENU_COUNT - 1;
      } else if (_selectedIndex >= MENU_COUNT) {
        _selectedIndex = 0;
      }

      // 检查是否需要滚动（选中项超出当前可见范围）
      const int visibleItems = VISIBLE_ITEMS;
      bool needsScroll = false;

      if (MENU_COUNT > visibleItems) {
        // 计算旧的可见范围
        int oldStartIndex = 0;
        int oldEndIndex = MENU_COUNT - 1;
        if (oldSelectedIndex >= visibleItems) {
          oldStartIndex = oldSelectedIndex - visibleItems + 1;
          oldEndIndex = oldSelectedIndex;
        } else {
          oldEndIndex = visibleItems - 1;
        }

        // 检查新选中项是否在旧的可见范围内
        if (_selectedIndex < oldStartIndex || _selectedIndex > oldEndIndex) {
          needsScroll = true;
        }
      }

      _needsRender = true;

      // 如果需要滚动，使用闪白快刷；否则使用局部刷新
      if (needsScroll) {
        LOG_DEBUG("[SettingsCard] Scrolling - using flicker refresh");
        // 先闪白
        _epd.refreshFlicker([](EPD_Class &d) {
          d.fillScreen(GxEPD_WHITE);
        });
        // 再渲染菜单
        _renderMenu(false);
      } else {
        LOG_DEBUG("[SettingsCard] Selection changed - using partial refresh");
        _renderMenu(false);
      }
    }
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

// ==================== 菜单渲染辅助函数 ====================

void SettingsCard::_drawMenuTitle(EPD_Class &d) {
  // 标题
  ChineseFont::drawString(d, LEFT_MARGIN, TITLE_Y, "设置", GxEPD_BLACK);

  // 分割线：标题下方1px，覆盖整个屏幕宽度，需要加OFFSET_Y
  d.drawFastHLine(0, DIVIDER_Y + OFFSET_Y, SCREEN_WIDTH, GxEPD_BLACK);
}

const char* SettingsCard::_getMenuText(int index) {
  switch (index) {
    case 0: return "网络设置";   // MENU_WIFI_CONFIG
    case 1: return "声音";       // MENU_SOUND_TOGGLE
    case 2: return "固件信息";   // MENU_FIRMWARE_INFO
    case 3: return "检查更新";   // MENU_CHECK_UPDATE
    case 4: return "恢复出厂";   // MENU_FACTORY_RESET
    default: return "未知";
  }
}

void SettingsCard::_drawMenuItem(EPD_Class &d, int index, int y, int selectedIndex, bool soundEnabled) {
  // 选中标记
  if (index == selectedIndex) {
    ChineseFont::drawString(d, SELECTION_MARKER_X, y, ">", GxEPD_BLACK);
  }

  // 菜单项文本（第一行）
  const char *menuText = _getMenuText(index);
  ChineseFont::drawString(d, MENU_TEXT_X, y, menuText, GxEPD_BLACK);

  // 菜单项值（第二行，缩进显示，增加间距）
  if (index == 1) { // MENU_SOUND_TOGGLE
    const char *value = soundEnabled ? "开" : "关";
    ChineseFont::drawString(d, MENU_TEXT_X + 10, y + 16, value, GxEPD_BLACK);
  } else if (index == 2) { // MENU_FIRMWARE_INFO - 显示版本号
    char versionBuf[32];
    snprintf(versionBuf, sizeof(versionBuf), "v%d.%d.%d.%d",
             VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);
    ChineseFont::drawString(d, MENU_TEXT_X + 10, y + 16, versionBuf, GxEPD_BLACK);
  }
}

void SettingsCard::_drawScrollIndicators(EPD_Class &d, int startIndex, int endIndex, int menuCount) {
  // 滚动指示器 - 使用GFX绘制三角形避免字体查找问题
  // 注意：直接使用GFX需要手动加 OFFSET_Y

  if (startIndex > 0) {
    // 向上箭头：顶点在Y=ARROW_UP_Y,X=ARROW_X，底边在Y=ARROW_UP_Y+6
    d.fillTriangle(ARROW_X, ARROW_UP_Y + OFFSET_Y,
                   ARROW_X - 4, ARROW_UP_Y + 6 + OFFSET_Y,
                   ARROW_X + 4, ARROW_UP_Y + 6 + OFFSET_Y,
                   GxEPD_BLACK);
  }
  if (endIndex < menuCount - 1) {
    // 向下箭头：底部顶点在Y=ARROW_DOWN_Y,X=ARROW_X，顶边在Y=ARROW_DOWN_Y-6
    d.fillTriangle(ARROW_X, ARROW_DOWN_Y + OFFSET_Y,
                   ARROW_X - 4, ARROW_DOWN_Y - 6 + OFFSET_Y,
                   ARROW_X + 4, ARROW_DOWN_Y - 6 + OFFSET_Y,
                   GxEPD_BLACK);
  }
}

// ==================== 主渲染函数 ====================

void SettingsCard::_renderMenu(bool forceDeep) {
  LOG_DEBUG("[SettingsCard] Rendering menu");

  // 捕获需要的变量（避免捕获this指针）
  int selectedIndex = _selectedIndex;
  bool soundEnabled = _soundEnabled;

  // 绘制逻辑封装为 lambda
  auto drawMenu = [selectedIndex, soundEnabled](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 绘制标题和分割线
    _drawMenuTitle(d);

    // 计算滚动偏移（最多显示VISIBLE_ITEMS个项目）
    const int MENU_COUNT = 5;   // 菜单项总数
    int startIndex = 0;
    int endIndex = MENU_COUNT - 1;

    // 如果菜单项超过VISIBLE_ITEMS个，实现滚动
    if (MENU_COUNT > VISIBLE_ITEMS) {
      // 确保选中项在可见范围内
      if (selectedIndex < VISIBLE_ITEMS) {
        startIndex = 0;
        endIndex = VISIBLE_ITEMS - 1;
      } else {
        startIndex = selectedIndex - VISIBLE_ITEMS + 1;
        endIndex = selectedIndex;
      }
    }

    // 绘制菜单项
    for (int i = startIndex; i <= endIndex && i < MENU_COUNT; i++) {
      int y = MENU_START_Y + (i - startIndex) * MENU_ITEM_HEIGHT;
      _drawMenuItem(d, i, y, selectedIndex, soundEnabled);
    }

    // 绘制滚动指示器
    _drawScrollIndicators(d, startIndex, endIndex, MENU_COUNT);
  };

  // 使用新 API - 刷新策略
  if (forceDeep) {
    _epd.refreshDeep(drawMenu);
  } else {
    // 菜单选择变化使用局部刷新
    _epd.refreshPartial(drawMenu);
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
  LOG_PRINTF("[SettingsCard] Execute menu item: %d\n", index);

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
  LOG_DEBUG("[SettingsCard] Entering WiFi provisioning");

  _state = STATE_WIFI_PROVISIONING;

  // 启动配网
  if (_wifiProv.start()) {
    LOG_DEBUG("[SettingsCard] WiFi provisioning started");
  } else {
    LOG_DEBUG("[SettingsCard] ERROR: Failed to start WiFi provisioning");
    _state = STATE_MENU;
    _renderMenu();
  }
}

void SettingsCard::_exitWiFiProvisioning() {
  LOG_DEBUG("[SettingsCard] Exiting WiFi provisioning");

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

  LOG_PRINTF("[SettingsCard] Sound toggled: %s\n",
                _soundEnabled ? "ON" : "OFF");

  _needsRender = true;
  _renderMenu(false); // 切换开关使用局部刷新
}

void SettingsCard::_showFirmwareInfo() {
  LOG_DEBUG("[SettingsCard] Showing firmware info");

  // 使用静态字符数组，避免String生命周期问题
  static char versionBuf[32];
  static char buildDateBuf[64];

  snprintf(versionBuf, sizeof(versionBuf), "v%d.%d.%d.%d", VERSION_MAJOR,
           VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);
  snprintf(buildDateBuf, sizeof(buildDateBuf), "%s %s", __DATE__, __TIME__);

  _epd.refreshFull([](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题居中
    int16_t titleWidth = ChineseFont::getStringWidth("固件信息");
    int16_t titleX = (104 - titleWidth) / 2;
    ChineseFont::drawString(d, titleX, 8, "固件信息", GxEPD_BLACK);

    // 版本信息左对齐
    ChineseFont::drawString(d, 10, 40, "版本:", GxEPD_BLACK);
    ChineseFont::drawString(d, 10, 60, versionBuf, GxEPD_BLACK);

    // 构建日期
    ChineseFont::drawString(d, 10, 80, "构建:", GxEPD_BLACK);
    ChineseFont::drawString(d, 10, 100, buildDateBuf, GxEPD_BLACK);

    // 提示在底部
    ChineseFont::drawString(d, 10, 190, "按键返回", GxEPD_BLACK);
  });

  // 进入信息显示状态，等待用户按键返回
  _state = STATE_INFO_DISPLAY;
  _infoDisplayType = INFO_FIRMWARE;
}

void SettingsCard::_checkUpdate() {
  LOG_DEBUG("[SettingsCard] Checking for updates");

  if (_otaManager.checkUpdate()) {
    String version = _otaManager.getLatestVersion();
    size_t size = _otaManager.getUpdateSize();

    // 按值捕获，避免引用失效
    _epd.refreshFull([version, size](EPD_Class &d) {
      d.fillScreen(GxEPD_WHITE);
      d.setTextColor(GxEPD_BLACK);

      // 标题（Y=8）
      ChineseFont::drawString(d, 10, 8, "发现更新", GxEPD_BLACK);

      // 版本（Y=28）
      ChineseFont::drawString(d, 10, 28, "版本:", GxEPD_BLACK);
      ChineseFont::drawString(d, 58, 28, version, GxEPD_BLACK);

      // 大小（Y=46）
      ChineseFont::drawString(d, 10, 46, "大小:", GxEPD_BLACK);
      String sizeStr = String(size / 1024) + " KB";
      ChineseFont::drawString(d, 58, 46, sizeStr, GxEPD_BLACK);

      // 提示（Y=70, Y=88）
      ChineseFont::drawString(d, 10, 70, "按键更新", GxEPD_BLACK);
      ChineseFont::drawString(d, 10, 88, "长按取消", GxEPD_BLACK);
    });

    // 进入信息显示状态，等待用户确认
    _state = STATE_INFO_DISPLAY;
    _infoDisplayType = INFO_UPDATE_AVAILABLE;
  } else {
    // 没有更新，显示提示后返回菜单
    _epd.refreshFull([](EPD_Class &d) {
      d.fillScreen(GxEPD_WHITE);
      d.setTextColor(GxEPD_BLACK);

      // 标题（Y=8）
      ChineseFont::drawString(d, 10, 8, "无更新", GxEPD_BLACK);

      // 消息（Y=28）
      ChineseFont::drawString(d, 10, 28, "已是最新版本", GxEPD_BLACK);

      // 提示（Y=86）
      ChineseFont::drawString(d, 10, 86, "按键返回", GxEPD_BLACK);
    });

    _state = STATE_INFO_DISPLAY;
    _infoDisplayType = INFO_NO_UPDATE;
  }
}

void SettingsCard::_factoryReset() {
  LOG_DEBUG("[SettingsCard] Factory reset requested");

  _epd.refreshFull([](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题（Y=8）
    ChineseFont::drawString(d, 10, 8, "恢复出厂?", GxEPD_BLACK);

    // 警告消息（Y=28, Y=46）
    ChineseFont::drawString(d, 10, 28, "所有数据将被", GxEPD_BLACK);
    ChineseFont::drawString(d, 10, 46, "清除!", GxEPD_BLACK);

    // 提示（Y=86）
    ChineseFont::drawString(d, 10, 86, "长按确认", GxEPD_BLACK);
  });

  // 进入信息显示状态，等待用户确认
  _state = STATE_INFO_DISPLAY;
  _infoDisplayType = INFO_FACTORY_RESET;
}

bool SettingsCard::_showConfirmDialog(const String &message) {
  // TODO: 实现确认对话框
  return false;
}
