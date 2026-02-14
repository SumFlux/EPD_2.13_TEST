#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <Arduino.h>
#include "Driver/Framebuffer.h"

/**
 * @brief 状态栏渲染器
 *
 * 在右上角渲染WiFi断联图标和5段电池图标
 * 使用透明背景叠加技术（只修改图标的非透明像素）
 */
class StatusBar {
public:
    StatusBar();

    /**
     * @brief 初始化状态栏
     * @return true 成功，false 失败
     */
    bool begin();

    /**
     * @brief 叠加状态栏图标到framebuffer（旧接口）
     * @deprecated 请使用 renderToFramebuffer() 代替
     *
     * @param framebuffer 帧缓冲区指针
     * @param width 屏幕宽度（像素）
     * @param height 屏幕高度（像素）
     * @param wifiConnected WiFi是否连接
     * @param batteryLevel 电池电量（0-5，5段显示）
     */
    void overlay(uint8_t* framebuffer, int width, int height,
                 bool wifiConnected, int batteryLevel);

    /**
     * @brief 渲染状态栏到 Framebuffer（新接口）
     *
     * @param fb Framebuffer 对象
     */
    void renderToFramebuffer(Framebuffer& fb);

    /**
     * @brief 设置 WiFi 连接状态
     */
    void setWifiConnected(bool connected) { _wifiConnected = connected; }

    /**
     * @brief 设置电池电量
     * @param level 电池电量（0-5）
     */
    void setBatteryLevel(int level) { _batteryLevel = level; }

private:
    /**
     * @brief 叠加单个图标
     *
     * @param framebuffer 帧缓冲区指针
     * @param width 屏幕宽度（像素）
     * @param height 屏幕高度（像素）
     * @param icon 图标数据（16x16，32字节）
     * @param x 图标X坐标
     * @param y 图标Y坐标
     * @param iconWidth 图标宽度（像素）
     * @param iconHeight 图标高度（像素）
     */
    void _overlayIcon(uint8_t* framebuffer, int width, int height,
                      const uint8_t* icon, int x, int y,
                      int iconWidth, int iconHeight);

    /**
     * @brief 设置framebuffer中的像素
     *
     * @param framebuffer 帧缓冲区指针
     * @param width 屏幕宽度（像素）
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @param color 颜色（true=黑色，false=白色）
     */
    void _setPixel(uint8_t* framebuffer, int width, int x, int y, bool color);

    /**
     * @brief 获取framebuffer中的像素
     *
     * @param framebuffer 帧缓冲区指针
     * @param width 屏幕宽度（像素）
     * @param x 像素X坐标
     * @param y 像素Y坐标
     * @return true=黑色，false=白色
     */
    bool _getPixel(const uint8_t* framebuffer, int width, int x, int y);

    // 状态变量
    bool _wifiConnected = true;
    int _batteryLevel = 5;

    // 图标位置（右上角）
    static const int WIFI_ICON_X = 70;
    static const int WIFI_ICON_Y = 2;
    static const int BATTERY_ICON_X = 85;
    static const int BATTERY_ICON_Y = 2;

    // 图标尺寸
    static const int ICON_WIDTH = 16;
    static const int ICON_HEIGHT = 16;

    // 内置图标数据（16x16，32字节）
    // WiFi断联图标
    static const uint8_t WIFI_DISCONNECTED_ICON[32];

    // 电池图标（0-5格）
    static const uint8_t BATTERY_ICON_0[32];
    static const uint8_t BATTERY_ICON_1[32];
    static const uint8_t BATTERY_ICON_2[32];
    static const uint8_t BATTERY_ICON_3[32];
    static const uint8_t BATTERY_ICON_4[32];
    static const uint8_t BATTERY_ICON_5[32];
};

#endif // STATUS_BAR_H
