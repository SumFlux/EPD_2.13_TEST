import apiClient from './client'
import type { Profile, ProfileRequest } from '@/types'

export const profileApi = {
  /**
   * 获取用户档案
   */
  get: async (): Promise<Profile> => {
    const response = await apiClient.get<Profile>('/profile')
    return response.data
  },

  /**
   * 创建或更新用户档案
   */
  save: async (data: ProfileRequest): Promise<Profile> => {
    const response = await apiClient.post<Profile>('/profile', data)
    return response.data
  },
}

export default profileApi
