#include "Core/StatusBar.h"

// WiFi断联图标（16x16）
// 简单的WiFi符号加上一个X
const uint8_t StatusBar::WIFI_DISCONNECTED_ICON[32] = {
    0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0xFF, 0x00,
    0xC3, 0x81, 0x3C, 0x42, 0x18, 0x24, 0x00, 0x18,
    0x00, 0x18, 0x18, 0x24, 0x3C, 0x42, 0xC3, 0x81,
    0xFF, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00
};

// 电池图标 0格（空）
const uint8_t StatusBar::BATTERY_ICON_0[32] = {
    0x00, 0x00, 0x00, 0x00, 0xFC, 0x3F, 0xFC, 0x3F,
    0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30,
    0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30,
    0xFC, 0x3F, 0xFC, 0x3F, 0x00, 0x00, 0x00, 0x00
};

// 电池图标 1格
const uint8_t StatusBar::BATTERY_ICON_1[32] = {
    0x00, 0x00, 0x00, 0x00, 0xFC, 0x3F, 0xFC, 0x3F,
    0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30,
    0x0C, 0x30, 0x0C, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xFC, 0x3F, 0xFC, 0x3F, 0x00, 0x00, 0x00, 0x00
};

// 电池图标 2格
const uint8_t StatusBar::BATTERY_ICON_2[32] = {
    0x00, 0x00, 0x00, 0x00, 0xFC, 0x3F, 0xFC, 0x3F,
    0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30,
    0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xFC, 0x3F, 0xFC, 0x3F, 0x00, 0x00, 0x00, 0x00
};

// 电池图标 3格
const uint8_t StatusBar::BATTERY_ICON_3[32] = {
    0x00, 0x00, 0x00, 0x00, 0xFC, 0x3F, 0xFC, 0x3F,
    0x0C, 0x30, 0x0C, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xFC, 0x3F, 0xFC, 0x3F, 0x00, 0x00, 0x00, 0x00
};

// 电池图标 4格
const uint8_t StatusBar::BATTERY_ICON_4[32] = {
    0x00, 0x00, 0x00, 0x00, 0xFC, 0x3F, 0xFC, 0x3F,
    0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xFC, 0x3F, 0xFC, 0x3F, 0x00, 0x00, 0x00, 0x00
};

// 电池图标 5格（满）
const uint8_t StatusBar::BATTERY_ICON_5[32] = {
    0x00, 0x00, 0x00, 0x00, 0xFC, 0x3F, 0xFC, 0x3F,
    0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30, 0xFC, 0x30,
    0xFC, 0x3F, 0xFC, 0x3F, 0x00, 0x00, 0x00, 0x00
};

StatusBar::StatusBar() {
}

bool StatusBar::begin() {
    Serial.println("[StatusBar] Initialized");
    return true;
}

void StatusBar::overlay(uint8_t* framebuffer, int width, int height,
                        bool wifiConnected, int batteryLevel) {
    if (framebuffer == nullptr) {
        Serial.println("[StatusBar] ERROR: framebuffer is null");
        return;
    }

    // 叠加WiFi图标（如果断联）
    if (!wifiConnected) {
        _overlayIcon(framebuffer, width, height,
                     WIFI_DISCONNECTED_ICON,
                     WIFI_ICON_X, WIFI_ICON_Y,
                     ICON_WIDTH, ICON_HEIGHT);
    }

    // 叠加电池图标（5段显示）
    const uint8_t* batteryIcon = nullptr;
    switch (batteryLevel) {
        case 0: batteryIcon = BATTERY_ICON_0; break;
        case 1: batteryIcon = BATTERY_ICON_1; break;
        case 2: batteryIcon = BATTERY_ICON_2; break;
        case 3: batteryIcon = BATTERY_ICON_3; break;
        case 4: batteryIcon = BATTERY_ICON_4; break;
        case 5: batteryIcon = BATTERY_ICON_5; break;
        default: batteryIcon = BATTERY_ICON_5; break;
    }

    _overlayIcon(framebuffer, width, height,
                 batteryIcon,
                 BATTERY_ICON_X, BATTERY_ICON_Y,
                 ICON_WIDTH, ICON_HEIGHT);
}

void StatusBar::_overlayIcon(uint8_t* framebuffer, int width, int height,
                             const uint8_t* icon, int x, int y,
                             int iconWidth, int iconHeight) {
    // 遍历图标的每个像素
    for (int dy = 0; dy < iconHeight; dy++) {
        for (int dx = 0; dx < iconWidth; dx++) {
            // 计算图标数据中的位置
            int iconByteIndex = (dy * iconWidth + dx) / 8;
            int iconBitIndex = (dy * iconWidth + dx) % 8;

            // 读取图标像素（1=黑色，0=透明）
            bool iconPixel = (icon[iconByteIndex] >> iconBitIndex) & 0x01;

            // 如果图标像素是黑色，则叠加到framebuffer
            if (iconPixel) {
                int fbX = x + dx;
                int fbY = y + dy;

                // 边界检查
                if (fbX >= 0 && fbX < width && fbY >= 0 && fbY < height) {
                    _setPixel(framebuffer, width, fbX, fbY, true);
                }
            }
        }
    }
}

void StatusBar::_setPixel(uint8_t* framebuffer, int width, int x, int y, bool color) {
    // 计算framebuffer中的位置
    // 假设framebuffer格式：每行27字节（212像素 / 8 = 26.5，向上取整为27）
    int bytesPerRow = (width + 7) / 8;
    int byteIndex = y * bytesPerRow + (x / 8);
    int bitIndex = x % 8;

    if (color) {
        // 设置为黑色（位设为1）
        framebuffer[byteIndex] |= (1 << bitIndex);
    } else {
        // 设置为白色（位设为0）
        framebuffer[byteIndex] &= ~(1 << bitIndex);
    }
}

bool StatusBar::_getPixel(const uint8_t* framebuffer, int width, int x, int y) {
    int bytesPerRow = (width + 7) / 8;
    int byteIndex = y * bytesPerRow + (x / 8);
    int bitIndex = x % 8;

    return (framebuffer[byteIndex] >> bitIndex) & 0x01;
}
