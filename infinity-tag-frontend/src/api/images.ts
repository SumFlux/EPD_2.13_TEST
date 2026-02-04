import apiClient, { createFormData } from './client'
import type { ImageItem, ImageOptions, ImageReorderRequest } from '@/types'

export const imagesApi = {
  /**
   * 实时预览图片处理效果（返回 PNG blob）
   */
  preview: async (file: File, options?: ImageOptions): Promise<Blob> => {
    const formData = createFormData(file, options)
    const response = await apiClient.post('/images/preview', formData, {
      headers: { 'Content-Type': 'multipart/form-data' },
      responseType: 'blob',
    })
    return response.data
  },

  /**
   * 上传图片
   */
  upload: async (file: File, options?: ImageOptions): Promise<ImageItem> => {
    const formData = createFormData(file, options)
    const response = await apiClient.post<ImageItem>('/images/', formData, {
      headers: { 'Content-Type': 'multipart/form-data' },
    })
    return response.data
  },

  /**
   * 获取图片列表
   */
  list: async (): Promise<ImageItem[]> => {
    const response = await apiClient.get<ImageItem[]>('/images/')
    return response.data
  },

  /**
   * 删除图片
   */
  delete: async (id: number): Promise<void> => {
    await apiClient.delete(`/images/${id}`)
  },

  /**
   * 重新排序图片
   */
  reorder: async (imageIds: number[]): Promise<void> => {
    const data: ImageReorderRequest = { image_ids: imageIds }
    await apiClient.put('/images/reorder', data)
  },

  /**
   * 下载设备位图
   */
  getBitmap: async (id: number): Promise<Blob> => {
    const response = await apiClient.get(`/images/${id}/bitmap`, {
      responseType: 'blob',
    })
    return response.data
  },
}

export default imagesApi
