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
    const response = await apiClient.get<{ data: Almanac[]; total: number }>('/almanac/history', {
      params: { days },
    })
    return response.data.data  // 后端返回 { data: [...], total: n }
  },
}

export default almanacApi
