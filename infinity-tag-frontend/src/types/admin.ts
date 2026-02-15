// ============================================
// 管理员相关类型定义 - 设备即用户架构
// ============================================

// 从 api.ts 导入共享的认证类型，避免重复定义
export type {
  ActivateRequest,
  ActivateResponse,
  SetPasswordRequest,
  SetPasswordResponse,
  LoginRequest,
  LoginResponse,
} from './api'

// ---------- 管理员认证 ----------
export interface AdminLoginRequest {
  username: string
  password: string
}

export interface AdminLoginResponse {
  access_token: string
  token_type: 'bearer'
  expires_in: number
}

// ---------- 设备管理（基于 User 模型）----------
export interface DeviceCreateRequest {
  device_code: string
  uuid: string
  init_password: string
}

export interface DeviceCreateResponse {
  id: number
  device_code: string
  uuid: string
  status: string
  created_at: string
}

export interface DeviceBatchItem {
  device_code: string
  uuid: string
  init_password: string
}

export interface DeviceBatchImportRequest {
  devices: DeviceBatchItem[]
}

export interface DeviceBatchImportResponse {
  success_count: number
  failed_count: number
  errors?: string[]
}

export interface Device {
  id: number
  device_code: string
  uuid: string
  status: 'pending' | 'activated' | 'disabled'
  password_set: boolean
  activated_at?: string
  last_login_at?: string
  created_at: string
}

export interface DeviceListResponse {
  total: number
  page: number
  page_size: number
  users: Device[]  // 注意：后端返回的是 users 字段
}

// ---------- 用户管理（设备即用户）----------
export interface AdminUser {
  id: number
  device_code: string
  uuid: string
  status: string
  password_set: boolean
  activated_at?: string
  last_login_at?: string
  created_at: string
}

export interface AdminUserDetail extends AdminUser {
  // 用户详情与基础信息相同
}

export interface AdminUserListResponse {
  total: number
  page: number
  page_size: number
  users: AdminUser[]
}

// ---------- 统计数据 ----------
export interface AdminStats {
  total_devices: number
  pending_devices: number
  activated_devices: number
  disabled_devices: number
  total_users: number
  users_with_password: number
}

// ---------- 固件管理 ----------
export interface FirmwareResponse {
  id: number
  version_str: string
  version_code: number
  file_path: string
  checksum: string
  description?: string
  is_active: boolean
  created_at: string
}
