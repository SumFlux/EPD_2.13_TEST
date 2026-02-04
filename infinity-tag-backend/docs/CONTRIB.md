# 开发贡献指南 (Contributing Guide)

欢迎参与 Infinity Tag Backend 的开发！本文档旨在帮助你快速上手开发环境、理解代码结构以及掌握测试流程。

## 🛠 开发环境搭建

### 1. 基础依赖

确保你的系统已安装以下工具：
- **Python 3.11+**
- **MySQL 8.0+**
- **Redis 5.0+**

### 2. 初始化项目

```bash
# 克隆仓库（如果你还没做）
git clone <repository_url>
cd infinity-tag-backend

# 创建虚拟环境
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate

# 安装依赖
pip install -r requirements.txt

# 配置环境变量
cp .env.example .env
# 编辑 .env 填入数据库密码和 API Key
```

## 🧪 测试流程

我们在项目中实施了严格的测试标准，提交代码前请务必通过所有测试。

### 运行单元测试

使用 `pytest` 运行所有测试用例：

```bash
pytest tests/ -v
```

### 运行渲染引擎测试 (New)

针对新增的 E-ink 渲染功能，我们提供了专门的视觉测试脚本。该脚本会生成预览图片，用于检查布局和字体渲染效果。

```bash
# 运行渲染器测试脚本
python scripts/test_renderer.py
```

**检查结果：**
运行后，请检查生成的 `test_render_preview.png` 图片，确认：
1. 文字是否清晰可见（无乱码）。
2. 布局是否符合 250x122 分辨率限制。
3. 二值化（Dithering）效果是否正常。

## 🎨 代码风格

本项目遵循严格的代码规范：

- **格式化**: 使用 `black`
- **Linting**: 使用 `flake8`
- **类型检查**: 使用 `mypy`

```bash
# 自动格式化
black app/ scripts/ tests/

# 代码检查
flake8 app/
```

## 📂 目录结构说明

```
app/
├── api/
│   └── v1/endpoints/renderer.py  # [新增] 渲染相关接口
├── core/
│   ├── renderer_engine.py        # [新增] 墨水屏渲染核心引擎
│   └── renderer/                 # 渲染器辅助类
├── services/
│   └── renderer_service.py       # 渲染业务逻辑
assets/
└── fonts/                        # [新增] 字体文件存放目录
```

## 🚀 提交规范

请遵循 [Conventional Commits](https://www.conventionalcommits.org/) 规范：

- `feat`: 新功能
- `fix`: 修复 Bug
- `docs`: 文档变更
- `style`: 代码格式（不影响逻辑）
- `refactor`: 重构
- `test`: 测试相关

---
**Happy Coding!** 🚀
