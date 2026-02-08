#ifndef SETTINGS_CARD_H
#define SETTINGS_CARD_H

#include "Core/Card.h"
#include "Core/ConfigManager.h"
#include "Driver/EPD_Driver.h"
#include "Network/OTAManager.h"
#include "Network/WiFiProvisioning.h"
#include "Version.h"
#include <vector>

// 菜单文本常量（避免lambda内部静态变量初始化问题）
namespace {
  static const char* const SETTINGS_MENU_TEXTS[] = {
    "网络设置",  // MENU_WIFI_CONFIG
    "声音",      // MENU_SOUND_TOGGLE
    "固件信息",  // MENU_FIRMWARE_INFO
    "检查更新",  // MENU_CHECK_UPDATE
    "恢复出厂"   // MENU_FACTORY_RESET
  };

  // UI 布局常量
  static const int VISIBLE_ITEMS = 4;        // 最多显示的菜单项数量
  static const int MENU_ITEM_HEIGHT = 18;    // 菜单项行高（px）
  static const int MENU_START_Y = 28;        // 菜单项起始Y坐标
  static const int TITLE_Y = 8;              // 标题Y坐标
  static const int DIVIDER_Y = 25;           // 分割线Y坐标（标题下方1px）
  static const int SCREEN_WIDTH = 212;       // 屏幕宽度（px）
  static const int SCREEN_HEIGHT = 104;      // 可见区域高度（px）
  static const int OFFSET_Y = 18;            // EPD硬件偏移
  static const int RIGHT_MARGIN = 10;        // 右边距（px）
  static const int ARROW_UP_Y = 26;          // 上箭头Y坐标
  static const int ARROW_DOWN_Y = 103;       // 下箭头Y坐标（屏幕底部）
  static const int ARROW_X = 207;            // 箭头X坐标（中心点）
  static const int SELECTION_MARKER_X = 5;   // 选中标记X坐标
  static const int MENU_TEXT_X = 25;         // 菜单文本X坐标
  static const int LEFT_MARGIN = 10;         // 左边距（px）
}

/**
 * @brief 设置卡片
 *
 * 提供系统设置功能：
 * - WiFi配网
 * - 声音开关
 * - 固件版本显示
 * - 检查更新
 * - 恢复出厂设置
 */
class SettingsCard : public Card {
public:
  SettingsCard(EPD_Driver &epd, ConfigManager &config,
               WiFiProvisioning &wifiProv, OTAManager &otaManager);
  ~SettingsCard();

  // 生命周期
  void onEnter() override;
  void onExit() override;

  // 事件处理
  void onEvent(const Event &event) override;

  // 渲染
  void render(uint8_t *framebuffer, size_t size) override;

  // 元数据
  String getName() const override { return "设置"; }
  String getCategory() const override { return "系统"; }
  String getLogoPath() const override { return "/icons/card_settings.bin"; }
  int getOrder() const override { return 999; } // 最后一张卡片

  /**
   * @brief 更新方法（处理配网流程）
   */
  void update();

private:
  EPD_Driver &_epd;
  ConfigManager &_config;
  WiFiProvisioning &_wifiProv;
  OTAManager &_otaManager;

  // 菜单项
  enum MenuItem {
    MENU_WIFI_CONFIG = 0, // WiFi配网
    MENU_SOUND_TOGGLE,    // 声音开关
    MENU_FIRMWARE_INFO,   // 固件版本
    MENU_CHECK_UPDATE,    // 检查更新
    MENU_FACTORY_RESET,   // 恢复出厂设置
    MENU_COUNT
  };

  // 状态
  enum State {
    STATE_MENU,             // 菜单模式
    STATE_WIFI_PROVISIONING, // WiFi配网模式
    STATE_INFO_DISPLAY      // 信息显示模式（固件信息、更新检查等）
  };

  // 信息显示子状态
  enum InfoDisplayType {
    INFO_FIRMWARE,          // 固件信息（短按返回）
    INFO_UPDATE_AVAILABLE,  // 有更新可用（短按更新，长按取消）
    INFO_NO_UPDATE,         // 无更新（短按返回）
    INFO_FACTORY_RESET      // 恢复出厂设置确认（长按确认，短按取消）
  };

  State _state;
  InfoDisplayType _infoDisplayType; // 当前信息显示类型
  int _selectedIndex; // 当前选中的菜单项
  bool _needsRender;  // 是否需要重新渲染
  int _partialRefreshCount; // 局部刷新计数器

  /**
   * @brief 渲染菜单界面
   * @param forceDeep 是否强制深度刷新
   */
  // 缓存配置
  bool _soundEnabled;

  void _renderMenu(bool forceDeep = false);

  /**
   * @brief 绘制菜单标题和分割线
   * @param d EPD显示对象
   */
  static void _drawMenuTitle(EPD_Class &d);

  /**
   * @brief 绘制单个菜单项
   * @param d EPD显示对象
   * @param index 菜单项索引
   * @param y Y坐标
   * @param selectedIndex 当前选中的索引
   * @param soundEnabled 声音开关状态
   */
  static void _drawMenuItem(EPD_Class &d, int index, int y, int selectedIndex, bool soundEnabled);

  /**
   * @brief 绘制滚动指示器
   * @param d EPD显示对象
   * @param startIndex 起始索引
   * @param endIndex 结束索引
   * @param menuCount 菜单项总数
   */
  static void _drawScrollIndicators(EPD_Class &d, int startIndex, int endIndex, int menuCount);

  /**
   * @brief 获取菜单项文本
   * @param index 菜单项索引
   * @return 菜单项文本
   */
  static const char* _getMenuText(int index);

  /**
   * @brief 渲染菜单项
   * @param index 菜单项索引
   * @param y Y坐标
   * @param selected 是否选中
   */
  void _renderMenuItem(int index, int y, bool selected);

  /**
   * @brief 获取菜单项文本
   * @param index 菜单项索引
   * @return 菜单项文本
   */
  String _getMenuItemText(int index);

  /**
   * @brief 获取菜单项值（如开关状态）
   * @param index 菜单项索引
   * @return 菜单项值
   */
  String _getMenuItemValue(int index);

  /**
   * @brief 执行菜单项操作
   * @param index 菜单项索引
   */
  void _executeMenuItem(int index);

  /**
   * @brief 进入WiFi配网模式
   */
  void _enterWiFiProvisioning();

  /**
   * @brief 退出WiFi配网模式
   */
  void _exitWiFiProvisioning();

  /**
   * @brief 切换声音开关
   */
  void _toggleSound();

  /**
   * @brief 显示固件信息
   */
  void _showFirmwareInfo();

  /**
   * @brief 检查固件更新
   */
  void _checkUpdate();

  /**
   * @brief 恢复出厂设置
   */
  void _factoryReset();

  /**
   * @brief 显示确认对话框
   * @param message 提示信息
   * @return true 确认，false 取消
   */
  bool _showConfirmDialog(const String &message);
};

#endif // SETTINGS_CARD_H
