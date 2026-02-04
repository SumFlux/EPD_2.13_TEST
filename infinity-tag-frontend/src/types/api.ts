// ============================================
// Infinity Tag API 类型定义
// ============================================

// ---------- 通用响应 ----------
export interface ApiResponse<T> {
  success: boolean
  data?: T
  error?: string
  message?: string
}

// ---------- 认证相关 ----------
export interface ActivateRequest {
  device_id?: string
  password: string
}

export interface LoginRequest {
  device_id: string
  password: string
}

export interface AuthResponse {
  access_token: string
  token_type: 'bearer'
  device_secret?: string // 仅激活时返回
  user_id: number
  device_id: string
}

export interface UserInfo {
  id: number
  device_id: string
  created_at: string
}

// ---------- 用户档案 ----------
export type Gender = 0 | 1 // 0: 女, 1: 男

export interface ProfileRequest {
  nickname: string
  gender: Gender
  birth_year: number
  birth_month: number
  birth_day: number
  birth_hour: number // -1 表示未知, 0-23 时辰
  is_lunar: boolean
  occupation?: string
  notes?: string
}

export interface Profile extends ProfileRequest {
  id: number
  user_id: number
  created_at: string
  updated_at?: string
}

// ---------- 黄历 ----------
export interface AlmanacGenerateRequest {
  target_date?: string // YYYY-MM-DD，默认今天
  force_regenerate?: boolean
}

export interface Almanac {
  id: number
  user_id: number
  date: string
  lunar_date: string
  ganzhi_year: string
  ganzhi_month: string
  ganzhi_day: string
  favorable: string // 宜
  unfavorable: string // 忌
  lucky_direction: string // 吉方
  lucky_item: string // 吉物
  energy_level: number // 0-100
  commentary: string // 运势解读
  generated_at: string
  created_at: string
}

export interface AlmanacHistoryRequest {
  days?: number // 默认30天
}

// ---------- 图片管理 ----------
export type RotateAngle = 0 | 90 | 180 | 270

export interface ImageOptions {
  rotate?: RotateAngle
  crop_x?: number
  crop_y?: number
  crop_w?: number
  crop_h?: number
  invert?: boolean
  dither?: boolean // true=抖动, false=阈值
  threshold?: number // 0-255
}

export interface ImagePreviewRequest extends ImageOptions {
  image: File
}

export interface ImageUploadRequest extends ImageOptions {
  image: File
}

export interface ImageItem {
  id: number
  url: string
  display_order: number
  view_count: number
  created_at: string
}

export interface ImageReorderRequest {
  image_ids: number[]
}

// ---------- 占卜功能 ----------
export type DivinationMode = '本命' | '客座'

export interface DivinationWordsRequest {
  count?: number // 默认8
  category?: string
}

export interface DivinationCategory {
  name: string
  description?: string
}

export interface DivinationInterpretRequest {
  mode: DivinationMode
  intent: string // 求测意图，最多10字
  words: [string, string] // 选中的两个字
}

export interface DivinationResult {
  id: number
  mode: DivinationMode
  intent: string
  selected_words: [string, string]
  result_idiom: string
  result_interpretation: string
  result_advice: string
  created_at: string
}

// ---------- 服务端渲染 ----------
export interface RendererPreviewRequest {
  target_date?: string // YYYY-MM-DD
}
