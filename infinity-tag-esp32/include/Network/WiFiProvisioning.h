#ifndef WIFI_PROVISIONING_H
#define WIFI_PROVISIONING_H

#include "Core/ConfigManager.h"
#include "Driver/EPD_Driver.h"
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>


/**
 * @brief WiFi配网管理器（工具类）
 *
 * 不是卡片，而是由SettingsCard调用的配网工具
 * 启动AP模式，显示二维码和WiFi信息，提供Captive Portal
 */
class WiFiProvisioning {
public:
  WiFiProvisioning(EPD_Driver &epd, ConfigManager &config);
  ~WiFiProvisioning();

  /**
   * @brief 启动配网流程
   * @return true 成功，false 失败
   */
  bool start();

  /**
   * @brief 停止配网流程
   */
  void stop();

  /**
   * @brief 更新方法（处理DNS和Web请求）
   *
   * 必须在主循环中调用以处理Captive Portal
   */
  void update();

  /**
   * @brief 检查是否正在配网
   * @return true 正在配网，false 未配网
   */
  bool isProvisioning() const { return _isAPStarted; }

  /**
   * @brief 检查配网是否完成
   * @return true 已完成，false 未完成
   */
  bool isConfigured() const { return _isConfigured; }

private:
  EPD_Driver &_epd;
  ConfigManager &_config;

  // AP模式配置
  String _apSSID;
  String _apPassword;
  IPAddress _apIP;

  // Web服务器和DNS服务器
  WebServer *_webServer;
  DNSServer *_dnsServer;

  // 状态
  bool _isConfigured;
  bool _isAPStarted;

  // 常量
  static const int DNS_PORT = 53;
  static const int WEB_PORT = 80;

  /**
   * @brief 启动AP模式
   * @return true 成功，false 失败
   */
  bool _startAP();

  /**
   * @brief 停止AP模式
   */
  void _stopAP();

  /**
   * @brief 生成随机AP密码
   * @return 8位随机密码
   */
  String _generatePassword();

  /**
   * @brief 生成WiFi二维码数据
   * @return 二维码字符串（WIFI:T:WPA;S:xxx;P:xxx;;）
   */
  String _generateQRCodeData();

  /**
   * @brief 渲染配网界面
   *
   * 左侧：二维码
   * 右侧：WiFi名称和密码
   */
  void _renderConfigUI();

  /**
   * @brief 渲染二维码
   * @param d Display引用
   * @param x X坐标
   * @param y Y坐标
   * @param data 二维码数据
   */
  void _renderQRCode(EPD_Class &d, int x, int y, const String &data);

  /**
   * @brief 设置Web服务器路由
   */
  void _setupWebServer();

  /**
   * @brief 处理根路径请求
   */
  void _handleRoot();

  /**
   * @brief 处理配置提交
   */
  void _handleConfig();

  /**
   * @brief 处理404
   */
  void _handleNotFound();

  /**
   * @brief 获取配置页面HTML
   * @return HTML字符串
   */
  String _getConfigPageHTML();
};

#endif // WIFI_PROVISIONING_H
