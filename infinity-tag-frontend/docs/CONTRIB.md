# 开发贡献指南 (Contributing Guide)

欢迎参与 Infinity Tag Frontend 的开发！本文档旨在帮助你快速上手开发环境、理解代码结构以及掌握开发规范。

## 🛠 开发环境搭建

### 1. 基础依赖

确保你的系统已安装以下工具：
- **Node.js 18+** (推荐 20 LTS)
- **pnpm 8+** (推荐) 或 npm 9+

### 2. 初始化项目

```bash
# 克隆仓库
git clone <repository_url>
cd infinity-tag-frontend

# 安装依赖 (推荐使用 pnpm)
pnpm install

# 配置环境变量
cp .env.example .env
# 编辑 .env 填入后端 API 地址
```

### 3. 启动开发服务器

```bash
pnpm dev
```

访问 http://localhost:5173，支持热模块替换 (HMR)。

## 📁 目录结构说明

```
src/
├── api/                      # API 请求层
│   ├── client.ts             # Axios 实例、拦截器配置
│   ├── auth.ts               # 认证 API (登录、激活)
│   ├── profile.ts            # 用户档案 API
│   ├── almanac.ts            # 黄历 API
│   ├── images.ts             # 图片管理 API
│   ├── divination.ts         # 解字测算 API
│   └── admin.ts              # 管理后台 API
├── components/               # 通用组件
│   └── common/
│       └── Layout.tsx        # 页面布局组件
├── constants/                # 常量定义
│   └── routes.ts             # 路由路径常量
├── features/                 # 功能模块 (按业务划分)
│   ├── auth/                 # 认证模块
│   │   ├── SetupPage.tsx     # 激活/登录页
│   │   └── SetPasswordPage.tsx
│   ├── profile/              # 用户档案模块
│   ├── almanac/              # 黄历模块
│   ├── images/               # 图片管理模块
│   └── admin/                # 管理后台模块
│       ├── LoginPage.tsx
│       ├── DashboardPage.tsx
│       ├── DevicesPage.tsx
│       └── UsersPage.tsx
├── stores/                   # Zustand 状态管理
│   ├── authStore.ts          # 认证状态 (token, deviceId)
│   ├── profileStore.ts       # 用户档案状态
│   ├── almanacStore.ts       # 黄历数据状态
│   ├── imagesStore.ts        # 图片列表状态
│   └── adminStore.ts         # 管理后台状态
├── types/                    # TypeScript 类型定义
│   ├── index.ts              # 通用类型
│   ├── api.ts                # API 响应类型
│   └── admin.ts              # 管理后台类型
├── utils/                    # 工具函数
│   └── logger.ts             # 日志工具 (生产环境禁用 console)
├── styles/                   # 全局样式
│   └── index.css             # Tailwind 入口
├── App.tsx                   # 根组件 (路由配置)
└── main.tsx                  # 应用入口
```

## 🎨 代码风格

### TypeScript 规范

- 使用严格模式 (`strict: true`)
- 所有组件使用函数式组件 + Hooks
- 优先使用 `type` 而非 `interface`
- API 响应必须定义类型

### 命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| 组件文件 | PascalCase | `DevicesPage.tsx` |
| 工具函数 | camelCase | `formatDate.ts` |
| 常量 | UPPER_SNAKE_CASE | `ROUTES.ADMIN_LOGIN` |
| 类型 | PascalCase | `type DeviceStatus` |

### 文件组织

- 一个功能模块一个目录 (`features/<module>/`)
- 每个模块目录包含 `index.ts` 导出文件
- 相关类型定义放在 `types/` 目录

## 🔧 可用脚本

| 命令 | 说明 |
|------|------|
| `pnpm dev` | 启动开发服务器 (Vite HMR) |
| `pnpm build` | TypeScript 编译 + Vite 生产构建 |
| `pnpm preview` | 本地预览生产构建 |
| `pnpm lint` | ESLint 代码检查 |

## 📦 状态管理

项目使用 **Zustand** 进行状态管理，配合 `persist` 中间件实现持久化。

### Store 设计原则

1. **按功能划分 Store**：`authStore`, `profileStore`, `adminStore` 等
2. **使用 persist 中间件**：关键状态自动持久化到 localStorage
3. **partialize 选择性持久化**：只持久化必要字段

### 示例

```typescript
import { create } from 'zustand'
import { persist } from 'zustand/middleware'

interface ExampleState {
  data: string | null
  setData: (data: string) => void
}

export const useExampleStore = create<ExampleState>()(
  persist(
    (set) => ({
      data: null,
      setData: (data) => set({ data }),
    }),
    {
      name: 'example-storage',
      partialize: (state) => ({ data: state.data }),
    }
  )
)
```

## 🌐 API 请求

### 请求封装

所有 API 请求通过 `src/api/client.ts` 中的 Axios 实例发送：

- **请求拦截器**：自动添加 `Authorization` header
- **响应拦截器**：401 错误自动跳转登录页

### 添加新 API

1. 在 `src/api/` 下创建或编辑对应模块文件
2. 在 `src/types/` 下定义响应类型
3. 使用 TanStack Query 封装请求（如需缓存）

```typescript
// src/api/example.ts
import apiClient from './client'
import type { ExampleResponse } from '@/types'

export const exampleApi = {
  getData: async (): Promise<ExampleResponse> => {
    const response = await apiClient.get('/example')
    return response.data
  },
}
```

## 🛡️ 安全注意事项

1. **敏感信息**：不要在前端代码中硬编码 API Key 或密码
2. **Token 存储**：当前使用 localStorage，注意 XSS 防护
3. **用户输入**：所有用户输入需在后端验证

## 🧪 测试

*(待完善)*

```bash
# 运行单元测试
pnpm test

# 运行 E2E 测试
pnpm test:e2e
```

## 🚀 提交规范

遵循 Conventional Commits 规范：

```
<type>: <description>

[optional body]
```

类型：
- `feat`: 新功能
- `fix`: Bug 修复
- `docs`: 文档更新
- `style`: 代码格式 (不影响逻辑)
- `refactor`: 重构
- `test`: 测试相关
- `chore`: 构建/工具链

示例：
```
feat: 添加设备批量导入功能

- 支持 CSV 文件上传
- 添加进度提示
```

## 🔗 相关资源

- [React 19 文档](https://react.dev/)
- [Vite 文档](https://vite.dev/)
- [Zustand 文档](https://zustand.docs.pmnd.rs/)
- [TanStack Query 文档](https://tanstack.com/query/)
- [Tailwind CSS 文档](https://tailwindcss.com/)
