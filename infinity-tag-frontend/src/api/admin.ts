import apiClient from './client'
import type {
  AdminLoginRequest,
  AdminLoginResponse,
  DeviceCreateRequest,
  DeviceCreateResponse,
  DeviceBatchImportRequest,
  DeviceBatchImportResponse,
  Device,
  DeviceListResponse,
  AdminUserListResponse,
  AdminUserDetail,
  AdminStats
} from '@/types/admin'

export const adminApi = {
  /**
   * 管理员登录
   */
  login: async (data: AdminLoginRequest): Promise<AdminLoginResponse> => {
    const response = await apiClient.post<AdminLoginResponse>('/admin/login', data)
    return response.data
  },

  /**
   * 获取统计数据
   */
  getStats: async (): Promise<AdminStats> => {
    const response = await apiClient.get<AdminStats>('/admin/stats')
    return response.data
  },

  /**
   * 获取设备列表
   */
  getDevices: async (params?: {
    page?: number
    page_size?: number
    status?: string
    batch_name?: string
  }): Promise<DeviceListResponse> => {
    const response = await apiClient.get<DeviceListResponse>('/admin/devices', { params })
    return response.data
  },

  /**
   * 录入单个设备
   */
  createDevice: async (data: DeviceCreateRequest): Promise<DeviceCreateResponse> => {
    const response = await apiClient.post<DeviceCreateResponse>('/admin/devices', data)
    return response.data
  },

  /**
   * 批量导入设备
   */
  batchImportDevices: async (data: DeviceBatchImportRequest): Promise<DeviceBatchImportResponse> => {
    const response = await apiClient.post<DeviceBatchImportResponse>('/admin/devices/batch', data)
    return response.data
  },

  /**
   * 获取设备详情
   */
  getDevice: async (deviceId: number): Promise<Device> => {
    const response = await apiClient.get<Device>(`/admin/devices/${deviceId}`)
    return response.data
  },

  /**
   * 禁用设备
   */
  disableDevice: async (deviceId: number): Promise<Device> => {
    const response = await apiClient.put<Device>(`/admin/devices/${deviceId}/disable`)
    return response.data
  },

  /**
   * 重置设备
   */
  resetDevice: async (deviceId: number): Promise<Device> => {
    const response = await apiClient.put<Device>(`/admin/devices/${deviceId}/reset`)
    return response.data
  },

  /**
   * 删除设备
   */
  deleteDevice: async (deviceId: number): Promise<void> => {
    await apiClient.delete(`/admin/devices/${deviceId}`)
  },

  /**
   * 获取用户列表
   */
  getUsers: async (params?: {
    page?: number
    page_size?: number
  }): Promise<AdminUserListResponse> => {
    const response = await apiClient.get<AdminUserListResponse>('/admin/users', { params })
    return response.data
  },

  /**
   * 获取用户详情
   */
  getUserDetail: async (userId: number): Promise<AdminUserDetail> => {
    const response = await apiClient.get<AdminUserDetail>(`/admin/users/${userId}`)
    return response.data
  },

  /**
   * 禁用用户
   */
  disableUser: async (userId: number): Promise<void> => {
    await apiClient.put(`/admin/users/${userId}/disable`)
  },

  /**
   * 上传固件 (OTA)
   */
  uploadFirmware: async (formData: FormData): Promise<any> => {
    // 注意: 必须显式移除 Content-Type，否则 client.ts 的默认 application/json 会被使用，导致 boundary 丢失
    const response = await apiClient.post('/ota/upload', formData, {
      headers: {
        'Content-Type': undefined
      }
    })
    return response.data
  }
}

export default adminApi
