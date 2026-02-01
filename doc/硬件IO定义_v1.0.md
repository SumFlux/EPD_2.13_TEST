# 硬件 IO 定义 (Hardware IO Definition)

| 文档版本 | 日期 | 修改人 | 关键变更 |
| :--- | :--- | :--- | :--- |
| **v1.0** | **2026-02-01** | **AI Assistant** | **文档创建。基于 V2 原理图及用户反馈整理 IO 映射；确认移除 SD 卡接口，修正电源键、蜂鸣器及震动开关引脚定义。** |

---

## 1. 主控制器 (ESP32-S3) IO 分配

| 引脚 (IO) | 网络标签 (Net Label) | 功能 (Function) | 备注 (Notes) |
| :--- | :--- | :--- | :--- |
| **Power & Control** | | | |
| GND | GND | 地 | |
| 3V3 | 3V3 | 3.3V 电源 | |
| EN | RST | 复位 / 使能 | |
| IO0 | BOOT | 启动模式 | |
| IO21 | CHRG_DETECT | 充电检测 | |
| IO47 | PWR_IO | 电源控制/状态 | |
| IO38 | PWR_KEY | 编码器按键 (Encoder Button) | 复用编码器按键 |
| IO48 | SWKEY | 震动开关 (Vibration Switch) | |
| IO45 | BZL | 蜂鸣器 (Buzzer) | |
| | | | |
| **E-Paper Display (墨水屏)** | | | |
| IO5 | SPI_MOSI | SPI MOSI | 墨水屏数据线 |
| IO6 | SPI_SCLK | SPI Clock | 墨水屏时钟线 |
| IO7 | EPD_CS | EPD Chip Select | 墨水屏片选 |
| IO17 | EPD_DC | EPD Data/Command | 墨水屏数据/命令切换 |
| IO18 | EPD_RES | EPD Reset | 墨水屏复位 |
| IO8 | EPD_BUSY | EPD Busy | 墨水屏忙信号 |
| | | | |
| **Encoder (旋转编码器)** | | | |
| IO40 | ENC_A | Encoder A | 编码器 A 相 |
| IO39 | ENC_B | Encoder B | 编码器 B 相 |
| IO38 | PWR_KEY | Encoder Button | 编码器按键 (同上) |
| | | | |
| **USB & UART** | | | |
| IO19 | USB_D- | USB D- | USB 数据负 |
| IO20 | USB_D+ | USB D+ | USB 数据正 |
| TXD0 | TX | UART TX | 串口发送 |
| RXD0 | RX | UART RX | 串口接收 |
| | | | |
| **Sensors & Others** | | | |
| IO4 | BAT_ADC | Battery ADC | 电池电压采集 |
| IO15 | 32I | 32kHz XTAL In | 外部 32kHz 晶振输入 |
| IO16 | 32O | 32kHz XTAL Out | 外部 32kHz 晶振输出 |

## 2. 变更说明 (Change Notes)

相较于旧版设计，本版本 (v1.0) 主要硬件变动如下：

1.  **接口移除**: 移除了 SD 卡接口 (原 IO1, IO2, IO41, IO42)。
2.  **电源控制**: 电源键 (PWR_KEY) 迁移至 IO38 (复用编码器按键)；电源状态控制 (PWR_IO) 定义为 IO47。
3.  **交互组件**:
    - 蜂鸣器 (BZL) 迁移至 IO45。
    - 新增/确认 震动开关 (SWKEY) 连接于 IO48。
