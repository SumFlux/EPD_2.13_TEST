#ifndef WIFI_CONFIG_CARD_H
#define WIFI_CONFIG_CARD_H

#include "Core/Card.h"
#include "Core/ConfigManager.h"
#include "Driver/EPD_Driver.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>

/**
 * @brief WiFi配网卡片
 *
 * 启动AP模式，显示二维码和WiFi信息
 * 提供Captive Portal强制门户进行配置
 */
class WiFiConfigCard : public Card {
public:
    WiFiConfigCard(EPD_Driver& epd, ConfigManager& config);
    ~WiFiConfigCard();

    // 生命周期
    void onEnter() override;
    void onExit() override;

    // 事件处理
    void onEvent(const Event& event) override;

    // 渲染
    void render(uint8_t* framebuffer, size_t size) override;

    /**
     * @brief 更新方法（处理DNS和Web请求）
     *
     * 必须在主循环中调用以处理Captive Portal
     */
    void update();

    // 元数据
    String getName() const override { return "配网"; }
    String getCategory() const override { return "系统"; }
    String getLogoPath() const override { return "/icons/card_wifi.bin"; }
    int getOrder() const override { return 0; }

private:
    EPD_Driver& _epd;
    ConfigManager& _config;

    // AP模式配置
    String _apSSID;
    String _apPassword;
    IPAddress _apIP;

    // Web服务器和DNS服务器
    WebServer* _webServer;
    DNSServer* _dnsServer;

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
     * @param x X坐标
     * @param y Y坐标
     * @param data 二维码数据
     */
    void _renderQRCode(int x, int y, const String& data);

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

#endif // WIFI_CONFIG_CARD_H
