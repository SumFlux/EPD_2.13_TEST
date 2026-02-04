# 运维操作手册 (Runbook)

本文档用于指导生产环境的部署、配置维护以及常见故障处理。

## 1. 系统架构

- **应用服务**: FastAPI (Python 3.9+)
- **数据库**: MySQL 8.0+
- **缓存/队列**: Redis
- **反向代理**: Nginx (推荐)

## 2. 环境变量配置

在部署根目录创建 `.env` 文件。以下是关键配置项：

### 基础配置
| 变量名 | 说明 | 示例 |
|--------|------|------|
| `APP_NAME` | 应用名称 | Infinity Tag Backend |
| `ENVIRONMENT` | 环境模式 | production / development |
| `DEBUG` | 调试模式 | False |

### 数据库 & Redis
| 变量名 | 说明 | 示例 |
|--------|------|------|
| `MYSQL_HOST` | 数据库主机 | localhost |
| `MYSQL_PORT` | 端口 | 3306 |
| `MYSQL_USER` | 用户名 | infinitytag |
| `MYSQL_PASSWORD`| 密码 | secure_password |
| `MYSQL_DATABASE`| 数据库名 | infinity_tag |
| `REDIS_HOST` | Redis 主机 | localhost |
| `REDIS_PORT` | Redis 端口 | 6379 |

### 安全配置 (CRITICAL)
| 变量名 | 说明 | 示例 |
|--------|------|------|
| `JWT_SECRET_KEY` | JWT 签名密钥 | **务必修改为随机长字符串** |
| `JWT_ALGORITHM` | 签名算法 | HS256 |
| `AI_API_KEY` | AI 服务密钥 | sk-xxxxxx |

## 3. 部署流程

### 方式一：Docker Compose (推荐)

当代码更新或需要重新部署时：

```bash
# 1. 拉取最新代码
git pull

# 2. 停止旧服务
docker-compose down

# 3. 重新构建并启动 (确保包含新依赖)
docker-compose up -d --build

# 4. 查看日志确认启动成功
docker-compose logs -f app
```

### 方式二：手动部署 (Systemd)

适用于裸机部署：

1. **安装依赖**
   ```bash
   source venv/bin/activate
   pip install -r requirements.txt
   ```

2. **运行数据库迁移**
   ```bash
   alembic upgrade head
   ```

3. **配置 Systemd 服务**
   创建 `/etc/systemd/system/infinity-tag.service`:
   ```ini
   [Unit]
   Description=Infinity Tag Backend
   After=network.target

   [Service]
   User=www-data
   WorkingDirectory=/var/www/infinity-tag-backend
   ExecStart=/var/www/infinity-tag-backend/venv/bin/uvicorn app.main:app --host 0.0.0.0 --port 8000 --workers 4
   Restart=always

   [Install]
   WantedBy=multi-user.target
   ```

4. **管理服务**
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable infinity-tag
   sudo systemctl restart infinity-tag
   ```

## 4. 静态资源管理 (字体)

服务端渲染引擎依赖于字体文件，路径为 `assets/fonts/`。

- **必须包含**: `ChangBanDianSong-12.ttf` (默认字体)
- **添加字体**: 将 `.ttf` 文件上传至该目录，重启服务生效。
- **故障排查**: 如果生成的图片全是方框 (□□□)，请检查该目录下是否有中文字体。

## 5. 常见运维操作

### 数据库备份
```bash
mysqldump -u [user] -p[password] infinity_tag > backup_$(date +%F).sql
```

### 查看应用日志
```bash
# Docker 环境
docker logs --tail 100 -f infinity-tag-app

# 过滤渲染相关日志
docker logs infinity-tag-app | grep "renderer"
```

## 6. 故障排查 (Troubleshooting)

### Q1: 图片处理失败 / IOError
**原因**: 服务器缺少必要的系统库 (如 `libgl1`, `zlib`)。
**解决**:
- 如果是 Docker，请检查 Dockerfile 是否包含 `libgl1-mesa-glx` 等依赖。
- 如果是 Ubuntu 手动部署: `sudo apt-get install libgl1`

### Q2: 墨水屏显示模糊
**原因**: 请求了预览图而非位图，或者二值化阈值设置不当。
**解决**:
- 确认设备请求的是 `/images/{id}/bitmap` 接口。
- 检查上传时的 `dither` 参数是否符合预期。

### Q3: 500 Internal Server Error
**解决**:
- 检查数据库连接是否正常。
- 检查 `logs/` 目录下的详细堆栈信息。

### Q4: 黄历生成返回默认批注
**原因**: AI 服务调用失败，使用了降级方案。
**解决**:
- 检查 `.env` 中 `AI_API_KEY` 是否有效。
- 检查 `AI_BASE_URL` 是否可访问。
- 查看日志中的 `Failed to parse AI fortune response` 错误。

### Q5: 十神计算结果异常
**原因**: 用户档案中的八字日柱格式错误。
**解决**:
- 确认 `profile.bazi_day` 格式为 "甲子" (天干+地支)。
- 检查 `app/utils/ten_gods.py` 中的 `TIAN_GAN` 常量。

### Q6: 字库 API 返回空列表
**原因**: `word_bank.json` 文件缺失或格式错误。
**解决**:
- 确认 `app/data/word_bank.json` 存在。
- 检查 JSON 格式是否正确 (可用 `python -m json.tool` 验证)。

## 7. 命理模块监控

### 关键指标

| 指标 | 正常范围 | 告警阈值 |
|------|---------|---------|
| AI 调用成功率 | > 95% | < 90% |
| 黄历生成延迟 | < 2s | > 5s |
| 十神计算延迟 | < 10ms | > 100ms |

### 日志关键词

```bash
# 监控 AI 服务失败
grep "Failed to parse AI" logs/app.log

# 监控十神计算异常
grep "无效天干" logs/app.log

# 监控字库加载
grep "word_bank" logs/app.log
```

### 数据文件检查

```bash
# 验证 fortune_rules.json 格式
python -m json.tool app/data/fortune_rules.json > /dev/null && echo "OK"

# 验证 word_bank.json 格式
python -m json.tool app/data/word_bank.json > /dev/null && echo "OK"

# 统计字库字数
python -c "import json; d=json.load(open('app/data/word_bank.json')); print(f'总字数: {len(d.get(\"all_words\", []))}')"
```
