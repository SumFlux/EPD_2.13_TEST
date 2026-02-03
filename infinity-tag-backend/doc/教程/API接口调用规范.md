# Infinity Tag Backend - API 接口调用规范

本文档定义了 Infinity Tag 后端服务的 API 交互标准、数据格式及核心接口说明，供前端（墨水屏/小程序）开发参考。

**最后更新**: 2026-02-03
**接口基准地址**: `https://api.infinitytag.app/api/v1` (生产环境) / `http://localhost:8000/api/v1` (开发环境)

---

## 1. 通用交互规范

### 1.1 响应结构
所有 API 接口（除特殊说明外）均返回统一的 JSON 格式：

```json
{
  "success": true,          // 请求是否成功
  "message": "ok",          // 提示消息 (用户友好)
  "data": { ... },          // 业务数据 (成功时返回)
  "error_code": 1001        // 错误码 (仅 success=false 时返回)
}
```

### 1.2 HTTP 状态码
- **200 OK**: 请求成功。
- **400 Bad Request**: 参数校验失败（如日期格式错误）。
- **401 Unauthorized**: 未登录或 Token 过期。
- **403 Forbidden**: 无权限访问。
- **429 Too Many Requests**: 请求过于频繁（触发限流）。
- **500 Internal Server Error**: 服务器内部错误。

### 1.3 认证方式
所有受保护接口需要在 HTTP Header 中携带 JWT Token：

```http
Authorization: Bearer <your_access_token>
```

---

## 2. 核心 API 详解

### 2.1 设备认证

#### 2.1.1 设备激活 (登录)
通过设备上的 6 位短码进行认证。首次认证为激活，后续为登录。

- **接口**: `POST /auth/activate`
- **说明**: 设备开机后调用，获取长期有效的 Access Token。

**请求参数**:
```json
{
  "device_code": "A1B2C3",  // 必填，设备背面或屏幕显示的6位码
  "device_uuid": "硬件序列号hash" // 可选，用于绑定硬件
}
```

**响应数据**:
```json
{
  "success": true,
  "data": {
    "access_token": "eyJhbGciOiJIUzI1Ni...",
    "token_type": "bearer",
    "expires_in": 604800  // 秒 (7天)
  }
}
```

---

### 2.2 用户档案 (Profile)

#### 2.2.1 获取/创建/更新档案
设置用户的生辰八字信息，这是生成个性化黄历的基础。

- **接口**: `POST /profile` (创建/更新)
- **接口**: `GET /profile` (获取)

**请求参数 (POST)**:
```json
{
  "nickname": "张三",
  "gender": 1,          // 1:男, 0:女
  "birth_year": 1990,
  "birth_month": 5,
  "birth_day": 20,
  "birth_hour": 14,     // 0-23, 若不知道时辰传 -1
  "is_lunar": false,    // false:公历(阳历), true:农历(阴历)
  "occupation": "程序员", // 可选，职业信息 (替换原MBTI)
  "notes": "备注信息"    // 可选
}
```

**响应数据**:
```json
{
  "success": true,
  "data": {
    "user_id": 10086,
    "nickname": "张三",
    "birth_year": 1990,
    ...
  }
}
```

---

### 2.3 智能黄历 (Almanac)

#### 2.3.1 获取今日黄历
生成或获取指定日期的个性化黄历运势。

- **接口**: `POST /almanac/generate`

**请求参数**:
```json
{
  "target_date": "2026-02-03"  // 可选，默认当天
}
```

**响应数据 (Data)**:
```json
{
  "id": 502,
  "date": "2026-02-03",
  "lunar_date_str": "丙午年正月初七",  // 农历日期

  // 核心运势
  "daily_fortune": "今日财运亨通，适合投资理财，但在人际交往中需注意...",
  "lucky_color": "琥珀色",
  "lucky_direction": "正南方",
  "lucky_time": "未时 (13:00-15:00)",

  // 宜忌列表
  "auspicious": ["签约", "搬家", "祭祀"],
  "inauspicious": ["动土", "借贷"],

  // 结构化评分 (0-100)，用于绘制雷达图
  "wealth_score": 88,    // 财运
  "health_score": 75,    // 健康
  "love_score": 60,      // 桃花
  "career_score": 92     // 事业
}
```

---

## 3. 数据字典与术语

### 3.1 时辰对照表
API 中 `birth_hour` 参数对应的时辰：

| 数值 | 时辰 | 时间段 |
|---|---|---|
| 23, 0 | 子时 | 23:00 - 01:00 |
| 1, 2 | 丑时 | 01:00 - 03:00 |
| ... | ... | ... |
| 21, 22 | 亥时 | 21:00 - 23:00 |
| -1 | 未知 | 未知时辰 |

### 3.2 评分说明
黄历中的分值 (`*_score`) 为 AI 综合天干地支刑冲合害关系计算得出：
- **>= 90**: 大吉 (Excellent)
- **80 - 89**: 吉 (Good)
- **60 - 79**: 平 (Average)
- **< 60**: 凶 (Bad)

---

## 4. 开发调试

### Postman 测试
1. 导入项目根目录下的 `openapi.json` (启动后端服务后访问 `/openapi.json` 获取)。
2. 配置环境变量 `baseUrl` 为 `http://localhost:8000/api/v1`。
3. 先调用 `Auth` 接口获取 Token，并在 Collection 设置中添加 Authorization Header。

### 常见错误码
- **1001**: 设备码无效
- **1002**: 档案未填写 (无法生成黄历)
- **2001**: AI 服务暂时不可用
