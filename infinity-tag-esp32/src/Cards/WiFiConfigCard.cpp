#include "Cards/WiFiConfigCard.h"
#include <qrcode.h>

WiFiConfigCard::WiFiConfigCard(EPD_Driver& epd, ConfigManager& config)
    : _epd(epd), _config(config),
      _webServer(nullptr), _dnsServer(nullptr),
      _isConfigured(false), _isAPStarted(false) {
}

WiFiConfigCard::~WiFiConfigCard() {
    _stopAP();
}

void WiFiConfigCard::onEnter() {
    Serial.println("[WiFiConfigCard] Entering WiFi config mode");

    // 生成AP配置
    uint32_t chipId = (uint32_t)ESP.getEfuseMac();
    _apSSID = "InfinityTag-" + String(chipId & 0xFFFF, HEX);
    _apPassword = _generatePassword();
    _apIP = IPAddress(192, 168, 4, 1);

    // 启动AP模式
    if (_startAP()) {
        _renderConfigUI();
    } else {
        Serial.println("[WiFiConfigCard] ERROR: Failed to start AP");
    }
}

void WiFiConfigCard::onExit() {
    Serial.println("[WiFiConfigCard] Exiting WiFi config mode");
    _stopAP();
}

void WiFiConfigCard::onEvent(const Event& event) {
    // WiFi配网卡片不处理输入事件
    // 配置完成后会自动重启设备
}

void WiFiConfigCard::render(uint8_t* framebuffer, size_t size) {
    // WiFi配网卡片使用直接渲染，不使用framebuffer
    // 因为需要显示二维码等复杂内容
}

void WiFiConfigCard::update() {
    if (!_isAPStarted) {
        return;
    }

    // 处理DNS请求（Captive Portal）
    if (_dnsServer) {
        _dnsServer->processNextRequest();
    }

    // 处理Web请求
    if (_webServer) {
        _webServer->handleClient();
    }
}

bool WiFiConfigCard::_startAP() {
    Serial.println("[WiFiConfigCard] Starting AP mode...");

    // 断开现有WiFi连接
    WiFi.disconnect(true);
    delay(100);

    // 启动AP模式
    WiFi.mode(WIFI_AP);
    delay(100);

    bool success = WiFi.softAP(_apSSID.c_str(), _apPassword.c_str());
    if (!success) {
        Serial.println("[WiFiConfigCard] ERROR: Failed to start AP");
        return false;
    }

    // 配置AP IP
    WiFi.softAPConfig(_apIP, _apIP, IPAddress(255, 255, 255, 0));

    Serial.printf("[WiFiConfigCard] APted: %s\n", _apSSID.c_str());
    Serial.printf("[WiFiConfigCard] AP password: %s\n", _apPassword.c_str());
    Serial.printf("[WiFiConfigCard] AP IP: %s\n", _apIP.toString().c_str());

    // 启动DNS服务器（Captive Portal）
    _dnsServer = new DNSServer();
    _dnsServer->start(DNS_PORT, "*", _apIP);

    // 启动Web服务器
    _webServer = new WebServer(WEB_PORT);
    _setupWebServer();
    _webServer->begin();

    Serial.println("[WiFiConfigCard] Web server started");

    _isAPStarted = true;
    return true;
}

void WiFiConfigCard::_stopAP() {
    if (_webServer) {
        _webServer->stop();
        delete _webServer;
        _webServer = nullptr;
    }

    if (_dnsServer) {
        _dnsServer->stop();
        delete _dnsServer;
        _dnsServer = nullptr;
    }

    WiFi.softAPdisconnect(true);
    _isAPStarted = false;

    Serial.println("[WiFiConfigCard] AP stopped");
}

String WiFiConfigCard::_generatePassword() {
    // 生成8位随机密码
    String password = "";
    const char charset[] = "0123456789";
    for (int i = 0; i < 8; i++) {
        password += charset[random(0, strlen(charset))];
    }
    return password;
}

String WiFiConfigCard::_generateQRCodeData() {
    // WiFi二维码格式：WIFI:T:WPA;S:SSID;P:PASSWORD;;
    return "WIFI:T:WPA;S:" + _apSSID + ";P:" + _apPassword + ";;";
}

void WiFiConfigCard::_renderConfigUI() {
    Serial.println("[WiFiConfigCard] Rendering config UI");

    _epd.getDisplay().setFullWindow();
    _epd.getDisplay().firstPage();
    do {
        _epd.getDisplay().fillScreen(GxEPD_WHITE);
        _epd.getDisplay().setTextColor(GxEPD_BLACK);

        // 标题
        _epd.getDisplay().setTextSize(2);
        _epd.getDisplay().setCursor(10, 28);
        _epd.getDisplay().print("WiFi Setup");

        // 左侧：二维码（80x80）
        String qrData = _generateQRCodeData();
        _renderQRCode(10, 45, qrData);

        // 右侧：WiFi信息
        _epd.getDisplay().setTextSize(1);
        _epd.getDisplay().setCursor(100, 50);
        _epd.getDisplay().print("SSID:");
        _epd.getDisplay().setCursor(100, 62);
        _epd.getDisplay().print(_apSSID);

        _epd.getDisplay().setCursor(100, 78);
        _epd.getDisplay().print("Password:");
        _epd.getDisplay().setCursor(100, 90);
        _epd.getDisplay().print(_apPassword);

        _epd.getDisplay().setCursor(100, 106);
        _epd.getDisplay().print("Scan QR code");

    } while (_epd.getDisplay().nextPage());

    Serial.println("[WiFiConfigCard] Config UI rendered");
}

void WiFiConfigCard::_renderQRCode(int x, int y, const String& data) {
    // 使用qrcode库生成二维码
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, data.c_str());

    // 计算缩放比例（80x80像素）
    int scale = 80 / qrcode.size;
    if (scale < 1) scale = 1;

    // 绘制二维码
    for (uint8_t qy = 0; qy < qrcode.size; qy++) {
        for (uint8_t qx = 0; qx < qrcode.size; qx++) {
            if (qrcode_getModule(&qrcode, qx, qy)) {
                // 黑色模块
                _epd.getDisplay().fillRect(
                    x + qx * scale,
                    y + qy * scale,
                    scale, scale,
                    GxEPD_BLACK
                );
            }
        }
    }
}

void WiFiConfigCard::_setupWebServer() {
    // 绑定路由
    _webServer->on("/", [this]() { _handleRoot(); });
    _webServer->on("/config", HTTP_POST, [this]() { _handleConfig(); });
    _webServer->onNotFound([this]() { _handleNotFound(); });
}

void WiFiConfigCard::_handleRoot() {
    Serial.println("[WiFiConfigCard] Serving config page");
    _webServer->send(200, "text/html", _getConfigPageHTML());
}

void WiFiConfigCard::_handleConfig() {
    Serial.println("[WiFiConfigCard] Received config submission");

    if (!_webServer->hasArg("ssid") || !_webServer->hasArg("password")) {
        _webServer->send(400, "text/plain", "Missing parameters");
        return;
    }

    String ssid = _webServer->arg("ssid");
    String password = _webServer->arg("password");

    Serial.printf("[WiFiConfigCard] SSID: %s\n", ssid.c_str());
    Serial.println("[WiFiConfigCard] Password: [hidden]");

    // 保存配置到NVS
    _config.setWiFiSSID(ssid);
    _config.setWiFiPassword(password);
    _config.setFirstBoot(false);

    // 发送成功响应
    String html = "<html><body>";
    html += "<h1>Configuration Saved!</h1>";
    html += "<p>Device will restart in 3 seconds...</p>";
    html += "<script>setTimeout(function(){window.location.href='/';}, 3000);</script>";
    html += "</body></html>";

    _webServer->send(200, "text/html", html);

    // 延迟重启
    delay(3000);
    ESP.restart();
}

void WiFiConfigCard::_handleNotFound() {
    // Captive Portal：重定向所有未知请求到根路径
    _webServer->sendHeader("Location", "/", true);
    _webServer->send(302, "text/plain", "");
}

String WiFiConfigCard::_getConfigPageHTML() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>InfinityTag WiFi Setup</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 400px;
            margin: 50px auto;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            background-color: white;
            padding: 30px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        h1 {
            color: #333;
            text-align: center;
            margin-bottom: 30px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 5px;
            color: #666;
            font-weight: bold;
        }
        input[type="text"],
        input[type="password"] {
            width: 100%;
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
            box-sizing: border-box;
            font-size: 16px;
        }
        button {
            width: 100%;
            padding: 12px;
            background-color: #4CAF50;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 16px;
            cursor: pointer;
            margin-top: 10px;
        }
        button:hover {
            background-color: #45a049;
        }
        .info {
            background-color: #e3f2fd;
            padding: 15px;
            border-radius: 5px;
            margin-bottom: 20px;
            font-size: 14px;
            color: #1976d2;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🔧 InfinityTag Setup</h1>
        <div class="info">
            Please enter your WiFi credentials to connect your device to the internet.
        </div>
        <form action="/config" method="POST">
            <div class="form-group">
                <label for="ssid">WiFi Name (SSID):</label>
                <input type="text" id="ssid" name="ssid" required placeholder="Enter WiFi name">
            </div>
            <div class="form-group">
                <label for="password">WiFi Password:</label>
                <input type="password" id="password" name="password" required placeholder="Enter WiFi password">
            </div>
            <button type="submit">Save & Connect</button>
        </form>
    </div>
</body>
</html>
)";
    return html;
}
