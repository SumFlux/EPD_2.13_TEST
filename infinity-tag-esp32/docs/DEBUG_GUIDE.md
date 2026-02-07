# ESP32 重启问题调试指南

## 🔍 墨水屏调试功能已启用

现在设备会在屏幕上显示启动进度，帮助您定位重启原因。

### 启动进度显示

设备启动时会显示 5 个步骤的进度条：

```
Starting...
[=========>          ]
Step 3/5
Connect WiFi...
```

**如果设备在某个步骤重启，您就能知道问题出在哪里：**

| 步骤 | 显示内容 | 可能的问题 |
|------|----------|------------|
| 1/5 | Init hardware... | 硬件初始化失败（墨水屏/输入设备） |
| 2/5 | Allocate memory... | PSRAM 分配失败 |
| 3/5 | Connect WiFi... | WiFi 连接超时（看门狗重启） |
| 4/5 | Authenticate... | HTTP 请求超时（看门狗重启） |
| 5/5 | Ready! | 启动成功 |

---

## 🚨 常见重启原因

### 1. WiFi 连接超时（最可能）

**症状**：屏幕停在 "Step 3/5 Connect WiFi..." 然后重启

**原因**：WiFi 连接超过 10 秒触发看门狗重启

**解决方案**：

#### 方案 A：禁用看门狗（临时调试）

在 `src/main.cpp` 的 `setup()` 开头添加：

```cpp
void setup() {
  // 禁用看门狗（仅用于调试）
  disableCore0WDT();
  disableCore1WDT();

  Serial.begin(115200);
  delay(500);
  // ... 其余代码
}
```

#### 方案 B：增加 WiFi 超时时间

修改 `src/main.cpp` 第 199 行：

```cpp
// 从 10 秒改为 30 秒
if (!network.waitForConnection(30000)) {
```

#### 方案 C：检查 WiFi 配置

确认以下配置正确：

```cpp
#define WIFI_SSID "SumHome"          // ✅ 已配置
#define WIFI_PASSWORD "94449999"     // ✅ 已配置
```

**检查清单**：
- [ ] WiFi 名称是否正确（区分大小写）
- [ ] WiFi 密码是否正确
- [ ] 路由器是否开启 2.4GHz 频段（ESP32 不支持 5GHz）
- [ ] 设备是否在 WiFi 信号范围内

---

### 2. HTTP 请求超时

**症状**：屏幕停在 "Step 4/5 Authenticate..." 然后重启

**原因**：后端服务器无响应或网络不通

**解决方案**：

#### 方案 A：检查后端服务器

```bash
# 在电脑上测试后端是否可访问
curl http://192.168.31.57:8000/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"device_id":"0BFB78","password":"123456"}'
```

**预期响应**：
```json
{
  "access_token": "eyJhbGciOiJIUzI1Ni...",
  "token_type": "bearer"
}
```

#### 方案 B：增加 HTTP 超时时间

修改 `src/Network/ImageFetcher.cpp` 第 15 行：

```cpp
// 从 5 秒改为 15 秒
http.setTimeout(15000);
```

#### 方案 C：临时跳过认证

在 `src/main.cpp` 中注释掉认证步骤（仅用于测试）：

```cpp
// Step 4: Authenticate with backend
displayProgress("Authenticate...", 4, 5);
Serial.println("Authenticating...");

// 临时跳过认证
/*
if (!imageFetcher.authenticate(DEVICE_ID, DEVICE_PASSWORD)) {
  displayError("Auth Failed");
  g_systemReady = false;
  Serial.println("--- READY (No Auth) ---");
  return;
}
*/
Serial.println("[OK] Authenticated (SKIPPED)");
```

---

### 3. 内存不足

**症状**：屏幕停在 "Step 2/5 Allocate memory..." 然后重启

**原因**：PSRAM 分配失败

**解决方案**：

检查 `platformio.ini` 是否包含 PSRAM 标志：

```ini
build_flags =
    -DBOARD_HAS_PSRAM          # ✅ 必须有这一行
    -DARDUINO_USB_CDC_ON_BOOT=1
```

---

### 4. 硬件初始化失败

**症状**：屏幕停在 "Step 1/5 Init hardware..." 然后重启

**原因**：墨水屏或输入设备初始化失败

**解决方案**：

检查硬件连接：
- 墨水屏排线是否插好
- SPI 引脚是否正确
- 电源是否稳定

---

## 🛠️ 分阶段调试法

如果上述方法都不行，使用分阶段调试：

### 阶段 1：最小化启动（只初始化硬件）

注释掉网络相关代码，只测试硬件：

```cpp
void setup() {
  disableCore0WDT();
  disableCore1WDT();

  Serial.begin(115200);
  delay(500);

  // 只初始化硬件
  epd.begin();
  input.begin();
  displayProgress("Hardware OK", 1, 1);

  // 注释掉所有网络代码
  /*
  g_bitmapBuffer = ...
  network.begin(...);
  imageFetcher.authenticate(...);
  */

  while(1) { delay(1000); } // 停在这里，不进入 loop
}
```

**如果这样不重启**：说明硬件正常，问题在网络部分

---

### 阶段 2：测试内存分配

```cpp
void setup() {
  disableCore0WDT();
  disableCore1WDT();

  Serial.begin(115200);
  delay(500);

  epd.begin();
  input.begin();
  displayProgress("Test memory...", 1, 1);

  // 测试内存分配
  if (psramFound()) {
    g_bitmapBuffer = (uint8_t*)ps_malloc(BITMAP_SIZE);
  } else {
    g_bitmapBuffer = (uint8_t*)malloc(BITMAP_SIZE);
  }

  if (g_bitmapBuffer == nullptr) {
    displayError("Memory Failed");
  } else {
    displayProgress("Memory OK", 1, 1);
  }

  while(1) { delay(1000); }
}
```

**如果这样不重启**：说明内存正常，问题在 WiFi/HTTP

---

### 阶段 3：测试 WiFi（不认证）

```cpp
void setup() {
  disableCore0WDT();
  disableCore1WDT();

  Serial.begin(115200);
  delay(500);

  epd.begin();
  input.begin();
  displayProgress("Test WiFi...", 1, 1);

  network.begin(WIFI_SSID, WIFI_PASSWORD);

  if (network.waitForConnection(30000)) {
    displayProgress("WiFi OK", 1, 1);
  } else {
    displayError("WiFi Failed");
  }

  while(1) { delay(1000); }
}
```

**如果这样重启**：说明问题在 WiFi 连接

---

## 📊 重启原因速查表

| 屏幕显示 | 重启原因 | 优先解决方案 |
|----------|----------|--------------|
| Step 1/5 | 硬件初始化失败 | 检查硬件连接 |
| Step 2/5 | 内存分配失败 | 检查 PSRAM 配置 |
| Step 3/5 | WiFi 超时 | 禁用看门狗 + 增加超时 |
| Step 4/5 | HTTP 超时 | 检查后端服务器 |
| 黑屏/白屏 | 启动前崩溃 | 检查电源/硬件 |
| 随机重启 | 栈溢出/内存泄漏 | 减少内存占用 |

---

## 🔧 快速修复代码

### 修复 1：禁用看门狗（最快）

在 `src/main.cpp` 开头添加：

```cpp
#include <esp_task_wdt.h>

void setup() {
  // 禁用看门狗
  esp_task_wdt_init(30, false);  // 30秒超时，不自动重启

  Serial.begin(115200);
  // ... 其余代码
}
```

### 修复 2：增加所有超时时间

```cpp
// WiFi 超时：10秒 → 30秒
network.waitForConnection(30000);

// HTTP 超时：5秒 → 15秒
http.setTimeout(15000);
```

### 修复 3：添加喂狗代码

在长时间操作中添加喂狗：

```cpp
bool NetworkManager::waitForConnection(uint32_t timeout_ms) {
    unsigned long startTime = millis();

    while (!isConnected()) {
        if (millis() - startTime > timeout_ms) {
            return false;
        }

        esp_task_wdt_reset();  // 喂狗
        Serial.print(".");
        delay(500);
    }

    return true;
}
```

---

## 📝 调试日志

请记录以下信息：

1. **屏幕最后显示的内容**：
   - [ ] Step 1/5 Init hardware...
   - [ ] Step 2/5 Allocate memory...
   - [ ] Step 3/5 Connect WiFi...
   - [ ] Step 4/5 Authenticate...
   - [ ] Step 5/5 Ready!
   - [ ] 其他：___________

2. **重启间隔时间**：
   - [ ] 立即重启（<1秒）
   - [ ] 几秒后重启（1-5秒）
   - [ ] 长时间后重启（>10秒）

3. **WiFi 环境**：
   - 路由器型号：___________
   - 信号强度：___________
   - 2.4GHz 是否开启：[ ] 是 [ ] 否

4. **后端服务器**：
   - 是否运行：[ ] 是 [ ] 否
   - 能否 ping 通：[ ] 是 [ ] 否
   - 端口是否开放：[ ] 是 [ ] 否

---

## 🎯 推荐调试顺序

1. **先禁用看门狗**（最快）
2. **增加超时时间**（WiFi 30秒，HTTP 15秒）
3. **分阶段测试**（硬件 → 内存 → WiFi → HTTP）
4. **检查后端服务器**（curl 测试）
5. **临时跳过认证**（测试 WiFi 是否正常）

---

## 💡 提示

- 墨水屏刷新较慢，每个步骤会停留 100ms 让您看清
- 如果屏幕一直黑屏，说明硬件初始化就失败了
- 如果屏幕显示进度条但卡住，说明在等待网络响应
- 重启后屏幕会保留最后的画面（墨水屏特性）

---

## 📞 需要帮助？

如果以上方法都不行，请提供：
1. 屏幕最后显示的内容（拍照）
2. 重启间隔时间
3. WiFi 和后端服务器状态

我会帮您进一步分析！
