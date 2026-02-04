import apiClient from './client'
import type { Profile, ProfileRequest } from '@/types'

export const profileApi = {
  /**
   * 获取用户档案
   */
  get: async (): Promise<Profile> => {
    const response = await apiClient.get<Profile>('/profile/')
    return response.data
  },

  /**
   * 获取用户档案 (别名，兼容其他调用)
   */
  getProfile: async (): Promise<Profile | null> => {
    try {
      const response = await apiClient.get<Profile>('/profile/')
      return response.data
    } catch (err: unknown) {
      const axiosError = err as { response?: { status?: number } }
      if (axiosError.response?.status === 404) {
        return null
      }
      throw err
    }
  },

  /**
   * 创建或更新用户档案
   */
  save: async (data: ProfileRequest): Promise<Profile> => {
    const response = await apiClient.post<Profile>('/profile/', data)
    return response.data
  },
}

export default profileApi
