# 运维部署手册 (Runbook)

本文档用于指导 Infinity Tag Frontend 的生产环境部署、配置维护以及常见故障处理。

## 1. 系统架构

```
                    ┌─────────────┐
                    │   Nginx     │
                    │ (反向代理)   │
                    └──────┬──────┘
                           │
           ┌───────────────┼───────────────┐
           │               │               │
           ▼               ▼               ▼
    ┌─────────────┐  ┌─────────────┐  ┌─────────────┐
    │  静态资源   │  │  /api/v1    │  │   /assets   │
    │  (前端SPA)  │  │  (后端API)  │  │  (图片存储) │
    └─────────────┘  └─────────────┘  └─────────────┘
```

- **前端**: 静态 SPA，部署到 CDN 或 Nginx
- **API 代理**: Nginx 反向代理到后端服务
- **资源存储**: 图片等静态资源

## 2. 环境变量配置

在构建时通过 `.env` 文件或 CI/CD 环境变量配置：

| 变量 | 说明 | 生产环境示例 |
|------|------|-------------|
| `VITE_API_BASE_URL` | 后端 API 地址 | `/api/v1` 或 `https://api.example.com/v1` |
| `VITE_FONT_CN_PATH` | 中文字体路径 | `/fonts/` |
| `VITE_FONT_CN_FILE` | 中文字体文件 | `NotoSerifSC-Regular.otf` |
| `VITE_FONT_CN_NAME` | 中文字体名称 | `Noto Serif SC` |
| `VITE_FONT_EN_PATH` | 英文字体路径 | `/fonts/` |
| `VITE_FONT_EN_FILE` | 英文字体文件 | `JetBrainsMono-Regular.ttf` |
| `VITE_FONT_EN_NAME` | 英文字体名称 | `JetBrains Mono` |

> **注意**: `VITE_` 前缀的变量会被打包进前端代码，不要放置敏感信息。

## 3. 构建与部署

### 方式一：静态文件部署 (推荐)

```bash
# 1. 安装依赖
pnpm install --frozen-lockfile

# 2. 构建生产版本
pnpm build

# 3. 构建产物在 dist/ 目录
ls -la dist/

# 4. 部署到 Web 服务器
rsync -avz dist/ user@server:/var/www/infinity-tag-frontend/
```

### 方式二：Docker 部署

```dockerfile
# Dockerfile
FROM node:20-alpine AS builder
WORKDIR /app
COPY package.json pnpm-lock.yaml ./
RUN npm install -g pnpm && pnpm install --frozen-lockfile
COPY . .
RUN pnpm build

FROM nginx:alpine
COPY --from=builder /app/dist /usr/share/nginx/html
COPY nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
```

```bash
# 构建镜像
docker build -t infinity-tag-frontend:latest .

# 运行容器
docker run -d -p 80:80 infinity-tag-frontend:latest
```

### 方式三：Vercel/Netlify 部署

1. 连接 Git 仓库
2. 设置构建命令: `pnpm build`
3. 设置输出目录: `dist`
4. 配置环境变量

## 4. Nginx 配置

### 基础配置

```nginx
server {
    listen 80;
    server_name example.com;
    root /var/www/infinity-tag-frontend;
    index index.html;

    # SPA 路由支持
    location / {
        try_files $uri $uri/ /index.html;
    }

    # API 代理
    location /api/ {
        proxy_pass http://localhost:8000;
        proxy_http_version 1.1;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }

    # 静态资源缓存
    location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg|woff|woff2|ttf|otf)$ {
        expires 1y;
        add_header Cache-Control "public, immutable";
    }

    # 禁止访问隐藏文件
    location ~ /\. {
        deny all;
    }
}
```

### HTTPS 配置 (推荐)

```nginx
server {
    listen 443 ssl http2;
    server_name example.com;

    ssl_certificate /etc/ssl/certs/example.com.pem;
    ssl_certificate_key /etc/ssl/private/example.com.key;
    ssl_protocols TLSv1.2 TLSv1.3;

    # ... 其余配置同上
}

# HTTP 重定向到 HTTPS
server {
    listen 80;
    server_name example.com;
    return 301 https://$server_name$request_uri;
}
```

## 5. 常见运维操作

### 查看构建信息

```bash
# 查看构建版本 (如果配置了版本号)
cat dist/version.json

# 查看构建产物大小
du -sh dist/*
```

### 清理缓存

```bash
# 清理 node_modules
rm -rf node_modules
pnpm install

# 清理构建缓存
rm -rf dist node_modules/.vite
```

### 回滚部署

```bash
# 保留上一个版本
mv /var/www/infinity-tag-frontend /var/www/infinity-tag-frontend.bak

# 恢复上一个版本
mv /var/www/infinity-tag-frontend.bak /var/www/infinity-tag-frontend
```

## 6. 故障排查 (Troubleshooting)

### Q1: 页面白屏

**可能原因**:
1. JavaScript 加载失败
2. 路由配置错误
3. API 请求失败导致组件崩溃

**解决方案**:
```bash
# 检查浏览器控制台错误
# 检查网络请求是否成功
# 检查 Nginx 配置的 try_files
```

### Q2: API 请求 404

**原因**: Nginx 代理配置问题

**解决**:
```bash
# 检查 Nginx 配置
nginx -t

# 检查后端服务是否运行
curl http://localhost:8000/api/v1/health

# 重载 Nginx
nginx -s reload
```

### Q3: 刷新页面 404

**原因**: SPA 路由未配置 fallback

**解决**: 确保 Nginx 配置了 `try_files $uri $uri/ /index.html;`

### Q4: 资源加载慢

**解决方案**:
1. 启用 Gzip 压缩
2. 配置 CDN
3. 检查资源缓存配置

```nginx
# 启用 Gzip
gzip on;
gzip_types text/plain text/css application/json application/javascript text/xml application/xml;
gzip_min_length 1000;
```

### Q5: 登录后跳转异常

**原因**: Token 存储或路由守卫问题

**解决**:
```javascript
// 清理浏览器 localStorage
localStorage.clear()

// 刷新页面重试
```

### Q6: 字体显示异常

**原因**: 字体文件未正确加载

**解决**:
```bash
# 检查字体文件是否存在
ls -la dist/fonts/

# 检查 MIME 类型配置
# Nginx 需要正确配置 .woff2, .ttf 等类型
```

## 7. 监控与告警

### 关键指标

| 指标 | 正常范围 | 告警阈值 |
|------|---------|---------|
| 首屏加载时间 (FCP) | < 1.5s | > 3s |
| 资源加载错误率 | 0% | > 1% |
| API 请求成功率 | > 99% | < 95% |

### 推荐监控工具

- **性能监控**: Google Lighthouse, Web Vitals
- **错误监控**: Sentry
- **用户行为**: Google Analytics, 百度统计

## 8. 安全加固

### CSP 配置

```nginx
add_header Content-Security-Policy "default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; font-src 'self'; connect-src 'self' https://api.example.com;" always;
```

### 其他安全头

```nginx
add_header X-Frame-Options "SAMEORIGIN" always;
add_header X-Content-Type-Options "nosniff" always;
add_header X-XSS-Protection "1; mode=block" always;
add_header Referrer-Policy "strict-origin-when-cross-origin" always;
```

## 9. 版本历史

| 版本 | 日期 | 说明 |
|------|------|------|
| 0.1.0 | 2026-02 | 初始版本 |
