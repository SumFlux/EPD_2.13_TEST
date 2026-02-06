# 项目架构文档

> 最后更新: 2026-02-06
# ... (omitting header changes to save space)

### 3. 震动交互 (Shake Action)

| 特性 | 参数 | 逻辑描述 |
|------|------|----------|
| **高频捕获** | ISR debounce = 80ms | 忽略微小弹跳，只响应有效碰撞 |
| **摇晃检测** | Window = 1000ms | 在1秒时间窗口内计数脉冲 |
| **触发阈值** | Count >= 6 | 需检测到6次脉冲才视为"摇晃" |
| **防连击** | Cooldown = 500ms | 触发后暂定响应，防止数字跳变 |

### 4. 刷新策略

```
普通局刷 → 每3次闪白 → 每20次深度恢复
```

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
    - 脚本安全: 运维脚本 (`upload_firmware.py`) 采用本地 `.env` 配置签发 Token，避免明文密码传输

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
