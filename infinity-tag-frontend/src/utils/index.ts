import { clsx, type ClassValue } from 'clsx'
import { twMerge } from 'tailwind-merge'

/**
 * 合并 Tailwind 类名，处理冲突
 */
export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

/**
 * 格式化日期为 YYYY-MM-DD
 */
export function formatDate(date: Date | string): string {
  const d = typeof date === 'string' ? new Date(date) : date
  return d.toISOString().split('T')[0]
}

/**
 * 获取能量等级对应的颜色类名
 */
export function getEnergyColor(level: number): string {
  if (level >= 70) return 'bg-energy-high'
  if (level >= 40) return 'bg-energy-medium'
  return 'bg-energy-low'
}

/**
 * 获取能量等级描述
 */
export function getEnergyLabel(level: number): string {
  if (level >= 80) return '大吉'
  if (level >= 60) return '中吉'
  if (level >= 40) return '小吉'
  if (level >= 20) return '平'
  return '凶'
}

/**
 * 延迟函数
 */
export function delay(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms))
}

/**
 * 安全解析 JSON
 */
export function safeJsonParse<T>(json: string, fallback: T): T {
  try {
    return JSON.parse(json) as T
  } catch {
    return fallback
  }
}
