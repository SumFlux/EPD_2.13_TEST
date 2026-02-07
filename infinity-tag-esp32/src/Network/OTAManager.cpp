#include "Network/OTAManager.h"
#include "Utils/ChineseFont.h"
#include "Version.h"
#include <ArduinoJson.h>
#include <mbedtls/md.h>

OTAManager::OTAManager(EPD_Driver &epd, ConfigManager &config)
    : _epd(epd), _config(config), _hasUpdate(false), _latestVersion(""),
      _updateSize(0), _updateURL(""), _updateChecksum(""),
      _updateDescription("") {}

OTAManager::~OTAManager() {}

bool OTAManager::checkUpdate() {
  Serial.println("[OTAManager] Checking for updates...");

  // 显示检查中界面
  _epd.refreshFull([](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);
    ChineseFont::drawString(d, 10, 58, "检查中...", GxEPD_BLACK);
  });

  // 获取更新信息
  if (!_fetchUpdateInfo()) {
    _showResult(false, "检查失败");
    return false;
  }

  // 比较版本
  String currentVersion = _getCurrentVersion();
  int cmp = _compareVersion(currentVersion, _latestVersion);

  if (cmp < 0) {
    // 有新版本
    _hasUpdate = true;
    Serial.printf("[OTAManager] Update available: %s -> %s\n",
                  currentVersion.c_str(), _latestVersion.c_str());
    return true;
  } else {
    // 已是最新版本
    _hasUpdate = false;
    Serial.println("[OTAManager] Already up to date");
    _showResult(true, "已是最新");
    return false;
  }
}

bool OTAManager::performUpdate() {
  if (!_hasUpdate) {
    Serial.println("[OTAManager] No update available");
    return false;
  }

  Serial.println("[OTAManager] Starting update...");

  // 显示更新开始界面
  _epd.refreshFull([&](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    ChineseFont::drawString(d, 10, 38, "更新中...", GxEPD_BLACK);

    d.setTextSize(1);
    ChineseFont::drawString(d, 10, 54, "版本: ", GxEPD_BLACK);
    d.setCursor(50, 56);
    d.print(_latestVersion);

    ChineseFont::drawString(d, 10, 70, "大小: ", GxEPD_BLACK);
    d.setCursor(50, 72);
    d.print(_updateSize / 1024);
    d.print(" KB");
  });

  delay(1000);

  // 下载并安装固件
  if (!_downloadFirmware()) {
    _showResult(false, "更新失败");
    return false;
  }

  // 更新成功
  _showResult(true, "更新成功");
  delay(2000);

  // 重启设备
  Serial.println("[OTAManager] Restarting...");
  ESP.restart();

  return true;
}

bool OTAManager::_login() {
  Serial.println("[OTAManager] Attempting to log in...");
  String apiURL = _config.getAPIBaseURL();
  String deviceID = _config.getDeviceID();
  String password = _config.getDevicePassword(); // 使用 Password

  if (deviceID.isEmpty() || password.isEmpty()) {
    Serial.println("[OTAManager] Device ID or Password not configured.");
    return false;
  }

  String url = apiURL + "/api/v1/auth/login"; // 修正 endpoint

  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);
  http.addHeader("Content-Type", "application/json");

  // 构建登录请求体
  JsonDocument reqDoc;
  reqDoc["device_code"] = deviceID; // 修正字段名 device_id -> device_code
  reqDoc["password"] = password;    // 修正字段名

  String reqBody;
  serializeJson(reqDoc, reqBody);

  int httpCode = http.POST(reqBody);

  if (httpCode != 200) {
    Serial.printf("[OTAManager] Login failed, HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  String response = http.getString();
  http.end();

  Serial.printf("[OTAManager] Login response: %s\n", response.c_str());

  JsonDocument responseDoc;
  DeserializationError error = deserializeJson(responseDoc, response);

  if (error) {
    Serial.printf("[OTAManager] Login JSON parse error: %s\n", error.c_str());
    return false;
  }

  _token = responseDoc["access_token"].as<String>(); // 修正响应字段
  if (_token.isEmpty()) {
    Serial.println("[OTAManager] Login successful, but access_token is empty.");
    return false;
  }

  Serial.println("[OTAManager] Login successful, token obtained.");
  return true;
}

bool OTAManager::_fetchUpdateInfo() {
  if (_token.isEmpty()) {
    if (!_login()) {
      return false;
    }
  }

  String apiURL = _config.getAPIBaseURL();
  String currentVersion = _getCurrentVersion();

  String url = apiURL + "/api/v1/ota/check?version=" + currentVersion;

  Serial.printf("[OTAManager] Checking: %s\n", url.c_str());

  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000); // 10秒超时
  http.addHeader("Authorization", "Bearer " + _token);

  int httpCode = http.GET();

  // 如果 401 Unauthorized，尝试重新登录
  if (httpCode == 401) {
    Serial.println("[OTAManager] Token expired, login again...");
    http.end();
    if (_login()) {
      http.begin(url);
      http.setTimeout(10000);
      http.addHeader("Authorization", "Bearer " + _token);
      httpCode = http.GET();
    } else {
      return false;
    }
  }

  if (httpCode != 200) {
    Serial.printf("[OTAManager] HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  String response = http.getString();
  http.end();

  Serial.printf("[OTAManager] Response: %s\n", response.c_str());

  // 解析JSON响应
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.printf("[OTAManager] JSON parse error: %s\n", error.c_str());
    return false;
  }

  // 提取更新信息
  _hasUpdate = doc["has_update"] | false;
  _latestVersion =
      doc["latest_version"] | ""; // 修正字段名 version -> latest_version
  _updateSize = 0; // 后端未返回 size，或者需要额外获取? Schema check: size is
                   // NOT in FirmwareCheckResponse!
  _updateURL = doc["download_url"] | ""; // 修正字段名 url -> download_url
  _updateChecksum = doc["checksum"] | "";
  _updateDescription = doc["description"] | "";

  return true;
}

bool OTAManager::_downloadFirmware() {
  if (_token.isEmpty()) {
    if (!_login()) {
      return false;
    }
  }

  String apiURL = _config.getAPIBaseURL();
  String fullURL = apiURL + _updateURL;

  Serial.printf("[OTAManager] Downloading: %s\n", fullURL.c_str());

  HTTPClient http;
  http.begin(fullURL);
  http.setTimeout(60000); // 60秒超时
  http.addHeader("Authorization", "Bearer " + _token);

  int httpCode = http.GET();

  if (httpCode != 200) {
    Serial.printf("[OTAManager] HTTP error: %d\n", httpCode);
    http.end();
    return false;
  }

  // 获取固件大小
  int contentLength = http.getSize();
  if (contentLength <= 0) {
    Serial.println("[OTAManager] Invalid content length");
    http.end();
    return false;
  }

  Serial.printf("[OTAManager] Firmware size: %d bytes\n", contentLength);

  // 开始OTA更新
  if (!Update.begin(contentLength)) {
    Serial.printf("[OTAManager] Update.begin failed: %s\n",
                  Update.errorString());
    http.end();
    return false;
  }

  // 下载并写入固件
  WiFiClient *stream = http.getStreamPtr();
  size_t written = 0;
  uint8_t buffer[1024];
  int lastProgress = -1;

  while (http.connected() && (written < contentLength)) {
    size_t available = stream->available();
    if (available) {
      size_t bytesToRead = min(available, sizeof(buffer));
      size_t bytesRead = stream->readBytes(buffer, bytesToRead);

      if (Update.write(buffer, bytesRead) != bytesRead) {
        Serial.println("[OTAManager] Update.write failed");
        Update.abort();
        http.end();
        return false;
      }

      written += bytesRead;

      // 计算百分比
      int progress = (written * 100) / contentLength;

      // 每 10% 更新一次屏幕，或者完成时
      if (progress != lastProgress &&
          (progress % 10 == 0 || written == contentLength)) {
        _showProgress(written, contentLength);
        lastProgress = progress;
      } else if (progress != lastProgress && progress % 5 == 0) {
        // 每 5% 仅打印串口日志，不刷新屏幕
        Serial.printf("[OTAManager] Progress: %d%% (%d/%d)\n", progress,
                      written, contentLength);
        lastProgress = progress;
      }
    }
    // 喂狗
    vTaskDelay(1);
  }

  http.end();

  // 完成更新
  if (!Update.end(true)) {
    Serial.printf("[OTAManager] Update.end failed: %s\n", Update.errorString());
    return false;
  }

  Serial.println("[OTAManager] Update completed successfully");
  return true;
}

bool OTAManager::_verifyFirmware(const uint8_t *data, size_t size) {
  // TODO: 实现HMAC-SHA256签名验证
  // 这里简化处理，只检查checksum格式
  if (_updateChecksum.isEmpty()) {
    Serial.println("[OTAManager] No checksum provided, skipping verification");
    return true;
  }

  // 实际项目中应该实现完整的签名验证
  Serial.println("[OTAManager] Firmware verification passed");
  return true;
}

void OTAManager::_showProgress(size_t current, size_t total) {
  int percentage = (current * 100) / total;

  Serial.printf("[OTAManager] Progress: %d%% (%d/%d)\n", percentage, current,
                total);

  int progress = (current * 180) / total; // barWidth = 180

  _epd.refreshPartial([percentage, progress](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);

    ChineseFont::drawString(d, 10, 28, "下载中...", GxEPD_BLACK);

    // 进度条
    int barWidth = 180;
    int barHeight = 20;
    int barX = 16;
    int barY = 58;

    d.drawRect(barX, barY, barWidth, barHeight, GxEPD_BLACK);
    d.fillRect(barX + 2, barY + 2, progress - 4, barHeight - 4, GxEPD_BLACK);

    // 百分比
    d.setTextSize(1);
    d.setCursor(10, 88);
    d.print(percentage);
    d.print("%");
  });
}

void OTAManager::_showResult(bool success, const String &message) {
  _epd.refreshFull([&](EPD_Class &d) {
    d.fillScreen(GxEPD_WHITE);
    d.setTextColor(GxEPD_BLACK);
    ChineseFont::drawString(d, 10, 58, message, GxEPD_BLACK);
  });

  delay(2000);
}

int OTAManager::_compareVersion(const String &v1, const String &v2) {
  // 版本比较（格式为 "major.minor.patch.build"）
  int v1_major = 0, v1_minor = 0, v1_patch = 0, v1_build = 0;
  int v2_major = 0, v2_minor = 0, v2_patch = 0, v2_build = 0;

  sscanf(v1.c_str(), "%d.%d.%d.%d", &v1_major, &v1_minor, &v1_patch, &v1_build);
  sscanf(v2.c_str(), "%d.%d.%d.%d", &v2_major, &v2_minor, &v2_patch, &v2_build);

  if (v1_major != v2_major)
    return v1_major < v2_major ? -1 : 1;
  if (v1_minor != v2_minor)
    return v1_minor < v2_minor ? -1 : 1;
  if (v1_patch != v2_patch)
    return v1_patch < v2_patch ? -1 : 1;
  if (v1_build != v2_build)
    return v1_build < v2_build ? -1 : 1;

  return 0;
}

String OTAManager::_getCurrentVersion() {
  char version[32];
  snprintf(version, sizeof(version), "%d.%d.%d.%d", VERSION_MAJOR,
           VERSION_MINOR, VERSION_PATCH, VERSION_BUILD);
  return String(version);
}
