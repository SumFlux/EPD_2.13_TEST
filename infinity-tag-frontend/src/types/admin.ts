// ============================================
// 管理员相关类型定义
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

// ---------- 设备管理 ----------
export interface DeviceCreateRequest {
  uuid: string
  batch_name?: string
  notes?: string
}

export interface DeviceCreateResponse {
  id: number
  device_code: string
  init_password: string
  uuid: string
  batch_name?: string
  created_at: string
}

export interface DeviceBatchImportRequest {
  uuids: string[]
  batch_name?: string
}

export interface DeviceBatchImportResponse {
  success_count: number
  failed_count: number
  devices: DeviceCreateResponse[]
  errors: string[]
}

export interface Device {
  id: number
  device_code: string
  uuid: string
  status: 'pending' | 'activated' | 'disabled'
  user_id?: number
  batch_name?: string
  created_at: string
  activated_at?: string
  notes?: string
}

export interface DeviceListResponse {
  total: number
  page: number
  page_size: number
  devices: Device[]
}

// ---------- 用户管理 ----------
export interface AdminUser {
  id: number
  device_id: string
  password_set: boolean
  activated_at?: string
  last_login_at?: string
  created_at: string
}

export interface AdminUserDetail extends AdminUser {
  device_code?: string
  device_status?: string
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
