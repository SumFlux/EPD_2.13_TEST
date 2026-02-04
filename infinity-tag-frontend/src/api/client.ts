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
    const token = localStorage.getItem('access_token')
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
      // token 过期，清除并跳转登录
      localStorage.removeItem('access_token')
      localStorage.removeItem('device_id')
      window.location.href = ROUTES.SETUP
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
  formData.append('image', file)

  if (options) {
    Object.entries(options).forEach(([key, value]) => {
      if (value !== undefined && value !== null) {
        formData.append(key, String(value))
      }
    })
  }

  return formData
}

export default apiClient
