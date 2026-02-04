import { create } from 'zustand'
import type { Profile } from '@/types'

interface ProfileState {
  profile: Profile | null
  isLoading: boolean
  error: string | null

  // Actions
  setProfile: (profile: Profile) => void
  setLoading: (loading: boolean) => void
  setError: (error: string | null) => void
  reset: () => void
}

export const useProfileStore = create<ProfileState>((set) => ({
  profile: null,
  isLoading: false,
  error: null,

  setProfile: (profile: Profile) => {
    set({ profile, error: null })
  },

  setLoading: (isLoading: boolean) => {
    set({ isLoading })
  },

  setError: (error: string | null) => {
    set({ error, isLoading: false })
  },

  reset: () => {
    set({ profile: null, isLoading: false, error: null })
  },
}))

export default useProfileStore
