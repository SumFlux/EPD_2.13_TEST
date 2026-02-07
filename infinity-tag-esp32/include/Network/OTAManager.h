#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include "Core/ConfigManager.h"
#include "Driver/EPD_Driver.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>

/**
 * @brief OTA固件更新管理器
 *
 * 功能：
 * - 检查固件更新
 * - 下载固件
 * - 验证签名（HMAC-SHA256）
 * - 写入OTA分区
 * - 显示进度条
 * - 自动重启
 */
class OTAManager {
public:
  OTAManager(EPD_Driver &epd, ConfigManager &config);
  ~OTAManager();

  /**
   * @brief 检查固件更新
   * @return true 有更新，false 无更新或检查失败
   */
  bool checkUpdate();

  /**
   * @brief 执行固件更新
   * @return true 成功，false 失败
   */
  bool performUpdate();

  /**
   * @brief 获取最新版本信息
   * @return 版本字符串（如"1.0.2"）
   */
  String getLatestVersion() const { return _latestVersion; }

  /**
   * @brief 获取更新大小
   * @return 固件大小（字节）
   */
  size_t getUpdateSize() const { return _updateSize; }

  /**
   * @brief 获取更新描述
   * @return 更新描述
   */
  String getUpdateDescription() const { return _updateDescription; }

private:
  EPD_Driver &_epd;
  ConfigManager &_config;

  // 更新信息
  bool _hasUpdate;
  String _latestVersion;
  size_t _updateSize;
  String _updateURL;
  String _updateChecksum;
  String _updateDescription;

  // 鉴权信息
  String _token;

  /**
   * @brief 设备登录获取Token
   * @return true 成功，false 失败
   */
  bool _login();

  /**
   * @brief 从服务器获取更新信息
   * @return true 成功，false 失败
   */
  bool _fetchUpdateInfo();

  /**
   * @brief 下载固件
   * @return true 成功，false 失败
   */
  bool _downloadFirmware();

  /**
   * @brief 验证固件签名
   * @param data 固件数据
   * @param size 数据大小
   * @return true 验证通过，false 验证失败
   */
  bool _verifyFirmware(const uint8_t *data, size_t size);

  /**
   * @brief 显示更新进度
   * @param current 当前进度
   * @param total 总大小
   */
  void _showProgress(size_t current, size_t total);

  /**
   * @brief 显示更新结果
   * @param success 是否成功
   * @param message 消息
   */
  void _showResult(bool success, const String &message);

  /**
   * @brief 比较版本号
   * @param v1 版本1（如"1.0.1"）
   * @param v2 版本2（如"1.0.2"）
   * @return -1: v1<v2, 0: v1==v2, 1: v1>v2
   */
  int _compareVersion(const String &v1, const String &v2);

  /**
   * @brief 获取当前固件版本
   * @return 版本字符串
   */
  String _getCurrentVersion();
};

#endif // OTA_MANAGER_H
