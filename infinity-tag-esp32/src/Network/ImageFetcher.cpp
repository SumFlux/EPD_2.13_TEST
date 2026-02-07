#include "Network/ImageFetcher.h"

ImageFetcher::ImageFetcher(const char *baseUrl) {
  _baseUrl = String(baseUrl);
  _accessToken = "";
}

bool ImageFetcher::authenticate(const char *deviceId, const char *password) {
  HTTPClient http;
  String url = _baseUrl + "/api/v1/auth/login";

  Serial.println("Authenticating...");
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000); // 5 秒超时

  // 构建 JSON 请求体
  JsonDocument doc;
  doc["device_code"] = deviceId; // 后端期望 device_code，不是 device_id
  doc["password"] = password;

  String payload;
  serializeJson(doc, payload);

  // 发送 POST 请求
  int httpCode = http.POST(payload);

  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Authentication successful!");

    // 解析响应 JSON
    JsonDocument responseDoc;
    DeserializationError error = deserializeJson(responseDoc, response);

    if (error) {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }

    // 提取 access_token
    _accessToken = responseDoc["access_token"].as<String>();
    Serial.print("Token: ");
    Serial.println(_accessToken.substring(0, 20) + "...");

    http.end();
    return true;
  } else {
    Serial.print("Authentication failed! HTTP code: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.println(http.getString());
    }
    http.end();
    return false;
  }
}

bool ImageFetcher::fetchImageList(std::vector<ImageInfo> &outList) {
  if (!isAuthenticated()) {
    Serial.println("Error: Not authenticated");
    return false;
  }

  HTTPClient http;
  String url = _baseUrl + "/api/v1/images/";

  Serial.println("Fetching image list...");
  http.begin(url);
  http.addHeader("Authorization", "Bearer " + _accessToken);
  http.setTimeout(5000);

  int httpCode = http.GET();

  if (httpCode == 200) {
    String response = http.getString();

    // 解析 JSON 数组（假设最多 20 张图片）
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }

    // 清空输出列表
    outList.clear();

    // 遍历数组
    JsonArray images = doc.as<JsonArray>();
    for (JsonObject img : images) {
      ImageInfo info;
      info.id = img["id"];
      info.url = img["url"].as<String>();
      info.display_order = img["display_order"];
      outList.push_back(info);
    }

    Serial.print("Fetched ");
    Serial.print(outList.size());
    Serial.println(" images");

    // 打印图片信息
    for (size_t i = 0; i < outList.size(); i++) {
      Serial.print("Image ");
      Serial.print(i);
      Serial.print(": id=");
      Serial.print(outList[i].id);
      Serial.print(", url=");
      Serial.println(outList[i].url);
    }

    http.end();
    return true;
  } else {
    Serial.print("Fetch image list failed! HTTP code: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.println(http.getString());
    }
    http.end();
    return false;
  }
}

bool ImageFetcher::downloadBitmap(int imageId, uint8_t *buffer,
                                  size_t bufferSize) {
  if (!isAuthenticated()) {
    Serial.println("Error: Not authenticated");
    return false;
  }

  if (bufferSize != 2808) {
    Serial.println(
        "Error: Buffer size must be 2808 bytes (row-aligned format)");
    return false;
  }

  HTTPClient http;
  String url = _baseUrl + "/api/v1/images/" + String(imageId) + "/bitmap";

  Serial.print("Downloading bitmap for image ");
  Serial.print(imageId);
  Serial.println("...");

  http.begin(url);
  http.addHeader("Authorization", "Bearer " + _accessToken);
  http.setTimeout(10000); // 10 秒超时（下载可能较慢）

  int httpCode = http.GET();

  if (httpCode == 200) {
    // 验证 Content-Length
    int contentLength = http.getSize();
    if (contentLength != bufferSize) {
      Serial.print("Error: Content-Length mismatch (expected ");
      Serial.print(bufferSize);
      Serial.print(", got ");
      Serial.print(contentLength);
      Serial.println(")");
      http.end();
      return false;
    }

    // 获取响应流
    WiFiClient *stream = http.getStreamPtr();
    size_t bytesRead = stream->readBytes(buffer, bufferSize);

    Serial.print("Downloaded ");
    Serial.print(bytesRead);
    Serial.println(" bytes");

    http.end();

    if (bytesRead == bufferSize) {
      return true;
    } else {
      Serial.println("Error: Incomplete download");
      return false;
    }
  } else {
    Serial.print("Download bitmap failed! HTTP code: ");
    Serial.println(httpCode);
    if (httpCode > 0) {
      Serial.println(http.getString());
    }
    http.end();
    return false;
  }
}
