import apiClient from './client'
import type { Almanac, AlmanacGenerateRequest } from '@/types'

export const almanacApi = {
  /**
   * 生成/获取今日黄历
   */
  generate: async (data?: AlmanacGenerateRequest): Promise<Almanac> => {
    const response = await apiClient.post<Almanac>('/almanac/generate', data || {})
    return response.data
  },

  /**
   * 获取历史黄历
   */
  history: async (days: number = 30): Promise<Almanac[]> => {
    const response = await apiClient.get<Almanac[]>('/almanac/history', {
      params: { days },
    })
    return response.data
  },
}

export default almanacApi
