#include "Network/WiFiProvisioning.h"
#include "Utils/ChineseFont.h"
#include <qrcode.h>

WiFiProvisioning::WiFiProvisioning(EPD_Driver &epd, ConfigManager &config)
    : _epd(epd), _config(config), _webServer(nullptr), _dnsServer(nullptr),
      _isConfigured(false), _isAPStarted(false) {}

WiFiProvisioning::~WiFiProvisioning() { stop(); }

bool WiFiProvisioning::start() {
  Serial.println("[WiFiProvisioning] Starting provisioning...");

  // 生成AP配置
  uint32_t chipId = (uint32_t)ESP.getEfuseMac();
  _apSSID = "InfinityTag-" + String(chipId & 0xFFFF, HEX);
  _apPassword = _generatePassword();
  _apIP = IPAddress(192, 168, 4, 1);

  // 启动AP模式
  if (_startAP()) {
    _renderConfigUI();
    return true;
  } else {
    Serial.println("[WiFiProvisioning] ERROR: Failed to start AP");
    return false;
  }
}

void WiFiProvisioning::stop() {
  Serial.println("[WiFiProvisioning] Stopping provisioning...");
  _stopAP();
}

void WiFiProvisioning::update() {
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

bool WiFiProvisioning::_startAP() {
  Serial.println("[WiFiProvisioning] Starting AP mode...");

  // 断开现有WiFi连接
  WiFi.disconnect(true);
  delay(100);

  // 启动AP模式
  WiFi.mode(WIFI_AP);
  delay(100);

  bool success = WiFi.softAP(_apSSID.c_str(), _apPassword.c_str());
  if (!success) {
    Serial.println("[WiFiProvisioning] ERROR: Failed to start AP");
    return false;
  }

  // 配置AP IP
  WiFi.softAPConfig(_apIP, _apIP, IPAddress(255, 255, 255, 0));

  Serial.printf("[WiFiProvisioning] AP started: %s\n", _apSSID.c_str());
  Serial.printf("[WiFiProvisioning] AP password: %s\n", _apPassword.c_str());
  Serial.printf("[WiFiProvisioning] AP IP: %s\n", _apIP.toString().c_str());

  // 启动DNS服务器（Captive Portal）
  _dnsServer = new DNSServer();
  _dnsServer->start(DNS_PORT, "*", _apIP);

  // 启动Web服务器
  _webServer = new WebServer(WEB_PORT);
  _setupWebServer();
  _webServer->begin();

  Serial.println("[WiFiProvisioning] Web server started");

  _isAPStarted = true;
  return true;
}

void WiFiProvisioning::_stopAP() {
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

  Serial.println("[WiFiProvisioning] AP stopped");
}

String WiFiProvisioning::_generatePassword() {
  // 生成8位随机密码
  String password = "";
  const char charset[] = "0123456789";
  for (int i = 0; i < 8; i++) {
    password += charset[random(0, strlen(charset))];
  }
  return password;
}

String WiFiProvisioning::_generateQRCodeData() {
  // WiFi二维码格式：WIFI:T:WPA;S:SSID;P:PASSWORD;;
  return "WIFI:T:WPA;S:" + _apSSID + ";P:" + _apPassword + ";;";
}

void WiFiProvisioning::_renderConfigUI() {
  Serial.println("[WiFiProvisioning] Rendering config UI");

  String qrData = _generateQRCodeData();

  _epd.refreshFull([&](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    // 标题（使用中文字体，左上角坐标）
    ChineseFont::drawString(d, 10, 28, "WiFi配置", GxEPD_BLACK);

    // 左侧：二维码（80x80）
    _renderQRCode(d, 10, 45, qrData);

    // 右侧：WiFi信息
    d.setTextSize(1);
    d.setCursor(100, 50);
    d.print("SSID:");
    d.setCursor(100, 62);
    d.print(_apSSID);

    ChineseFont::drawString(d, 100, 76, "密码:", GxEPD_BLACK);
    d.setCursor(100, 90);
    d.print(_apPassword);

    ChineseFont::drawString(d, 100, 106, "扫描二维码", GxEPD_BLACK);
  });

  Serial.println("[WiFiProvisioning] Config UI rendered");
}

void WiFiProvisioning::_renderQRCode(EPD_Class &d, int x, int y,
                                     const String &data) {
  // 使用qrcode库生成二维码
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, data.c_str());

  // 计算缩放比例（80x80像素）
  int scale = 80 / qrcode.size;
  if (scale < 1)
    scale = 1;

  // 绘制二维码
  for (uint8_t qy = 0; qy < qrcode.size; qy++) {
    for (uint8_t qx = 0; qx < qrcode.size; qx++) {
      if (qrcode_getModule(&qrcode, qx, qy)) {
        d.fillRect(x + qx * scale, y + qy * scale, scale, scale, GxEPD_BLACK);
      }
    }
  }
}

void WiFiProvisioning::_setupWebServer() {
  // 绑定路由
  _webServer->on("/", [this]() { _handleRoot(); });
  _webServer->on("/config", HTTP_POST, [this]() { _handleConfig(); });
  _webServer->onNotFound([this]() { _handleNotFound(); });
}

void WiFiProvisioning::_handleRoot() {
  Serial.println("[WiFiProvisioning] Serving config page");
  _webServer->send(200, "text/html", _getConfigPageHTML());
}

void WiFiProvisioning::_handleConfig() {
  Serial.println("[WiFiProvisioning] Received config submission");

  if (!_webServer->hasArg("ssid") || !_webServer->hasArg("password")) {
    _webServer->send(400, "text/plain", "Missing parameters");
    return;
  }

  String ssid = _webServer->arg("ssid");
  String password = _webServer->arg("password");

  Serial.printf("[WiFiProvisioning] SSID: %s\n", ssid.c_str());
  Serial.println("[WiFiProvisioning] Password: [hidden]");

  // 保存配置到NVS
  _config.setWiFiSSID(ssid);
  _config.setWiFiPassword(password);
  _config.setFirstBoot(false);

  _isConfigured = true;

  // 发送成功响应
  String html = "<html><body>";
  html += "<h1>Configuration Saved!</h1>";
  html += "<p>Device will restart in 3 seconds...</p>";
  html += "<script>setTimeout(function(){window.location.href='/';}, "
          "3000);</script>";
  html += "</body></html>";

  _webServer->send(200, "text/html", html);

  // 延迟重启
  delay(3000);
  ESP.restart();
}

void WiFiProvisioning::_handleNotFound() {
  // Captive Portal：重定向所有未知请求到根路径
  _webServer->sendHeader("Location", "/", true);
  _webServer->send(302, "text/plain", "");
}

String WiFiProvisioning::_getConfigPageHTML() {
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
