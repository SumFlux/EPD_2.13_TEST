import apiClient from './client'

export const rendererApi = {
  /**
   * 获取黄历渲染预览图
   */
  preview: async (targetDate?: string): Promise<Blob> => {
    const response = await apiClient.get('/renderer/preview', {
      params: targetDate ? { target_date: targetDate } : undefined,
      responseType: 'blob',
    })
    return response.data
  },
}

export default rendererApi
