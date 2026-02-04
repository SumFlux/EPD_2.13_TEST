import apiClient from './client'
import type {
  DivinationCategory,
  DivinationInterpretRequest,
  DivinationResult,
} from '@/types'

export const divinationApi = {
  /**
   * 获取随机备选字
   */
  getWords: async (count: number = 8, category?: string): Promise<string[]> => {
    const response = await apiClient.get<string[]>('/divination/words', {
      params: { count, category },
    })
    return response.data
  },

  /**
   * 获取所有字库类别
   */
  getCategories: async (): Promise<DivinationCategory[]> => {
    const response = await apiClient.get<DivinationCategory[]>('/divination/categories')
    return response.data
  },

  /**
   * 解字测算
   */
  interpret: async (data: DivinationInterpretRequest): Promise<DivinationResult> => {
    const response = await apiClient.post<DivinationResult>('/divination/interpret', data)
    return response.data
  },
}

export default divinationApi
