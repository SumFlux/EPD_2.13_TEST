#ifndef SETTINGS_CARD_H
#define SETTINGS_CARD_H

#include "Core/Card.h"
#include "Core/ConfigManager.h"
#include "Driver/EPD_Driver.h"
#include "Network/OTAManager.h"
#include "Network/WiFiProvisioning.h"
#include "Version.h"
#include <vector>


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
