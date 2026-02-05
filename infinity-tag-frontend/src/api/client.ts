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
    let token: string | null = null
    const url = config.url || ''
    const isAdminRequest = url.startsWith('/admin') || url.includes('/ota/upload')

    if (isAdminRequest) {
      // 管理员接口，只使用管理员 token
      const adminStoreData = localStorage.getItem('infinity-tag-admin')
      if (adminStoreData) {
        try {
          const parsed = JSON.parse(adminStoreData)
          token = parsed.state?.adminToken
        } catch {
          // 解析失败，忽略
        }
      }
    } else {
      // 普通接口，使用用户 token
      token = localStorage.getItem('access_token')
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
      if (requestUrl.includes('/login')) {
        // 登录接口自身的 401 错误，不跳转，交由组件处理错误提示
        return Promise.reject(error)
      } else if (requestUrl.startsWith('/admin')) {
        // 管理员接口 401，清除管理员状态并跳转
        localStorage.removeItem('infinity-tag-admin')
        window.location.href = ROUTES.ADMIN_LOGIN
      } else {
        // 用户接口 401，清除所有用户状态并跳转
        localStorage.removeItem('access_token')
        localStorage.removeItem('device_id')
        localStorage.removeItem('infinity-tag-auth')
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
