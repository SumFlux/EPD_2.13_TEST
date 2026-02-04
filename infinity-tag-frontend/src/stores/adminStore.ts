import { create } from 'zustand'
import { persist } from 'zustand/middleware'
import type { AdminStats, Device, AdminUser } from '@/types/admin'

interface AdminState {
  adminToken: string | null
  isAdminAuthenticated: boolean
  stats: AdminStats | null
  devices: Device[]
  users: AdminUser[]
  totalDevices: number
  totalUsers: number

  // Actions
  setAdminAuth: (token: string) => void
  setStats: (stats: AdminStats) => void
  setDevices: (devices: Device[], total: number) => void
  setUsers: (users: AdminUser[], total: number) => void
  adminLogout: () => void
}

export const useAdminStore = create<AdminState>()(
  persist(
    (set) => ({
      adminToken: null,
      isAdminAuthenticated: false,
      stats: null,
      devices: [],
      users: [],
      totalDevices: 0,
      totalUsers: 0,

      setAdminAuth: (token: string) => {
        set({
          adminToken: token,
          isAdminAuthenticated: true
        })
      },

      setStats: (stats: AdminStats) => {
        set({ stats })
      },

      setDevices: (devices: Device[], total: number) => {
        set({ devices, totalDevices: total })
      },

      setUsers: (users: AdminUser[], total: number) => {
        set({ users, totalUsers: total })
      },

      adminLogout: () => {
        set({
          adminToken: null,
          isAdminAuthenticated: false,
          stats: null,
          devices: [],
          users: [],
          totalDevices: 0,
          totalUsers: 0
        })
      }
    }),
    {
      name: 'infinity-tag-admin',
      partialize: (state) => ({
        adminToken: state.adminToken,
        isAdminAuthenticated: state.isAdminAuthenticated
      })
    }
  )
)

export default useAdminStore
