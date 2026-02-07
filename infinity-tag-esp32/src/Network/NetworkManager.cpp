#include "Network/NetworkManager.h"

void NetworkManager::begin(const char* ssid, const char* password) {
    _ssid = String(ssid);
    _password = String(password);

    // 断开之前的连接
    WiFi.disconnect(true);
    delay(100);

    // 设置 WiFi 模式为 Station
    WiFi.mode(WIFI_STA);
    delay(100);

    // 设置主机名（可选，帮助调试）
    WiFi.setHostname("InfinityTag");

    // 开始连接
    Serial.print("Connecting to WiFi: ");
    Serial.println(_ssid);
    WiFi.begin(_ssid.c_str(), _password.c_str());
}

bool NetworkManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool NetworkManager::waitForConnection(uint32_t timeout_ms) {
    unsigned long startTime = millis();
    int dotCount = 0;

    while (!isConnected()) {
        // 检查超时
        if (millis() - startTime > timeout_ms) {
            Serial.println("\nWiFi connection timeout!");
            Serial.print("WiFi status: ");
            Serial.println(WiFi.status());
            return false;
        }

        // 喂狗（防止看门狗重启）
        yield();

        // 每秒打印一次状态
        if (dotCount % 2 == 0) {
            wl_status_t status = WiFi.status();
            Serial.print(" [");
            Serial.print(status);
            Serial.print("]");

            // 如果连接失败，提前返回
            if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
                Serial.println("\nWiFi connection failed!");
                Serial.print("Status: ");
                Serial.println(status);
                return false;
            }
        }

        Serial.print(".");
        dotCount++;
        delay(500);
    }

    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(getLocalIP());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    return true;
}

String NetworkManager::getLocalIP() {
    return WiFi.localIP().toString();
}
