#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WiFi.h>
#include <Arduino.h>

/**
 * @brief WiFi 连接管理器
 *
 * 负责 WiFi 连接初始化、状态检查和连接等待
 */
class NetworkManager {
public:
    /**
     * @brief 初始化 WiFi 连接
     * @param ssid WiFi SSID
     * @param password WiFi 密码
     */
    void begin(const char* ssid, const char* password);

    /**
     * @brief 检查 WiFi 是否已连接
     * @return true 已连接，false 未连接
     */
    bool isConnected();

    /**
     * @brief 阻塞式等待 WiFi 连接
     * @param timeout_ms 超时时间（毫秒），默认 10 秒
     * @return true 连接成功，false 超时失败
     */
    bool waitForConnection(uint32_t timeout_ms = 10000);

    /**
     * @brief 获取本地 IP 地址
     * @return IP 地址字符串
     */
    String getLocalIP();

private:
    String _ssid;
    String _password;
};

#endif // NETWORK_MANAGER_H
