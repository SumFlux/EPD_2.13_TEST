#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <vector>

/**
 * @brief 卡片配置结构体
 */
struct CardConfig {
  String name;       // 卡片名称（如"黄历"）
  String category;   // 分类名称（如"决策站"）
  String scriptPath; // Lua脚本路径（如"/cards/calendar.lua"）
  String logoPath;   // Logo图标路径（如"/icons/card_calendar.bin"）
  int order;         // 显示顺序
  bool enabled;      // 是否启用

  CardConfig()
      : name(""), category(""), scriptPath(""), logoPath(""), order(0),
        enabled(true) {}

  CardConfig(const String &n, const String &c, const String &s, const String &l,
             int o = 0, bool e = true)
      : name(n), category(c), scriptPath(s), logoPath(l), order(o), enabled(e) {
  }
};

/**
 * @brief 配置管理器
 *
 * 基于ESP32 NVS（Non-Volatile Storage）存储配置
 * 管理WiFi配置、设备凭证、卡片列表等
 */
class ConfigManager {
public:
  ConfigManager();
  ~ConfigManager();

  /**
   * @brief 初始化配置管理器
   * @return true 成功，false 失败
   */
  bool begin();

  // ==========================================
  // WiFi 配置
  // ==========================================

  String getWiFiSSID();
  void setWiFiSSID(const String &ssid);

  String getWiFiPassword();
  void setWiFiPassword(const String &password);

  bool hasWiFiConfig();

  // ==========================================
  // 设备配置
  // ==========================================

  String getDeviceID();
  void setDeviceID(const String &deviceId);

  String getDevicePassword();
  String getDeviceSecret() { return getDevicePassword(); }
  void setDevicePassword(const String &password);

  String getAPIBaseURL();
  void setAPIBaseURL(const String &url);

  // ==========================================
  // 用户设置
  // ==========================================

  bool getSoundEnabled();
  void setSoundEnabled(bool enabled);

  bool getVibrateEnabled();
  void setVibrateEnabled(bool enabled);

  // ==========================================
  // 卡片配置
  // ==========================================

  /**
   * @brief 获取卡片列表
   * @return 卡片配置列表
   */
  std::vector<CardConfig> getCardList();

  /**
   * @brief 更新卡片列表
   * @param cards 新的卡片配置列表
   */
  void updateCardList(const std::vector<CardConfig> &cards);

  /**
   * @brief 添加卡片
   * @param card 要添加的卡片配置
   */
  void addCard(const CardConfig &card);

  /**
   * @brief 移除卡片
   * @param name 卡片名称
   */
  void removeCard(const String &name);

  /**
   * @brief 获取卡片数量
   * @return 卡片数量
   */
  int getCardCount();

  // ==========================================
  // 系统配置
  // ==========================================

  /**
   * @brief 获取当前选中的卡片索引
   * @return 卡片索引
   */
  int getCurrentCardIndex();

  /**
   * @brief 设置当前选中的卡片索引
   * @param index 卡片索引
   */
  void setCurrentCardIndex(int index);

  /**
   * @brief 检查是否首次启动
   * @return true 首次启动，false 非首次启动
   */
  bool isFirstBoot();

  /**
   * @brief 设置首次启动标志
   * @param firstBoot true 首次启动，false 非首次启动
   */
  void setFirstBoot(bool firstBoot);

  /**
   * @brief 恢复出厂设置
   *
   * 清空所有配置，恢复到初始状态
   */
  void factoryReset();

private:
  Preferences _prefs;

  // NVS 命名空间
  static const char *NAMESPACE_WIFI;
  static const char *NAMESPACE_DEVICE;
  static const char *NAMESPACE_SETTINGS;
  static const char *NAMESPACE_CARDS;
  static const char *NAMESPACE_SYSTEM;

  // 辅助方法
  String _getCardKey(int index, const char *suffix);
};

#endif // CONFIG_MANAGER_H
