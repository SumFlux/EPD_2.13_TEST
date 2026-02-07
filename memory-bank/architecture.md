# 项目架构文档

> 最后更新: 2026-02-07
# ... (omitting header changes to save space)

### 3. 震动交互 (Shake Action)

| 特性 | 参数 | 逻辑描述 |
|------|------|----------|
| **高频捕获** | ISR debounce = 80ms | 忽略微小弹跳，只响应有效碰撞 |
| **摇晃检测** | Window = 1000ms | 在1秒时间窗口内计数脉冲 |
| **触发阈值** | Count >= 6 | 需检测到6次脉冲才视为"摇晃" |
| **防连击** | Cooldown = 500ms | 触发后暂定响应，防止数字跳变 |

### 4. EPD 刷新策略 (Callback API)

| 方法 | 刷新区域 | 闪烁 | 速度 | 用途 |
|------|----------|------|------|------|
| `refreshPartial` | 局部可变 | 无 | 快 | 数字跳变、菜单选择 |
| `refreshFlicker` | 局部全屏 | 1次 | 中 | 每5次局刷后自动消除残影 |
| `refreshFull` | 全屏 | 2次 | 中慢 | 切换卡片、场景切换 |
| `refreshDeep` | 全屏 | 3次 | 慢 | 开机初始化、刷图片、休眠前 |

**核心设计**: 所有模式均使用 `setPartialWindow` 实现，避免触发硬件全刷命令。自动策略: 每5次 `refreshPartial` 自动调用 `refreshFlicker`。

### 5. 版本管理

| 文件 | 作用 |
|------|------|
| `include/Version.h` | 定义 `VERSION_MAJOR/MINOR/PATCH/BUILD` 宏 |
| `tools/increment_version.py` | 自动递增 BUILD 号 (0-99 循环) |
| `build_and_release.md` | 工作流在编译前调用版本递增脚本 |

### 6. 多行显示 (drawInfo)

| 行号 | 内容 | Y坐标 |
|------|------|-------|
| 1 | `Firmware: vX.X.X.X` | 92 |
| 2 | `Encoder: 0-999` | 102 |
| 3 | `Vibration: 0-99` | 112 |

### 5. OTA 与 安全架构

- **版本控制**: 严格递增的 4 位版本号 (A.B.C.D)，防止降级攻击
- **安全校验**: 
    - 固件上传验证: Admin Token + Checksum 计算
    - 固件下载验证: User/Device Token
    - API 请求验证: 关键业务 (如解字) 强制通过 `X-Signature`, `X-Timestamp` 进行 HMAC-SHA256 签名校验
    - API 请求验证: 关键业务 (如解字) 强制通过 `X-Signature`, `X-Timestamp` 进行 HMAC-SHA256 签名校验
    - 脚本安全: 运维脚本 (`upload_firmware.py`) 采用本地 `.env` 配置签发 Token，避免明文密码传输
    - **设备端 OTA 策略**:
        - 登录获取 Token -> Check Update -> Download (带 Token Header)
        - 下载过程中每 10% 刷新一次屏幕，防止看门狗超时 (WDT Reset)
        - 强制 `vTaskDelay(1)` 让出 CPU 时间片

### 6. 前端架构 (Admin)

- **统一导航**: 使用 `AdminHeader` 组件管理所有管理后台页面的导航结构
- **拦截器策略**: `client.ts` 区分 Admin/User路由，分别加载 `infinity-tag-admin` 和 `access_token`，解决多账户 Token 冲突问题
- **状态管理**: 使用 Zustand (`adminStore`) 管理管理员会话和设备列表状态

## 硬件配置

| 接口 | GPIO |
|------|------|
| ... | ... |
| ENC_BTN | 38 |
| **VIB_SW (震动)** | **48** |

## 依赖库

- `GxEPD2@^1.6.6` - 墨水屏驱动库
- `Adafruit GFX Library@^1.12.1` - 图形库

## 技术要点

1. **避免全刷闪烁**：使用 `setPartialWindow(0,0,212,104)` 替代 `setFullWindow()`
2. **编码器防抖**：累积4步才触发1次变化
3. **摇晃算法**：基于时间窗口的脉冲计数 (Shake Detection)，有效区分误触和有意摇晃

### 7. 位图数据格式 (Bitmap Format)

- **分辨率**: 212 x 104 像素
- **颜色深度**: 1-bit (黑白)
- **数据对齐**: **行对齐 (Row-Aligned)**
  - 每行字节数: `(212 + 7) / 8 = 27` 字节
  - 总字节数: `27 * 104 = 2808` 字节
  - **非**紧密打包 (2756 字节)
- **字节序**: MSB First (位7为最左侧像素)
- **兼容性**: 适配 `GxEPD2` 库使用的 `Adafruit_GFX` 绘图标准
