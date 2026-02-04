# Infinity Tag Backend - API 接口调用规范

本文档定义了 Infinity Tag 后端服务的 API 交互标准、数据格式及核心接口说明，供前端（墨水屏/小程序/Web）开发参考。

**最后更新**: 2026-02-04
**接口基准地址**: `https://api.infinitytag.app/api/v1` (生产环境) / `http://localhost:8000/api/v1` (开发环境)

---

## 📋 目录

1. [通用交互规范](#1-通用交互规范)
2. [认证接口](#2-认证接口)
3. [用户档案接口](#3-用户档案接口)
4. [智能黄历接口](#4-智能黄历接口)
5. [图片管理接口（新增）](#5-图片管理接口新增)
6. [占卜功能接口](#6-占卜功能接口)
7. [服务端渲染接口](#7-服务端渲染接口)
8. [数据字典与术语](#8-数据字典与术语)

---

## 1. 通用交互规范

### 1.1 响应结构

大部分接口返回标准的 JSON 格式（部分特殊接口如图片预览、bitmap下载返回二进制流）：

**成功响应示例**:
```json
{
  "id": 123,
  "device_id": "ABC123",
  "created_at": "2026-02-04T10:00:00"
}
```

**错误响应示例**:
```json
{
  "detail": "Incorrect device ID or password"
}
```

### 1.2 HTTP 状态码

- **200 OK**: 请求成功
- **201 Created**: 资源创建成功
- **400 Bad Request**: 参数校验失败
- **401 Unauthorized**: 未登录或 Token 过期
- **403 Forbidden**: 无权限访问
- **404 Not Found**: 资源不存在
- **429 Too Many Requests**: 请求过于频繁（触发限流）
- **500 Internal Server Error**: 服务器内部错误

### 1.3 认证方式

所有受保护接口需要在 HTTP Header 中携带 JWT Token：

```http
Authorization: Bearer <your_access_token>
```

---

## 2. 认证接口

### 2.1 设备激活

首次注册设备，返回 Token 和设备密钥。

**接口**: `POST /auth/activate`

**请求参数**:
```json
{
  "device_id": "ABC123",  // 可选，不提供则自动生成6位短码
  "password": "your_password"  // 必填，用户设置的密码（最少6位）
}
```

**响应数据**:
```json
{
  "access_token": "eyJhbGciOiJIUzI1Ni...",
  "token_type": "bearer",
  "device_secret": "a1b2c3d4...",  // HMAC 密钥，仅首次返回，请妥善保存
  "user_id": 1,
  "device_id": "ABC123"
}
```

---

### 2.2 设备登录

已激活设备的登录接口。

**接口**: `POST /auth/login`

**请求参数**:
```json
{
  "device_id": "ABC123",
  "password": "your_password"
}
```

**响应数据**:
```json
{
  "access_token": "eyJhbGciOiJIUzI1Ni...",
  "token_type": "bearer"
}
```

---

### 2.3 获取当前用户信息

**接口**: `GET /auth/me`
**需要认证**: ✅

**响应数据**:
```json
{
  "id": 1,
  "device_id": "ABC123",
  "activated_at": "2026-02-01T08:00:00",
  "last_login_at": "2026-02-04T10:00:00"
}
```

---

## 3. 用户档案接口

### 3.1 创建/更新档案

设置用户的生辰八字信息，用于生成个性化黄历。

**接口**: `POST /profile`
**需要认证**: ✅

**请求参数**:
```json
{
  "nickname": "张三",
  "gender": 1,          // 1:男, 0:女
  "birth_year": 1990,
  "birth_month": 5,
  "birth_day": 20,
  "birth_hour": 14,     // 0-23, 若不知道时辰传 -1
  "is_lunar": false,    // false:公历(阳历), true:农历(阴历)
  "occupation": "程序员", // 可选
  "notes": "备注信息"    // 可选
}
```

**响应数据**:
```json
{
  "id": 1,
  "user_id": 1,
  "nickname": "张三",
  "birth_year": 1990,
  "birth_month": 5,
  "birth_day": 20,
  "created_at": "2026-02-04T10:00:00"
}
```

---

### 3.2 获取档案

**接口**: `GET /profile`
**需要认证**: ✅

**响应**: 同上

---

## 4. 智能黄历接口

### 4.1 生成/获取黄历

生成或获取指定日期的个性化黄历运势（需要先设置档案）。

**接口**: `POST /almanac/generate`
**需要认证**: ✅

**请求参数**:
```json
{
  "target_date": "2026-02-04"  // 可选，默认当天，格式: YYYY-MM-DD
}
```

**响应数据**:
```json
{
  "id": 502,
  "user_id": 1,
  "date": "2026-02-04",
  "lunar_date": "丙午年正月初七",
  "ganzhi_year": "丙午",
  "ganzhi_month": "庚寅",
  "ganzhi_day": "辛巳",

  // AI 生成内容
  "favorable": "签约、搬家、祭祀",
  "unfavorable": "动土、借贷",
  "lucky_direction": "正南方",
  "lucky_item": "琥珀",
  "energy_level": 85,
  "commentary": "今日财运亨通，适合投资理财...",

  "generated_at": "2026-02-04T06:00:00",
  "created_at": "2026-02-04T06:00:00"
}
```

---

### 4.2 获取历史黄历

获取最近 30 天的黄历记录。

**接口**: `GET /almanac/history`
**需要认证**: ✅

**查询参数**:
- `days`: 可选，默认 30，最多 90

**响应**: 数组，元素同上

---

## 5. 图片管理接口（新增）

从 v1.1 开始，支持用户上传自定义图片并进行裁剪、二值化处理，适配 2.13寸 墨水屏（250x122）。

### 5.1 实时预览

上传原图 + 处理参数，返回处理后的预览图（PNG 格式），**不会保存到数据库**。

**接口**: `POST /images/preview`
**需要认证**: ❌（无需认证，方便前端预览）

**请求格式**: `multipart/form-data`

**表单字段**:
- `file`: 图片文件（必填）
- `options`: JSON 字符串（可选），默认 `{}`

**`options` 参数结构**:
```json
{
  "rotate": 90,           // 旋转角度: 0, 90, 180, 270
  "crop_x": 10,           // 裁剪起点 X (相对于原图)
  "crop_y": 20,           // 裁剪起点 Y
  "crop_w": 200,          // 裁剪宽度
  "crop_h": 100,          // 裁剪高度
  "invert": false,        // 是否反色: true/false
  "dither": true,         // 二值化模式: true=抖动(适合照片), false=阈值(适合文字)
  "threshold": 128        // 阈值: 0-255 (仅当 dither=false 时生效)
}
```

**响应**: `image/png` 二进制流（可直接显示在 `<img>` 标签中）

**前端调用示例** (JavaScript):
```javascript
const formData = new FormData();
formData.append('file', fileInput.files[0]);
formData.append('options', JSON.stringify({
  rotate: 90,
  dither: false,
  threshold: 128,
  crop_x: 10,
  crop_y: 20,
  crop_w: 200,
  crop_h: 100
}));

const response = await fetch('/api/v1/images/preview', {
  method: 'POST',
  body: formData
});

const blob = await response.blob();
const imageUrl = URL.createObjectURL(blob);
// 显示在页面上: <img src={imageUrl} />
```

---

### 5.2 上传图片

上传并永久保存图片（每个用户最多 5 张）。

**接口**: `POST /images/`
**需要认证**: ✅

**请求格式**: 同预览接口（`multipart/form-data`）

**表单字段**:
- `file`: 图片文件（必填）
- `options`: JSON 字符串（可选），格式同预览接口

**响应数据**:
```json
{
  "id": 12,
  "url": "/assets/custom_images/1/abc123.png",  // 可通过此 URL 访问图片
  "display_order": 1,
  "view_count": 0,
  "created_at": "2026-02-04T12:00:00"
}
```

---

### 5.3 获取图片列表

**接口**: `GET /images/`
**需要认证**: ✅

**响应**: 数组，元素同上

---

### 5.4 删除图片

**接口**: `DELETE /images/{image_id}`
**需要认证**: ✅

**响应**:
```json
{
  "success": true
}
```

---

### 5.5 重新排序

调整图片显示顺序（用于轮播）。

**接口**: `PUT /images/reorder`
**需要认证**: ✅

**请求参数**:
```json
[
  { "id": 12, "new_order": 1 },
  { "id": 10, "new_order": 2 }
]
```

**响应**:
```json
{
  "success": true
}
```

---

### 5.6 下载设备端位图数据

ESP32 设备专用接口，返回打包后的 1-bit 位图数据（MSB first, Horizontal）。

**接口**: `GET /images/{image_id}/bitmap`
**需要认证**: ✅

**响应**: `application/octet-stream` 二进制流

**数据格式**:
- 分辨率: 250x122
- 格式: 1-bit per pixel, MSB first
- 大小: 约 3812 bytes

---

## 6. 占卜功能接口

### 6.1 获取随机备选字

从字库获取随机字供用户选择。

**接口**: `GET /divination/words`
**需要认证**: ❌

**查询参数**:
| 参数 | 类型 | 必填 | 默认值 | 说明 |
|------|------|------|--------|------|
| `count` | int | 否 | 8 | 获取字数 (1-20) |
| `category` | string | 否 | 无 | 字库类别筛选 |

**可用类别**: `天象`, `地理`, `人事`, `器物`, `德行`, `玄学`, `自然`, `动物`, `时令`, `情志`, `方位`, `数理`

**请求示例**:
```bash
# 获取8个随机字
GET /api/v1/divination/words

# 获取5个天象类别的字
GET /api/v1/divination/words?count=5&category=天象
```

**响应数据**:
```json
["天", "地", "人", "和", "道", "法", "术", "器"]
```

---

### 6.2 获取所有字库类别

**接口**: `GET /divination/categories`
**需要认证**: ❌

**响应数据**:
```json
["天象", "地理", "人事", "器物", "德行", "玄学", "自然", "动物", "时令", "情志", "方位", "数理"]
```

---

### 6.3 解字测算

用户选择两个字进行 AI 解签。

**接口**: `POST /divination/interpret`
**需要认证**: ✅

**请求参数**:
```json
{
  "mode": "本命",        // 必填，测算模式: "本命"(批终身) 或 "客座"(测机锋)
  "intent": "事业",      // 必填，求测意图，最多10字
  "words": ["天", "道"]  // 必填，选中的两个字
}
```

**响应数据**:
```json
{
  "id": 123,
  "mode": "本命",
  "intent": "事业",
  "selected_words": ["天", "道"],
  "result_idiom": "天道酬勤",
  "result_interpretation": "天道二字，上应苍穹，下循法理...(100-200字解签)",
  "result_advice": "宜：勤勉精进，脚踏实地。忌：急于求成，好高骛远。",
  "created_at": "2026-02-04T12:00:00"
}
```

---

### 6.4 灵签速断 (ESP32 端运行)

> ⚠️ **注意**: 此功能纯 ESP32 本地运行，无需服务端接口。

---

### 6.5 乾坤一掷 (ESP32 端运行)

> ⚠️ **注意**: 此功能纯 ESP32 本地运行，无需服务端接口。

---

## 7. 服务端渲染接口

### 7.1 获取黄历渲染图

后端直接生成适配墨水屏的黄历位图。

**接口**: `GET /renderer/preview`
**需要认证**: ✅

**查询参数**:
- `target_date`: 可选，默认当天

**响应**: `image/png` 二进制流（250x122，1-bit）

---

## 8. 数据字典与术语

### 8.1 时辰对照表

API 中 `birth_hour` 参数对应的时辰：

| 数值 | 时辰 | 时间段 |
|------|------|--------|
| 23, 0 | 子时 | 23:00 - 01:00 |
| 1, 2 | 丑时 | 01:00 - 03:00 |
| 3, 4 | 寅时 | 03:00 - 05:00 |
| 5, 6 | 卯时 | 05:00 - 07:00 |
| 7, 8 | 辰时 | 07:00 - 09:00 |
| 9, 10 | 巳时 | 09:00 - 11:00 |
| 11, 12 | 午时 | 11:00 - 13:00 |
| 13, 14 | 未时 | 13:00 - 15:00 |
| 15, 16 | 申时 | 15:00 - 17:00 |
| 17, 18 | 酉时 | 17:00 - 19:00 |
| 19, 20 | 戌时 | 19:00 - 21:00 |
| 21, 22 | 亥时 | 21:00 - 23:00 |
| -1 | 未知 | 不知道时辰 |

### 8.2 图片处理模式说明

**抖动模式 (`dither: true`)**:
- 使用 Floyd-Steinberg 算法
- 适合照片、风景、人像
- 效果：细腻的噪点，保留更多细节

**阈值模式 (`dither: false`)**:
- 使用简单的二值化阈值
- 适合文字、图标、线条
- 效果：锐利的边缘，黑白分明

### 8.3 图片裁剪建议

为了获得最佳显示效果，建议前端：
1. 使用裁剪工具时锁定宽高比为 **250:122** (约 2.05:1)
2. 裁剪后的内容会被强制缩放到 250x122，保持比例可避免拉伸变形

---

## 9. 开发调试

### 9.1 Swagger UI

启动后端服务后，访问 http://localhost:8000/docs 可查看自动生成的交互式 API 文档，支持在线测试。

### 9.2 认证流程

1. 调用 `/auth/activate` 或 `/auth/login` 获取 `access_token`
2. 在所有后续请求中添加 Header: `Authorization: Bearer {token}`
3. Token 有效期为 7 天（168小时）

---

**版本**: v1.2
**更新日期**: 2026-02-04
**变更记录**:
- v1.2: 完善解字系统接口 (字库API、类别筛选)，标注灵签/乾坤一掷为ESP32本地功能
- v1.1: 新增图片管理接口章节，更新认证接口参数
