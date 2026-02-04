import { create } from 'zustand'
import { persist } from 'zustand/middleware'
import type { AuthResponse, UserInfo } from '@/types'

interface AuthState {
  token: string | null
  deviceId: string | null
  deviceSecret: string | null
  user: UserInfo | null
  isAuthenticated: boolean

  // Actions
  setAuth: (response: AuthResponse) => void
  setUser: (user: UserInfo) => void
  logout: () => void
}

export const useAuthStore = create<AuthState>()(
  persist(
    (set) => ({
      token: null,
      deviceId: null,
      deviceSecret: null,
      user: null,
      isAuthenticated: false,

      setAuth: (response: AuthResponse) => {
        localStorage.setItem('access_token', response.access_token)
        localStorage.setItem('device_id', response.device_id)
        if (response.device_secret) {
          localStorage.setItem('device_secret', response.device_secret)
        }

        set({
          token: response.access_token,
          deviceId: response.device_id,
          deviceSecret: response.device_secret || null,
          isAuthenticated: true,
        })
      },

      setUser: (user: UserInfo) => {
        set({ user })
      },

      logout: () => {
        localStorage.removeItem('access_token')
        localStorage.removeItem('device_id')
        localStorage.removeItem('device_secret')

        set({
          token: null,
          deviceId: null,
          deviceSecret: null,
          user: null,
          isAuthenticated: false,
        })
      },
    }),
    {
      name: 'infinity-tag-auth',
      partialize: (state) => ({
        token: state.token,
        deviceId: state.deviceId,
        isAuthenticated: state.isAuthenticated,
      }),
    }
  )
)

export default useAuthStore
