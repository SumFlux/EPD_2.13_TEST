/**
 * 路由常量 - 集中管理应用路由路径
 */
export const ROUTES = {
  // 公开页面
  HOME: '/',
  SETUP: '/setup',
  SETUP_PASSWORD: '/setup/password',

  // 管理后台
  ADMIN_LOGIN: '/admin/login',
  ADMIN_DASHBOARD: '/admin/dashboard',
  ADMIN_DEVICES: '/admin/devices',
  ADMIN_USERS: '/admin/users',
} as const

export type RouteKey = keyof typeof ROUTES
export type RoutePath = (typeof ROUTES)[RouteKey]
