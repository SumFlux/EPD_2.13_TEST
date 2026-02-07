# 项目进度记录

> 最后更新: 2026-02-07

## 当前版本

**V1.0.0.1**

## 已完成功能

### 2026-02-04

### 2026-02-07

- [x] **OTA 固件升级调试完成**:
    - **后端鉴权集成**: 恢复 `Depends(get_current_user)`，ESP32 实现设备登录 (`_login`) 获取 Token.
    - **接口参数修正**: 修正登录接口 (`device_id` -> `device_code`) 及 OTA Check 接口 (`version` -> `latest_version`, `url` -> `download_url`).
    - **版本号算法**: 升级版本比较逻辑，支持 Major.Minor.Patch.Build (4位) 比较.
    - **稳定性优化**: 解决下载过程触发 Task WDT 复位问题 (EPD 刷新率降低至 10%，循环中添加 `vTaskDelay(1)`).
    - **验证通过**: 成功从 v1.0.0.6 OTA 升级至 v1.0.0.7.

- [x] **EPD 刷新 API 重构** (2026-02-07)
  - [x] 实现回调式刷新接口 (`refreshPartial`, `refreshFull` 等)
  - [x] 迁移所有业务模块 (`main`, `SettingsCard`, `OTAManager` 等)
  - [x] 实现自动全刷策略 (每 5 次局部刷新触发一次消除残影)
  - [x] **中文字体支持** (2026-02-08)
  - [x] 集成 `U8g2_for_Adafruit_GFX`
  - [x] 实现 `ChineseFont` 工具类 (UTF-8 解码, 混排对齐)
  - [x] 修复位图渲染逻辑 (12x12, 24字节/字, MSB-first)
  - [x] 扩展字库覆盖所有 UI 文本 (250+ 字符)
  - [x] 解决链接错误 (`static inline findChineseBitmap`)
  - [x] 统一坐标系统 (左上角原点)，修复 UI 重叠
- [x] **代码质量优化** (2026-02-08)
  - [x] 修复 `main.cpp` 内存泄漏风险 (使用 `std::unique_ptr`)
  - [x] 修复 `InputManager` 数组越界风险 (三击检测 `_clickCount >= 3`)
  - [x] 优化调试输出 (引入 `Logger.h` 宏控制)
- [x] **位图格式修正**: 
    - 确认并修复位图数据格式为 **2808 字节 (行对齐)**，而非之前的 2756 字节 (紧密打包)
    - 更新后端 `renderer_service.py` 图像转换逻辑
    - 同步更新所有测试脚本 (`test_api_bitmap.py`, `test_existing_image.py` 等)
    - 修正相关文档说明 (`README.md`, `NETWORK_SETUP.md` 等)
- [x] **高优先级缺陷修复**:
    - `image_processing.py`: 添加空数据验证，捕获 PIL 异常，防止服务崩溃
    - `image_processing.py`: 为抖动算法添加 `try-except` 处理（内存不足/计算错误）
    - `main.cpp`: 优化看门狗 (WDT) 策略，使用 `esp_task_wdt_init(30, true)` 并定期喂狗，替代完全禁用
- [x] **代码质量提升**:
    - `image_processing.py`: 添加类型注解，将 Bayer 矩阵魔法数字替换为命名常量
    - `main.cpp`: 添加内存重新分配前的释放检查，防止内存泄漏
- [x] **编译兼容性修复**:
    - `main.cpp`: 修正 WDT 初始化代码以兼容 ESP32 Arduino Core v2.x
    - `ImageFetcher.cpp`: 迁移 ArduinoJson v6 代码 (Static/DynamicJsonDocument) 到 v7 (`JsonDocument`)

### 2026-02-06

- [x] **OTA 固件管理后端**: 完成固件模型、上传/下载 API 开发，支持版本号校验和 HMAC 签名验证
- [x] **安全增强**: 全面实现 Admin 接口鉴权，解字接口增加请求签名校验，优化 `upload_firmware.py` 脚本使用本地 Token 签发代替明文密码
- [x] **前端架构重构**: 
    - 实现 `AdminHeader` 组件，统一 Dashboard/Device/User/Firmware 页面导航
    - 修复 Admin 页面语法错误 (Duplicate Return/Div)
    - 优化 `client.ts` 拦截器，解决 Admin/User Token 冲突和 401 跳转问题
- [x] **前端 OTA 功能**: 完成固件上传页面开发，修复 Content-Type 导致的 422 错误
- [x] **运维工具**: 优化 `upload_firmware.py` 脚本，支持从 `.env` 读取配置，提升安全性
- [x] **全局分辨率适配**: 统一调整分辨率为 212x104 (原 250x122)，涉及 PRD、后端处理、脚本及文档
- [x] **固件版本自动管理**: 
    - 新增 `Version.h` 定义固件版本号 (vMAJOR.MINOR.PATCH.BUILD)
    - 新增 `tools/increment_version.py` 自动递增 BUILD 号
    - 更新 `build_and_release` 工作流，编译前自动执行版本递增
- [x] **EPD 多行显示**: 
    - 重构 `EPD_Driver::drawInfo()` 支持3行文本显示
    - 第1行: 固件版本, 第2行: 编码器计数(0-999), 第3行: 震动计数(0-99)
- [x] **输入逻辑分离**: 编码器和震动开关使用独立计数器，互不影响

### 2026-02-04

- [x] 更新 README 文档以匹配 V1.0.0.6 版本 (新增震动功能说明)

### 2026-02-03

- [x] 新增震动开关功能 (`PIN_SW_KEY` = IO48)
- [x] 实现智能摇晃检测算法 (Shake Detection)
    - 阈值: 1秒内需震动6次 (排除误触)
    - 消抖: 80ms 硬件中断消抖
    - 冷却: 触发后500ms 不响应
- [x] 实现摇晃交互：剧烈摇晃设备数字 +1

### 2026-02-02

- [x] 修复局刷不工作问题，简化代码结构
- [x] 添加右下角版本号显示 (V1.0.0.2)
- [x] 优化全刷逻辑，使用 `setPartialWindow` 全屏刷新避免库的多闪 LUT
- [x] 深度恢复改为单次黑白刷新

## 版本历史

| 版本 | 日期 | 主要变更 |
|------|------|----------|
| V1.0.0.1 | 2026-02-06 | 固件版本自动管理, 多行显示, 输入逻辑重构 |
| V1.0.0.6 | 2026-02-03 | 震动开关功能 (降敏摇晃检测) |
| V1.0.0.2 | 2026-02-02 | 修复局刷、添加版本号显示 |
| V1.0.0.1 | 2026-02-02 | 尝试自定义 LUT（已回滚） |
| V1.0.0.0 | 2026-02-01 | 初始版本，编码器控制 |

## 待办事项

- [ ] 集成到主项目


## 已知问题

1. **全刷闪烁**：初始化时的 `setFullWindow()` 仍会使用库默认 LUT，导致多次闪烁
2. **对比度**：高速连续局刷后对比度会下降，需定期恢复

## 技术洞察

1. GxEPD2 的 `setFullWindow()` 会触发 `_Init_Full()` 加载多闪烁 LUT
2. 使用全屏 `setPartialWindow()` 可绕过此限制
3. 编码器状态机需累积4步才算1格，避免抖动
