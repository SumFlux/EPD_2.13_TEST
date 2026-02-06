# Espressif Flash Download Tool 烧录指南

本文档介绍如何使用乐鑫官方的 [Flash Download Tool](https://www.espressif.com/en/support/download/other-tools) 将固件烧录到 ESP32-S3 设备。

## 1. 准备工作

### 1.1 获取固件文件
已将编译好的固件文件整理在 `infinity-tag-esp32/firmware_release/` 目录下：
- **bootloader.bin**: 引导程序
- **partitions.bin**: 分区表
- **firmware.bin**: 应用程序主固件

> [!NOTE]
> **关于 boot_app0.bin**:
> 通常还需要一个 `boot_app0.bin` 文件。如果你在烧录工具的安装目录下能找到它（通常在 `bin` 或 `resource` 文件夹），请使用它。如果没有，可以忽略或从 Arduino IDE/PlatformIO 的包中复制。它通常用于 OTA 分区引导，地址为 `0xe000`。

### 1.2 下载烧录工具
1. 访问 [乐鑫工具下载页面](https://www.espressif.com/en/support/download/other-tools)。
2. 下载 **Flash Download Tools** (最新版)。
3. 解压并运行 `flash_download_tool_xxx.exe`.

## 2. 工具配置

### 2.1 启动设置
打开工具后，会出现选择弹窗：
- **ChipType**: 选择 `ESP32-S3`
- **WorkMode**: 选择 `Develop`
- 点击 `OK` 进入主界面。

### 2.2 烧录配置 (SPIDownload 选项卡)

在主界面的 **SPIDownload** 面板中，按照以下表格填写文件路径和地址。请点击 `...` 按钮选择对应的 `.bin` 文件。

| ID | 文件路径 (File Path) | 地址 (Address) | 勾选 |
|:---|:---|:---|:---|
| 1 | `.../bootloader.bin` | **0x0** | ✅ |
| 2 | `.../partitions.bin` | **0x8000** | ✅ |
| 3 | `.../boot_app0.bin` (如有) | **0xe000** | ✅ |
| 4 | `.../firmware.bin` | **0x10000** | ✅ |

### 2.3 选项设置 (重要)
在面板下方的选项中，确保以下设置正确：

- **SPI SPEED**: `40MHz` 或 `80MHz`
- **SPI MODE**: `DIO` (通常 ESP32-S3 使用 DIO 模式)
- **FLASH SIZE**: 根据你的模组选择，通常是 `16MB` (128Mbit) 或 `8MB` (64Mbit)。
    - *如果不确定，可以先选 8MB 尝试*。
- **COM**: 选择你的设备连接的 COM 口。
- **BAUD**: 默认 `115200`，为了速度可以尝试更高的如 `921600`。

## 3. 烧录步骤

1. **连接设备**: 通过 USB 将 ESP32-S3 连接到电脑。
2. **进入下载模式** (如果无法自动识别):
    - 按住板子上的 **BOOT (0)** 键。
    - 按一下 **RESET (RST)** 键。
    - 松开 **BOOT** 键。
    - 此时工具下方应显示 `SYNC` 或 `DOWNLOAD` 状态。
3. **点击 START**: 点击界面左下角的 **START** 按钮。
4. **等待完成**: 进度条走完后，显示 **FINISH**。
5. **复位运行**: 按一下板子上的 **RESET** 键，程序开始运行。

## 4. 常见问题

- **无法同步 (Sync Failed)**: 
    - 检查 COM 口是否选对。
    - 尝试降低 BAUD 波特率。
    - 确保手动进入了下载模式 (按住 BOOT 复位)。
- **烧录后无法启动**:
    - 检查 SPI MODE 是否选错 (尝试 DIO 或 QIO)。
    - 检查各个 bin 文件的地址是否填写正确 (特别是 bootloader 必须是 0x0)。
