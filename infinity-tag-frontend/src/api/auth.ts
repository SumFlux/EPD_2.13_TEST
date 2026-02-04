import apiClient from './client'
import type { ActivateRequest, AuthResponse, LoginRequest, UserInfo } from '@/types'

export const authApi = {
  /**
   * 设备激活（首次注册）
   */
  activate: async (data: ActivateRequest): Promise<AuthResponse> => {
    const response = await apiClient.post<AuthResponse>('/auth/activate', data)
    return response.data
  },

  /**
   * 设备登录
   */
  login: async (data: LoginRequest): Promise<AuthResponse> => {
    const response = await apiClient.post<AuthResponse>('/auth/login', data)
    return response.data
  },

  /**
   * 获取当前用户信息
   */
  me: async (): Promise<UserInfo> => {
    const response = await apiClient.get<UserInfo>('/auth/me')
    return response.data
  },
}

export default authApi
