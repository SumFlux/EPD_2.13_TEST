/**
 * 日志工具 - 生产环境禁用 console 输出
 */
const isDev = import.meta.env.DEV

export const logger = {
  error: (message: string, ...args: unknown[]) => {
    if (isDev) {
      console.error(`[ERROR] ${message}`, ...args)
    }
    // 生产环境可以接入日志服务（如 Sentry）
  },

  warn: (message: string, ...args: unknown[]) => {
    if (isDev) {
      console.warn(`[WARN] ${message}`, ...args)
    }
  },

  info: (message: string, ...args: unknown[]) => {
    if (isDev) {
      console.info(`[INFO] ${message}`, ...args)
    }
  },

  debug: (message: string, ...args: unknown[]) => {
    if (isDev) {
      console.debug(`[DEBUG] ${message}`, ...args)
    }
  }
}

export default logger
