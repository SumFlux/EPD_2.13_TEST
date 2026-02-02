<div align="center">

# ESP32-S3 IL3897 E-Paper Driver

<!-- Badges -->
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3-blue)](https://www.espressif.com/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-green)](https://www.arduino.cc/)
[![PlatformIO](https://img.shields.io/badge/Build-PlatformIO-orange)](https://platformio.org/)
[![Version](https://img.shields.io/badge/Version-V1.0.0.2-blueviolet)](memory-bank/progress.md)

**高性能、低残影的 2.13 寸墨水屏驱动测试方案**

[项目文档](doc/IL3897墨水屏开发指南.md) | [硬件定义](doc/硬件IO定义_v1.0.md) | [更新日志](memory-bank/progress.md)

</div>

---

## 📖 项目简介 (Introduction)

本项目是一个针对 **ESP32-S3** 和 **2.13寸 IL3897 (B72)** 墨水屏的高性能驱动实现。区别于普通的示例代码，本项目专注于解决墨水屏开发中常见的痛点：**刷新慢**和**残影重**。

通过深度优化的 **Turbo Partial Mode（极速局刷模式）** 和 **智能残影消除策略**，实现了流畅的编码器交互体验和长期运行的显示质量保障。

## ✨ 核心特性 (Features)

- 🚀 **极速响应**: 优化的 SPI 传输与 `setPartialWindow` 策略，实现毫秒级响应。
- 👁️ **视觉优化**: 
    - **混合刷新引擎**: 结合全刷的清晰度与局刷的速度。
    - **智能抗残影**: 周期性轻度（闪白）与深度（反色）恢复策略，彻底告别重影。
- 🎛️ **交互集成**: 
    - 完整的旋转编码器支持（EC11）。
    - 硬件去抖动与状态机逻辑，操作丝般顺滑。
- 📊 **工程化架构**: 
    - 模块化代码结构，易于移植。
    - 完备的调试接口与状态指示。

## 🛠️ 技术栈 (Tech Stack)

*   **MCU**: Espressif ESP32-S3
*   **Display**: 2.13" E-Paper (IL3897/B72)
*   **Build System**: PlatformIO Core
*   **Framework**: Arduino
*   **Libraries**:
    *   `GxEPD2` (Display Driver)
    *   `Adafruit GFX` (Graphics)

## 🔌 硬件连接 (Hardware Interface)

| 模块 | 引脚 | ESP32-S3 GPIO | 备注 |
| :--- | :--- | :--- | :--- |
| **E-Paper** | MOSI | **GPIO 5** | SPI Data |
| | SCLK | **GPIO 6** | SPI Clock |
| | CS | **GPIO 7** | Chip Select |
| | DC | **GPIO 17** | Data/Command |
| | RST | **GPIO 18** | Reset |
| | BUSY | **GPIO 8** | Busy Signal |
| **Encoder** | A | **GPIO 40** | Clock |
| | B | **GPIO 39** | DT |
| | BTN | **GPIO 38** | Switch |

## 🚀 快速开始 (Quick Start)

### 前置条件
*   安装 VS Code
*   安装 PlatformIO 插件

### 构建步骤
1.  **克隆代码**:
    ```bash
    git clone [repository_url]
    ```
2.  **加载工程**: 使用 PlatformIO 打开 `EPD_TEST` 文件夹。
3.  **连接硬件**: 按照上述表格连接 ESP32-S3 与墨水屏。
4.  **编译烧录**: 点击底部状态栏的 `→` (Upload) 按钮。

## � 版本历史 (Changelog)

*   **V1.0.0.2** (2026-02-02)
    *   ✨ 新增右下角版本号显示
    *   🐛 修复局刷逻辑，简化刷新管道
    *   ⚡ 优化全刷策略，移除冗余闪烁
*   **V1.0.0.1**
    *   🧪 尝试自定义 LUT 实现（实验性）
*   **V1.0.0.0**
    *   🎉 初始版本，完成编码器与基础驱动集成

> 完整更新记录请参阅 [Progress Log](memory-bank/progress.md).

## 🤝 贡献 (Contributing)

欢迎提交 Pull Requests 或 Issues！对于重大变更，请先开 Issue 讨论您想要改变的内容。

## � 许可证 (License)

本项目采用 [MIT 许可证](LICENSE)。

---
<div align="center">
    Made with ❤️ by Agentic AI
</div>
