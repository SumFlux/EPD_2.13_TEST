import { create } from 'zustand'
import { persist } from 'zustand/middleware'
import type { SetPasswordResponse, LoginResponse, UserInfo } from '@/types'

interface AuthState {
  token: string | null
  deviceId: string | null
  user: UserInfo | null
  isAuthenticated: boolean

  // Actions
  setAuthFromLogin: (response: LoginResponse) => void
  setAuthFromSetPassword: (response: SetPasswordResponse) => void
  setUser: (user: UserInfo) => void
  logout: () => void
}

export const useAuthStore = create<AuthState>()(
  persist(
    (set) => ({
      token: null,
      deviceId: null,
      user: null,
      isAuthenticated: false,

      setAuthFromLogin: (response: LoginResponse) => {
        localStorage.setItem('access_token', response.access_token)

        set({
          token: response.access_token,
          isAuthenticated: true,
        })
      },

      setAuthFromSetPassword: (response: SetPasswordResponse) => {
        localStorage.setItem('access_token', response.access_token)
        localStorage.setItem('device_id', response.device_id)

        set({
          token: response.access_token,
          deviceId: response.device_id,
          isAuthenticated: true,
        })
      },

      setUser: (user: UserInfo) => {
        set({ user })
      },

      logout: () => {
        localStorage.removeItem('access_token')
        localStorage.removeItem('device_id')

        set({
          token: null,
          deviceId: null,
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
