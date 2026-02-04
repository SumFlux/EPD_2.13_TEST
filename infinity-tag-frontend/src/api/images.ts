import apiClient, { createFormData } from './client'
import type { ImageItem, ImageOptions } from '@/types'

// 获取后端基础 URL（用于静态资源）
const getBackendBaseUrl = (): string => {
  const apiUrl = import.meta.env.VITE_API_BASE_URL || '/api/v1'
  // 如果是相对路径，使用当前域名
  if (apiUrl.startsWith('/')) {
    return ''
  }
  // 如果是完整 URL，提取基础部分 (去掉 /api/v1)
  return apiUrl.replace(/\/api\/v1$/, '')
}

// 转换图片 URL 为完整路径
const normalizeImageUrl = (url: string): string => {
  if (url.startsWith('http')) {
    return url
  }
  const baseUrl = getBackendBaseUrl()
  return `${baseUrl}${url}`
}

// 处理图片列表，转换 URL
const normalizeImageItem = (item: ImageItem): ImageItem => ({
  ...item,
  url: normalizeImageUrl(item.url)
})

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
    return normalizeImageItem(response.data)
  },

  /**
   * 获取图片列表
   */
  list: async (): Promise<ImageItem[]> => {
    const response = await apiClient.get<ImageItem[]>('/images/')
    return response.data.map(normalizeImageItem)
  },

  /**
   * 删除图片
   */
  delete: async (id: number): Promise<void> => {
    await apiClient.delete(`/images/${id}`)
  },

  /**
   * 重新排序图片
   * 后端期望格式: [{ id: number, new_order: number }, ...]
   */
  reorder: async (imageIds: number[]): Promise<void> => {
    const data = imageIds.map((id, index) => ({ id, new_order: index }))
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
