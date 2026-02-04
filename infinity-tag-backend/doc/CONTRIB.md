# 开发贡献指南 (Contributing Guide)

欢迎参与 Infinity Tag Backend 的开发！本文档旨在帮助你快速上手开发环境、理解代码结构以及掌握测试流程。

## 🛠 开发环境搭建

### 1. 基础依赖

确保你的系统已安装以下工具：
- **Python 3.9+**
- **MySQL 8.0+**
- **Redis 6.0+**

### 2. 初始化项目

```bash
# 克隆仓库
git clone <repository_url>
cd infinity-tag-backend

# 创建虚拟环境
python -m venv venv
# Windows
venv\Scripts\activate
# Linux/macOS
source venv/bin/activate

# 安装依赖
pip install -r requirements.txt

# 配置环境变量
cp .env.example .env
# 编辑 .env 填入数据库密码和 API Key
```

### 3. 数据库迁移

项目使用 Alembic 进行数据库版本控制。

```bash
# 生成新的迁移脚本 (修改 models 后运行)
alembic revision --autogenerate -m "description of changes"

# 应用迁移 (更新数据库结构)
alembic upgrade head
```

### 4. 启动开发服务器

使用 Uvicorn 启动热重载开发服务器：

```bash
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
```

## 🖼 图片处理模块指南 (New)

本项目包含高级图片处理功能（抖动算法、旋转、反色等），专为 EPD（电子墨水屏）设备优化。

### 核心 API 流程

1. **预览 (/preview)**:
   - 前端上传图片 + JSON 参数。
   - 后端实时处理并返回图片流。
   - **特性**: 不保存到数据库，仅供用户调整参数。

2. **上传 (/images/)**:
   - 上传图片并应用处理参数。
   - 保存到服务器 `assets/` 目录，生成数据库记录。
   - 返回图片 URL 和 ID。

3. **获取位图 (/images/{id}/bitmap)**:
   - 获取供 ESP32 直接使用的原始位图数据（二值化后的 buffer）。

### 常用处理参数 (JSON)

在上传或预览时，通过 `options` 字段传递 JSON 字符串：

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `dither` | bool | false | 是否启用 Floyd-Steinberg 抖动算法 |
| `threshold` | int | 128 | 二值化阈值 (0-255，仅当 dither=false 时生效) |
| `invert` | bool | false | 是否反色 (黑白颠倒) |
| `rotate` | int | 0 | 旋转角度 (支持 0, 90, 180, 270) |
| `crop_x/y/w/h`| int | - | 裁剪区域 (可选) |

### 测试脚本使用

我们提供了完整的 Python 脚本来验证图片处理流程，无需前端即可测试后端逻辑。

#### 1. 基础流程测试
测试基本的登录、上传图片、获取列表和下载位图功能。
```bash
python scripts/test_image_flow.py
```

#### 2. 高级功能测试
测试实时预览接口、旋转、反色、裁剪等高级参数组合。
```bash
python scripts/test_advanced_image_flow.py
```
> **提示**: 运行高级测试后，会在根目录生成 `preview_result.png`，请打开查看处理效果。

## 🧪 测试流程

提交代码前请务必通过所有测试。

### 运行单元测试
```bash
# 运行所有测试
python -m pytest tests/ -v

# 运行带覆盖率的测试
python -m pytest tests/ -v --cov=app

# 仅运行集成测试
python -m pytest tests/ -m integration

# 运行特定测试文件
python -m pytest tests/integration/test_api/test_auth.py -v
```

### 测试目录结构
```
tests/
├── conftest.py              # 共享 fixtures（数据库、客户端、测试数据）
├── unit/                    # 单元测试
├── integration/             # 集成测试
│   └── test_api/
│       └── test_auth.py     # 认证接口测试
└── e2e/                     # 端到端测试
```

### 运行渲染引擎测试
针对 E-ink 渲染功能（文字排版）的视觉测试：
```bash
python scripts/test_renderer.py
```
*检查生成的 `test_render_preview.png` 确认字体和布局正常。*

## 🔮 命理计算模块指南 (New)

本项目包含完整的八字命理计算功能，用于生成个性化黄历和运势。

### 核心组件

| 模块 | 文件 | 功能 |
|------|------|------|
| 十神引擎 | `app/utils/ten_gods.py` | 计算日主与天干的十神关系 |
| 运势服务 | `app/services/fortune_service.py` | 基于十神生成个性化宜忌 |
| 字库服务 | `app/services/word_bank_service.py` | 解字测算的字库管理 |
| AI 服务 | `app/core/ai_service.py` | 天机批注和解签生成 |

### 十神计算示例

```python
from app.utils.ten_gods import calculate_ten_god, get_ten_god_nature

# 计算日主甲木遇丙火的十神关系
ten_god = calculate_ten_god("甲", "丙")  # 返回 "食神"

# 获取十神性质
nature = get_ten_god_nature("食神")
# {'element': '我生', 'nature': '泄秀', 'favorable_energy': '吉', ...}
```

### 运势生成 API 流程

1. **计算十神**: 根据用户日柱与当日天干计算十神关系
2. **规则匹配**: 从 `fortune_rules.json` 获取十神对应的宜忌活动
3. **职业加成**: 根据用户职业叠加专属建议
4. **关注排序**: 按用户关注领域优先级排序
5. **AI 增强** (可选): 调用 AI 生成个性化批注

### 数据文件说明

| 文件 | 用途 | 格式 |
|------|------|------|
| `app/data/fortune_rules.json` | 十神宜忌规则、职业映射、吉祥物等 | JSON |
| `app/data/word_bank.json` | 解字字库，按类别分组 (天象/地理/人事等) | JSON |

### 扩展字库

编辑 `app/data/word_bank.json` 添加新字：

```json
{
  "categories": {
    "新类别": ["字1", "字2", "字3"]
  },
  "all_words": ["...", "字1", "字2", "字3"]
}
```

## 🎨 代码风格

- **格式化**: `black app/ scripts/ tests/`
- **Linting**: `flake8 app/`
- **类型检查**: `mypy app/`

## 📂 目录结构说明

```
app/
├── api/
│   └── v1/endpoints/
│       ├── almanac.py        # 黄历生成接口
│       ├── divination.py     # 解字测算接口 (字库 API)
│       ├── images.py         # [核心] 图片上传与处理接口
│       └── renderer.py       # 文字渲染相关接口
├── core/
│   ├── ai_service.py         # AI 服务封装 (运势生成/解签)
│   └── renderer_engine.py    # 墨水屏渲染核心引擎
├── data/
│   ├── fortune_rules.json    # [新增] 十神宜忌规则库
│   └── word_bank.json        # [新增] 解字字库数据
├── services/
│   ├── almanac_service.py    # 黄历业务逻辑
│   ├── fortune_service.py    # [新增] 运势生成服务
│   ├── word_bank_service.py  # [新增] 字库加载服务
│   ├── image_service.py      # 图片数据库业务逻辑
│   └── image_processing.py   # [核心] Pillow 图片处理算法 (抖动/裁剪等)
├── utils/
│   ├── lunar.py              # 农历/八字计算工具
│   └── ten_gods.py           # [新增] 十神关系计算引擎
assets/
└── fonts/                    # 字体文件存放目录
```

## 🚀 提交规范

遵循 Conventional Commits 规范 (`feat`, `fix`, `docs`, `style`, `refactor`, `test`).
