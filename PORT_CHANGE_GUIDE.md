# 端口修改完成 - 启动指南

## 📝 修改总结

所有服务的端口已从 **8000** 改为 **8001**：

- ✅ **后端服务**：`app/main.py` → 端口 8001
- ✅ **前端代理**：`vite.config.ts` → 代理到 8001
- ✅ **ESP32 固件**：`src/main.cpp` → API_BASE_URL 使用 8001

## 🚀 启动步骤

### 1. 启动后端服务

**方法 A：使用启动脚本（推荐）**
```bash
cd F:\Documents\SumProject\5.1_EPD_TEST\infinity-tag-backend
start.bat
```

**方法 B：手动启动**
```bash
cd F:\Documents\SumProject\5.1_EPD_TEST\infinity-tag-backend
uvicorn app.main:app --reload --host 0.0.0.0 --port 8001
```

**验证后端启动成功**：
- 浏览器访问：http://localhost:8001/docs
- 应该看到 FastAPI 的 Swagger 文档

### 2. 启动前端服务

```bash
cd F:\Documents\SumProject\5.1_EPD_TEST\infinity-tag-frontend
npm run dev
```

**验证前端启动成功**：
- 浏览器访问：http://localhost:5173
- 前端会自动代理 API 请求到后端 8001 端口

### 3. 重新编译并上传 ESP32 固件

ESP32 的 API_BASE_URL 已修改为 `http://192.168.31.57:8001`

**使用 PlatformIO 编译上传**：
```bash
cd F:\Documents\SumProject\5.1_EPD_TEST\infinity-tag-esp32
pio run --target upload
```

或在 VSCode 中点击 PlatformIO 的 "Upload" 按钮。

## ✅ 验证清单

### 后端验证
```bash
# 检查端口监听
netstat -ano | findstr :8001

# 测试健康检查
curl http://localhost:8001/health
```

### 前端验证
1. 打开浏览器：http://localhost:5173
2. 打开开发者工具（F12）
3. 查看 Network 标签
4. API 请求应该成功（状态码 200）

### ESP32 验证
1. 上传新固件后重启 ESP32
2. 查看串口输出
3. 应该看到：
   ```
   Authenticating...
   Authentication successful!
   Fetching image list...
   Downloading bitmap for image XX...
   Downloaded 2808 bytes  ✅
   ```

## 🔧 故障排查

### 问题 1：后端启动失败 - 端口被占用

**症状**：
```
Error: [Errno 10048] error while attempting to bind on address ('0.0.0.0', 8001)
```

**解决方案**：
```bash
# 查找占用 8001 端口的进程
netstat -ano | findstr :8001

# 停止该进程（替换 PID）
taskkill /F /PID <PID>
```

### 问题 2：前端无法连接后端

**症状**：前端显示网络错误或 CORS 错误

**解决方案**：
1. 确认后端服务正在运行：`netstat -ano | findstr :8001`
2. 确认 `vite.config.ts` 中的代理配置正确（已改为 8001）
3. 重启前端开发服务器

### 问题 3：ESP32 连接失败

**症状**：
```
Authentication failed! HTTP code: -1
```

**解决方案**：
1. 确认 ESP32 和电脑在同一局域网
2. 确认后端服务监听在 `0.0.0.0:8001`（不是 `127.0.0.1:8001`）
3. 确认防火墙允许 8001 端口
4. 确认 `API_BASE_URL` 中的 IP 地址正确（192.168.31.57）

## 📊 端口使用情况

| 服务 | 端口 | 说明 |
|------|------|------|
| 后端 API | 8001 | FastAPI 服务 |
| 前端开发服务器 | 5173 | Vite 开发服务器 |
| MySQL | 3306 | 数据库 |
| Redis | 6379 | 缓存（如果使用） |

## 🎯 下一步

1. ✅ 启动后端服务（端口 8001）
2. ✅ 启动前端服务（端口 5173）
3. ✅ 重新编译并上传 ESP32 固件
4. ✅ 通过前端上传新图片（会自动处理为 212x104）
5. ✅ ESP32 下载并显示图片（应该收到 2808 字节，行对齐格式）

---

**注意**：如果之后需要改回 8000 端口，只需要修改以下 3 个文件：
1. `infinity-tag-backend/app/main.py` (第 111 行)
2. `infinity-tag-frontend/vite.config.ts` (第 17, 21 行)
3. `infinity-tag-esp32/src/main.cpp` (第 15 行)
