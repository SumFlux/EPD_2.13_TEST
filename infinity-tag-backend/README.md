# Infinity Tag Backend

**无止便签 Web 后端服务**

基于传统玄学与现代 AI 技术的电子挂件后端系统。

## 技术栈

- **框架**: Python 3.11+ / FastAPI
- **数据库**: MySQL 8.0 + Redis
- **AI 服务**: DMXAPI 中转站
- **部署**: Docker + Nginx

## 快速开始

### 1. 环境准备

```bash
# 安装 Python 依赖
pip install -r requirements.txt

# 复制环境变量模板
cp .env.example .env

# 编辑 .env 文件，配置数据库密码和 JWT 密钥
vim .env
```

### 2. 数据库初始化

```bash
# 创建数据库
mysql -u root -p
CREATE DATABASE infinity_tag CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

# 运行数据库迁移
python scripts/init_db.py
```

### 3. 启动开发服务器

```bash
# 开发模式启动
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000

# 或直接运行
python app/main.py
```

访问 API 文档: http://localhost:8000/docs

### 4. Docker 部署

```bash
# 构建并启动所有服务
docker-compose up -d

# 查看日志
docker-compose logs -f backend
```

## 项目结构

```
infinity-tag-backend/
├── app/
│   ├── main.py              # FastAPI 应用入口
│   ├── config.py            # 配置管理
│   ├── api/v1/              # API 路由层
│   ├── core/                # 核心业务逻辑
│   ├── models/              # 数据库模型
│   ├── schemas/             # 数据验证
│   ├── services/            # 业务服务
│   ├── repositories/        # 数据访问
│   ├── middleware/          # 中间件
│   └── utils/               # 工具函数
├── data/                    # 静态数据
├── migrations/              # 数据库迁移
├── tests/                   # 测试
├── scripts/                 # 部署脚本
├── requirements.txt         # Python 依赖
├── .env.example             # 环境变量模板
└── docker-compose.yml       # Docker 配置
```

## 核心功能

- ✅ **设备激活与影子账户** - 6位设备短码系统
- ✅ **用户档案管理** - 八字计算与存储
- ✅ **黄历系统** - AI 个性化生成 + 30天历史
- ✅ **解字系统** - AI 解签 + Prompt 注入防护
- ✅ **灵签速断** - 传统灵签抽取
- ✅ **乾坤一掷** - 掷币决策

## 文档

- [环境配置详细教程](doc/教程/环境配置详细教程.md)
- [本地调试指南](doc/教程/本地调试指南.md)
- [API 接口调用规范](doc/教程/API接口调用规范.md)
- [单片机通讯与开发指南](doc/教程/单片机通讯协议.md)
- [系统运维与操作指南](doc/教程/运维操作指南.md)

## 安全特性

- 🔒 **HTTPS 强制** - 自动重定向到 HTTPS
- 🔑 **HMAC 签名** - 防伪造和重放攻击
- 🔐 **密码加密** - bcrypt 加密存储
- 🛡️ **Prompt 注入防护** - AI 输入清洗
- ⏱️ **限流保护** - 防止 API 滥用

## 配置说明

所有配置通过环境变量管理，详见 `.env.example`。

关键配置项：
- `JWT_SECRET_KEY` - JWT 密钥（必须修改）
- `MYSQL_PASSWORD` - 数据库密码
- `AI_API_KEY` - AI 服务密钥（可动态修改）
- `AI_MODEL_NAME` - AI 模型名称（可动态修改）

## 开发指南

### 运行测试

```bash
pytest tests/ -v --cov=app
```

### 代码格式化

```bash
black app/
flake8 app/
```

### 数据库迁移

```bash
# 创建迁移
alembic revision --autogenerate -m "description"

# 应用迁移
alembic upgrade head
```

## 许可证

Copyright © 2026 Infinity Tag Team
