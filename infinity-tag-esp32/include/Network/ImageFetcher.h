#ifndef IMAGE_FETCHER_H
#define IMAGE_FETCHER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <vector>


/**
 * @brief 图片信息结构体
 */
struct ImageInfo {
  int id;
  String url;
  int display_order;
};

/**
 * @brief 图片获取器
 *
 * 负责后端 API 调用：认证、获取图片列表、下载位图数据
 */
class ImageFetcher {
public:
  /**
   * @brief 构造函数
   * @param baseUrl 后端 API 基础 URL（例如：http://192.168.1.100:8000）
   */
  ImageFetcher(const char *baseUrl);

  /**
   * @brief 认证设备
   * @param deviceId 设备 ID
   * @param password 设备密码
   * @return true 认证成功，false 认证失败
   */
  bool authenticate(const char *deviceId, const char *password);

  /**
   * @brief 获取图片列表
   * @param outList 输出参数，存储图片列表
   * @return true 获取成功，false 获取失败
   */
  bool fetchImageList(std::vector<ImageInfo> &outList);

  /**
   * @brief 下载位图数据
   * @param imageId 图片 ID
   * @param buffer 缓冲区指针
   * @param bufferSize 缓冲区大小（应为 2808 字节，行对齐格式）
   * @return true 下载成功，false 下载失败
   */
  bool downloadBitmap(int imageId, uint8_t *buffer, size_t bufferSize);

  /**
   * @brief 获取访问令牌
   * @return 访问令牌字符串
   */
  String getToken() { return _accessToken; }

  /**
   * @brief 检查是否已认证
   * @return true 已认证，false 未认证
   */
  bool isAuthenticated() { return !_accessToken.isEmpty(); }

private:
  String _baseUrl;
  String _accessToken;
};

#endif // IMAGE_FETCHER_H
