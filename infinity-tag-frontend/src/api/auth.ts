import apiClient from './client'
import type {
  ActivateRequest,
  ActivateResponse,
  SetPasswordRequest,
  SetPasswordResponse,
  LoginRequest,
  LoginResponse,
  UserInfo
} from '@/types'

export const authApi = {
  /**
   * 第一步：设备激活（验证设备码+初始密码）
   * 返回临时 token，需要继续调用 setPassword
   */
  activate: async (data: ActivateRequest): Promise<ActivateResponse> => {
    const response = await apiClient.post<ActivateResponse>('/auth/activate', data)
    return response.data
  },

  /**
   * 第二步：设置用户密码
   * 需要使用激活返回的 temp_token 作为 Authorization
   */
  setPassword: async (data: SetPasswordRequest, tempToken: string): Promise<SetPasswordResponse> => {
    const response = await apiClient.post<SetPasswordResponse>('/auth/set-password', data, {
      headers: {
        Authorization: `Bearer ${tempToken}`
      }
    })
    return response.data
  },

  /**
   * 设备登录
   */
  login: async (data: LoginRequest): Promise<LoginResponse> => {
    const response = await apiClient.post<LoginResponse>('/auth/login', data)
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
