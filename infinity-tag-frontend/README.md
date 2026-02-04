# Infinity Tag Frontend

**无止便签 Web 前端应用**

基于 React 19 + TypeScript + Vite 的现代化 SPA 应用，为电子墨水屏挂件提供配置与管理界面。

## 技术栈

- **框架**: React 19 + TypeScript 5.9
- **构建工具**: Vite 7
- **状态管理**: Zustand 5
- **数据请求**: TanStack Query 5 + Axios
- **样式**: Tailwind CSS 3
- **路由**: React Router 7

## 快速开始

### 1. 环境准备

```bash
# 安装依赖
npm install

# 复制环境变量模板
cp .env.example .env

# 编辑 .env 文件，配置 API 地址
```

### 2. 启动开发服务器

```bash
npm run dev
```

访问 http://localhost:5173

### 3. 生产构建

```bash
npm run build

# 预览生产构建
npm run preview
```

## 项目结构

```
infinity-tag-frontend/
├── src/
│   ├── api/                  # API 请求封装
│   │   ├── client.ts         # Axios 实例配置
│   │   ├── auth.ts           # 认证相关 API
│   │   ├── profile.ts        # 用户档案 API
│   │   ├── almanac.ts        # 黄历 API
│   │   ├── images.ts         # 图片管理 API
│   │   ├── divination.ts     # 解字测算 API
│   │   └── admin.ts          # 管理后台 API
│   ├── components/           # 通用组件
│   │   └── common/           # 布局、导航等
│   ├── constants/            # 常量定义
│   │   └── routes.ts         # 路由路径常量
│   ├── features/             # 功能模块 (按业务划分)
│   │   ├── auth/             # 认证相关页面
│   │   ├── profile/          # 用户档案页面
│   │   ├── almanac/          # 黄历页面
│   │   ├── images/           # 图片管理页面
│   │   └── admin/            # 管理后台页面
│   ├── stores/               # Zustand 状态管理
│   │   ├── authStore.ts      # 认证状态
│   │   ├── profileStore.ts   # 用户档案状态
│   │   ├── almanacStore.ts   # 黄历状态
│   │   ├── imagesStore.ts    # 图片状态
│   │   └── adminStore.ts     # 管理后台状态
│   ├── styles/               # 全局样式
│   ├── types/                # TypeScript 类型定义
│   ├── utils/                # 工具函数
│   │   └── logger.ts         # 日志工具 (生产环境禁用)
│   ├── App.tsx               # 根组件 (路由配置)
│   └── main.tsx              # 应用入口
├── public/                   # 静态资源
│   └── fonts/                # 字体文件
├── docs/                     # 项目文档
├── .env.example              # 环境变量模板
├── vite.config.ts            # Vite 配置
├── tailwind.config.js        # Tailwind 配置
└── package.json              # 项目依赖
```

## 核心功能

### 用户端
- ✅ **设备激活** - 扫码激活 + 手动输入
- ✅ **用户档案** - 八字信息录入与管理
- ✅ **黄历查看** - 今日运势 + 历史记录
- ✅ **图片管理** - 上传、预览、处理图片

### 管理后台 (/admin)
- ✅ **仪表盘** - 系统概览统计
- ✅ **设备管理** - 录入、禁用、重置设备
- ✅ **用户管理** - 查看、禁用用户

## 可用脚本

| 命令 | 说明 |
|------|------|
| `npm run dev` | 启动开发服务器 (HMR) |
| `npm run build` | TypeScript 编译 + 生产构建 |
| `npm run preview` | 预览生产构建 |
| `npm run lint` | ESLint 代码检查 |

## 环境变量

参见 [.env.example](.env.example) 获取完整配置项。

| 变量 | 说明 | 默认值 |
|------|------|--------|
| `VITE_API_BASE_URL` | 后端 API 地址 | `/api/v1` |
| `VITE_FONT_CN_PATH` | 中文字体路径 | `/fonts/` |
| `VITE_FONT_CN_FILE` | 中文字体文件名 | `NotoSerifSC-Regular.otf` |

## 文档

- [开发贡献指南](docs/CONTRIB.md)
- [运维部署手册](docs/RUNBOOK.md)

## 浏览器支持

- Chrome 90+
- Firefox 90+
- Safari 14+
- Edge 90+

## 许可证

Copyright © 2026 Infinity Tag Team
