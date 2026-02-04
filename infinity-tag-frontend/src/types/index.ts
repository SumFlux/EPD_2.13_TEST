export * from './api'

// 应用内部类型
export interface RouteConfig {
  path: string
  title: string
  icon?: string
}

// 表单状态
export type FormStatus = 'idle' | 'loading' | 'success' | 'error'

// 时辰选项
export interface HourOption {
  value: number
  label: string
  range: string
}

export const HOUR_OPTIONS: HourOption[] = [
  { value: -1, label: '未知', range: '' },
  { value: 0, label: '子时', range: '23:00-01:00' },
  { value: 1, label: '丑时', range: '01:00-03:00' },
  { value: 3, label: '寅时', range: '03:00-05:00' },
  { value: 5, label: '卯时', range: '05:00-07:00' },
  { value: 7, label: '辰时', range: '07:00-09:00' },
  { value: 9, label: '巳时', range: '09:00-11:00' },
  { value: 11, label: '午时', range: '11:00-13:00' },
  { value: 13, label: '未时', range: '13:00-15:00' },
  { value: 15, label: '申时', range: '15:00-17:00' },
  { value: 17, label: '酉时', range: '17:00-19:00' },
  { value: 19, label: '戌时', range: '19:00-21:00' },
  { value: 21, label: '亥时', range: '21:00-23:00' },
]

// 职业选项
export const OCCUPATION_OPTIONS = [
  '学生',
  '程序员/工程师',
  '设计师',
  '产品经理',
  '运营/市场',
  '金融/财务',
  '教师/学者',
  '医生/护士',
  '自由职业',
  '创业者',
  '其他',
] as const

export type Occupation = typeof OCCUPATION_OPTIONS[number]
