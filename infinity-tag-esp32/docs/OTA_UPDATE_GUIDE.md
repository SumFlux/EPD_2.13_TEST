# OTA固件更新功能说明

## 📦 功能概述

OTAManager 提供完整的固件更新功能，支持：
- ✅ 检查固件更新
- ✅ 下载固件
- ✅ 显示更新进度
- ✅ 自动重启
- ⚠️ 签名验证（待完善）

---

## 🏗️ 架构设计

### 组件关系

```
SettingsCard（设置卡片）
    ↓ 调用
OTAManager（OTA管理器）
    ↓ 使用
ESP32 Update API
```

### 文件结构

```
include/Network/
└── OTAManager.h          # OTA管理器头文件

src/Network/
└── OTAManager.cpp        # OTA管理器实现

src/Cards/
└── SettingsCard.cpp      # 集成OTA功能
```

---

## 🔧 使用方法

### 1. 在设置中触发更新

**用户操作**：
1. 进入设置卡片
2. 滚动到"Check Update"
3. 短按确认

**系统行为**：
```
检查更新 → 显示更新信息 → 用户确认 → 下载固件 → 安装 → 重启
```

### 2. 三击触发更新（可选）

**用户操作**：
- 在任意界面连续点击3次

**系统行为**：
```
检测三击 → 触发OTA检查 → 后续流程同上
```

---

## 📡 后端API接口

### 1. 检查更新

**请求**：
```http
GET /api/v1/firmware/check?device_id={device_id}&current_version={version}
```

**响应**：
```json
{
  "has_update": true,
  "version": "1.0.2",
  "size": 1048576,
  "url": "/api/v1/firmware/download/1.0.2",
  "checksum": "sha256:abcdef...",
  "description": "Bug fixes and improvements"
}
```

### 2. 下载固件

**请求**：
```http
GET /api/v1/firmware/download/{version}
```

**响应**：
- Content-Type: application/octet-stream
- 固件二进制数据

---

## 🎨 用户界面

### 检查更新界面

```
┌─────────────────────────┐
│ Checking...             │
└─────────────────────────┘
```

### 发现更新界面

```
┌─────────────────────────┐
│ Update Found            │
├─────────────────────────┤
│ Version: 1.0.2          │
│ Size: 1024 KB           │
│                         │
│ Press to update         │
│ Long press to cancel    │
└─────────────────────────┘
```

### 更新进度界面

```
┌─────────────────────────┐
│ Updating...             │
├─────────────────────────┤
│ Version: 1.0.2          │
│ Size: 1024 KB           │
│                         │
│ ████████████░░░░░░░░    │
│ 65%                     │
└─────────────────────────┘
```

### 更新完成界面

```
┌─────────────────────────┐
│ Update Success          │
└─────────────────────────┘
```

---

## 🔐 安全机制

### 1. HTTPS传输（生产环境）

```cpp
// 使用HTTPS下载固件
http.begin("https://api.example.com/firmware/download/1.0.2");
```

### 2. 签名验证（待实现）

```cpp
bool OTAManager::_verifyFirmware(const uint8_t* data, size_t size) {
    // TODO: 实现HMAC-SHA256签名验证
    // 1. 从服务器获取签名
    // 2. 计算固件的HMAC-SHA256
    // 3. 比较签名是否匹配
    return true;
}
```

### 3. 回滚机制

ESP32的双分区OTA自动支持回滚：
- 如果新固件启动失败，自动回滚到旧版本
- 通过 `Update.end(true)` 标记更新成功

### 4. 配置保护

- OTA只更新app分区
- NVS配置不受影响
- 用户数据不丢失

---

## 📊 更新流程

### 完整流程图

```
用户触发更新
    ↓
检查更新（GET /api/v1/firmware/check）
    ↓
有更新？
    ├─ 否 → 显示"Up to date"
    └─ 是 → 显示更新信息
              ↓
          用户确认？
              ├─ 否 → 返回菜单
              └─ 是 → 下载固件（GET /api/v1/firmware/download）
                        ↓
                    显示进度条
                        ↓
                    写入OTA分区
                        ↓
                    验证固件（可选）
                        ↓
                    标记更新成功
                        ↓
                    显示"Update Success"
                        ↓
                    重启设备
                        ↓
                    启动新固件
```

### 错误处理

| 错误 | 处理方式 |
|------|----------|
| 网络连接失败 | 显示"Check Failed"，返回菜单 |
| 下载失败 | 显示"Update Failed"，返回菜单 |
| 写入失败 | 调用 `Update.abort()`，返回菜单 |
| 验证失败 | 拒绝安装，返回菜单 |
| 启动失败 | 自动回滚到旧版本 |

---

## 🧪 测试场景

### 场景1：检查更新（无更新）

**操作**：
1. 进入设置
2. 选择"Check Update"
3. 短按确认

**预期**：
```
[OTAManager] Checking for updates...
[OTAManager] Already up to date
```

**屏幕显示**：
- "Checking..." → "Up to date"

---

### 场景2：检查更新（有更新）

**操作**：
1. 进入设置
2. 选择"Check Update"
3. 短按确认

**预期**：
```
[OTAManager] Checking for updates...
[OTAManager] Update available: 1.0.1 -> 1.0.2
```

**屏幕显示**：
- "Checking..." → "Update Found"
- 显示版本号和大小

---

### 场景3：执行更新

**操作**：
1. 发现更新后
2. 短按确认更新

**预期**：
```
[OTAManager] Starting update...
[OTAManager] Downloading: http://...
[OTAManager] Firmware size: 1048576 bytes
[OTAManager] Progress: 10% (102400/1048576)
[OTAManager] Progress: 20% (204800/1048576)
...
[OTAManager] Progress: 100% (1048576/1048576)
[OTAManager] Update completed successfully
[OTAManager] Restarting...
```

**屏幕显示**：
- "Updating..." + 进度条
- "Update Success"
- 设备重启

---

### 场景4：更新失败

**操作**：
1. 网络中断或服务器错误

**预期**：
```
[OTAManager] HTTP error: 404
[OTAManager] Update failed
```

**屏幕显示**：
- "Update Failed"
- 返回设置菜单

---

## 🔧 配置选项

### 1. API地址配置

```cpp
// 在 ConfigManager 中配置
config.setAPIBaseURL("http://192.168.31.57:8001");
```

### 2. 超时配置

```cpp
// 检查更新超时：10秒
http.setTimeout(10000);

// 下载固件超时：60秒
http.setTimeout(60000);
```

### 3. 进度更新频率

```cpp
// 每下载10KB更新一次进度
if (written % 10240 == 0 || written == contentLength) {
    _showProgress(written, contentLength);
}
```

---

## 📝 后端实现建议

### 数据库表设计

```sql
CREATE TABLE firmware_versions (
    id SERIAL PRIMARY KEY,
    version VARCHAR(20) NOT NULL,
    build_number INT NOT NULL,
    file_path VARCHAR(255) NOT NULL,
    file_size INT NOT NULL,
    checksum VARCHAR(64) NOT NULL,
    description TEXT,
    release_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT TRUE
);
```

### API实现示例（Python/FastAPI）

```python
@app.get("/api/v1/firmware/check")
async def check_firmware_update(
    device_id: str,
    current_version: str
):
    # 获取最新固件版本
    latest = db.query(FirmwareVersion)\
        .filter(FirmwareVersion.is_active == True)\
        .order_by(FirmwareVersion.build_number.desc())\
        .first()

    # 比较版本
    has_update = compare_version(current_version, latest.version) < 0

    return {
        "has_update": has_update,
        "version": latest.version,
        "size": latest.file_size,
        "url": f"/api/v1/firmware/download/{latest.version}",
        "checksum": latest.checksum,
        "description": latest.description
    }

@app.get("/api/v1/firmware/download/{version}")
async def download_firmware(version: str):
    firmware = db.query(FirmwareVersion)\
        .filter(FirmwareVersion.version == version)\
        .first()

    if not firmware:
        raise HTTPException(status_code=404)

    return FileResponse(
        firmware.file_path,
        media_type="application/octet-stream",
        filename=f"firmware_{version}.bin"
    )
```

---

## 🚀 未来优化

### 1. 增量更新

- 只下载变化的部分
- 减少下载时间和流量

### 2. 断点续传

- 支持下载中断后继续
- 提高更新成功率

### 3. 批量更新

- 支持多个设备同时更新
- 后台任务队列

### 4. 版本回退

- 支持手动回退到旧版本
- 保留多个历史版本

### 5. 更新通知

- 主动推送更新通知
- 定时检查更新

---

## ⚠️ 注意事项

### 1. 网络要求

- 需要稳定的WiFi连接
- 建议在信号良好的环境下更新
- 更新过程中不要断开WiFi

### 2. 电量要求

- 建议电量 > 30%
- 更新过程中不要断电
- 可以添加电量检查

### 3. 存储空间

- 需要足够的OTA分区空间
- 默认配置：3MB app0 + 3MB app1
- 固件大小不能超过分区大小

### 4. 兼容性

- 确保新固件与硬件兼容
- 测试后再发布
- 提供回滚机制

---

## 📊 代码统计

- **新增文件**：2个
  - `include/Network/OTAManager.h`
  - `src/Network/OTAManager.cpp`
- **修改文件**：3个
  - `src/main.cpp`
  - `include/Cards/SettingsCard.h`
  - `src/Cards/SettingsCard.cpp`
- **代码行数**：约 400 行

---

## 🎉 功能完成

OTA固件更新功能已完整实现：
- ✅ 检查更新
- ✅ 下载固件
- ✅ 显示进度
- ✅ 自动重启
- ✅ 集成到设置卡片

请重新编译上传固件测试！

---

生成时间：2026-02-07
版本：v1.0
