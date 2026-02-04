import { create } from 'zustand'
import type { ImageItem } from '@/types'

interface ImagesState {
  images: ImageItem[]
  isLoading: boolean
  error: string | null
  uploadProgress: number

  // Actions
  setImages: (images: ImageItem[]) => void
  addImage: (image: ImageItem) => void
  removeImage: (id: number) => void
  reorderImages: (imageIds: number[]) => void
  setLoading: (loading: boolean) => void
  setError: (error: string | null) => void
  setUploadProgress: (progress: number) => void
  reset: () => void
}

export const useImagesStore = create<ImagesState>((set) => ({
  images: [],
  isLoading: false,
  error: null,
  uploadProgress: 0,

  setImages: (images: ImageItem[]) => {
    set({ images, error: null })
  },

  addImage: (image: ImageItem) => {
    set((state) => ({
      images: [...state.images, image],
      error: null,
    }))
  },

  removeImage: (id: number) => {
    set((state) => ({
      images: state.images.filter((img) => img.id !== id),
    }))
  },

  reorderImages: (imageIds: number[]) => {
    set((state) => {
      const reordered = imageIds
        .map((id, index) => {
          const image = state.images.find((img) => img.id === id)
          return image ? { ...image, display_order: index } : null
        })
        .filter((img): img is ImageItem => img !== null)
      return { images: reordered }
    })
  },

  setLoading: (isLoading: boolean) => {
    set({ isLoading })
  },

  setError: (error: string | null) => {
    set({ error, isLoading: false })
  },

  setUploadProgress: (uploadProgress: number) => {
    set({ uploadProgress })
  },

  reset: () => {
    set({
      images: [],
      isLoading: false,
      error: null,
      uploadProgress: 0,
    })
  },
}))

export default useImagesStore
