import { create } from 'zustand'
import type { Almanac } from '@/types'

interface AlmanacState {
  today: Almanac | null
  history: Almanac[]
  selectedDate: string | null
  isLoading: boolean
  error: string | null

  // Actions
  setToday: (almanac: Almanac) => void
  setHistory: (almanacs: Almanac[]) => void
  setSelectedDate: (date: string | null) => void
  setLoading: (loading: boolean) => void
  setError: (error: string | null) => void
  reset: () => void
}

export const useAlmanacStore = create<AlmanacState>((set) => ({
  today: null,
  history: [],
  selectedDate: null,
  isLoading: false,
  error: null,

  setToday: (almanac: Almanac) => {
    set({ today: almanac, error: null })
  },

  setHistory: (history: Almanac[]) => {
    set({ history, error: null })
  },

  setSelectedDate: (selectedDate: string | null) => {
    set({ selectedDate })
  },

  setLoading: (isLoading: boolean) => {
    set({ isLoading })
  },

  setError: (error: string | null) => {
    set({ error, isLoading: false })
  },

  reset: () => {
    set({
      today: null,
      history: [],
      selectedDate: null,
      isLoading: false,
      error: null,
    })
  },
}))

export default useAlmanacStore
