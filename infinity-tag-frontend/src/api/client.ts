import axios, { type AxiosError, type InternalAxiosRequestConfig } from 'axios'
import { ROUTES } from '@/constants/routes'

const API_BASE_URL = import.meta.env.VITE_API_BASE_URL || '/api/v1'

export const apiClient = axios.create({
  baseURL: API_BASE_URL,
  timeout: 30000,
  headers: {
    'Content-Type': 'application/json',
  },
})

// 请求拦截器 - 添加 token
apiClient.interceptors.request.use(
  (config: InternalAxiosRequestConfig) => {
    // 优先使用用户 token，如果没有则尝试使用管理员 token
    let token = localStorage.getItem('access_token')

    // 如果是 admin 接口，尝试从 adminStore 获取 token
    if (!token && config.url?.startsWith('/admin')) {
      const adminStoreData = localStorage.getItem('infinity-tag-admin')
      if (adminStoreData) {
        try {
          const parsed = JSON.parse(adminStoreData)
          token = parsed.state?.adminToken
        } catch {
          // 解析失败，忽略
        }
      }
    }

    if (token && config.headers) {
      config.headers.Authorization = `Bearer ${token}`
    }
    return config
  },
  (error: AxiosError) => {
    return Promise.reject(error)
  }
)

// 响应拦截器 - 处理错误
apiClient.interceptors.response.use(
  (response) => response,
  (error: AxiosError) => {
    if (error.response?.status === 401) {
      const requestUrl = error.config?.url || ''

      // 根据请求路径决定跳转目标
      if (requestUrl.startsWith('/admin')) {
        // 管理员接口 401，清除管理员状态并跳转
        localStorage.removeItem('infinity-tag-admin')
        window.location.href = ROUTES.ADMIN_LOGIN
      } else {
        // 用户接口 401，清除所有用户状态并跳转
        localStorage.removeItem('access_token')
        localStorage.removeItem('device_id')
        localStorage.removeItem('infinity-tag-auth')  // 清除 Zustand persist 状态
        window.location.href = ROUTES.SETUP
      }
    }
    return Promise.reject(error)
  }
)

// 工具函数：创建 FormData
export function createFormData(
  file: File,
  options?: Record<string, unknown>
): FormData {
  const formData = new FormData()
  formData.append('file', file)  // 后端期望字段名为 'file'

  // 后端期望 options 是一个 JSON 字符串
  if (options && Object.keys(options).length > 0) {
    formData.append('options', JSON.stringify(options))
  }

  return formData
}

export default apiClient
