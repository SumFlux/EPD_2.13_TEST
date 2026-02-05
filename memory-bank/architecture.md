# 项目架构文档

> 最后更新: 2026-02-03
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

### 5. OTA 与 安全架构

- **版本控制**: 严格递增的 4 位版本号 (A.B.C.D)，防止降级攻击
- **安全校验**: 
    - 固件上传验证: Admin Token + Checksum 计算
    - 固件下载验证: User/Device Token
    - API 请求验证: 关键业务 (如解字) 强制通过 `X-Signature`, `X-Timestamp` 进行 HMAC-SHA256 签名校验

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

1. **避免全刷闪烁**：使用 `setPartialWindow(0,0,250,122)` 替代 `setFullWindow()`
2. **编码器防抖**：累积4步才触发1次变化
3. **摇晃算法**：基于时间窗口的脉冲计数 (Shake Detection)，有效区分误触和有意摇晃
