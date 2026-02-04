import { useEffect, useState, useRef } from 'react'
import { useImagesStore } from '@/stores'
import { imagesApi } from '@/api'
import type { ImageItem, ImageOptions } from '@/types'

const MAX_IMAGES = 5

export default function ImagesPage() {
  const { images, setImages, addImage, removeImage, isLoading, setLoading, error, setError } = useImagesStore()
  const [previewUrl, setPreviewUrl] = useState<string | null>(null)
  const [selectedFile, setSelectedFile] = useState<File | null>(null)
  const [options, setOptions] = useState<ImageOptions>({
    rotate: 0,
    invert: false,
    dither: true,
    threshold: 128,
  })
  const fileInputRef = useRef<HTMLInputElement>(null)
  const previewTimeoutRef = useRef<number>()

  useEffect(() => {
    const fetchImages = async () => {
      setLoading(true)
      try {
        const data = await imagesApi.list()
        setImages(data)
      } catch {
        setError('加载图片列表失败')
      } finally {
        setLoading(false)
      }
    }
    fetchImages()
  }, [setImages, setLoading, setError])

  // 实时预览
  useEffect(() => {
    if (!selectedFile) {
      setPreviewUrl(null)
      return
    }

    // 防抖
    if (previewTimeoutRef.current) {
      clearTimeout(previewTimeoutRef.current)
    }

    previewTimeoutRef.current = window.setTimeout(async () => {
      try {
        const blob = await imagesApi.preview(selectedFile, options)
        const url = URL.createObjectURL(blob)
        setPreviewUrl((prev) => {
          if (prev) URL.revokeObjectURL(prev)
          return url
        })
      } catch {
        setError('预览生成失败')
      }
    }, 300)

    return () => {
      if (previewTimeoutRef.current) {
        clearTimeout(previewTimeoutRef.current)
      }
    }
  }, [selectedFile, options, setError])

  const handleFileSelect = (e: React.ChangeEvent<HTMLInputElement>) => {
    const file = e.target.files?.[0]
    if (file) {
      setSelectedFile(file)
      setError(null)
    }
  }

  const handleUpload = async () => {
    if (!selectedFile) return

    if (images.length >= MAX_IMAGES) {
      setError(`最多只能上传 ${MAX_IMAGES} 张图片`)
      return
    }

    setLoading(true)
    try {
      const newImage = await imagesApi.upload(selectedFile, options)
      addImage(newImage)
      setSelectedFile(null)
      if (fileInputRef.current) {
        fileInputRef.current.value = ''
      }
    } catch {
      setError('上传失败')
    } finally {
      setLoading(false)
    }
  }

  const handleDelete = async (id: number) => {
    if (!confirm('确定要删除这张图片吗？')) return

    try {
      await imagesApi.delete(id)
      removeImage(id)
    } catch {
      setError('删除失败')
    }
  }

  const handleOptionChange = <K extends keyof ImageOptions>(key: K, value: ImageOptions[K]) => {
    setOptions((prev) => ({ ...prev, [key]: value }))
  }

  return (
    <div className="min-h-screen p-4">
      <div className="max-w-2xl mx-auto">
        <h1 className="text-2xl font-bold text-center mb-6">图片管理</h1>

        {error && (
          <div className="card mb-4 text-accent-secondary text-center">{error}</div>
        )}

        {/* 上传区域 */}
        <div className="card mb-6">
          <div className="mb-4">
            <label className="block text-sm text-text-secondary mb-2">选择图片</label>
            <input
              ref={fileInputRef}
              type="file"
              accept="image/*"
              onChange={handleFileSelect}
              className="block w-full text-sm text-text-secondary
                file:mr-4 file:py-2 file:px-4
                file:rounded file:border-0
                file:bg-accent-primary file:text-text-primary
                hover:file:bg-accent-primary/90
                file:cursor-pointer"
            />
          </div>

          {selectedFile && (
            <>
              {/* 预览 */}
              {previewUrl && (
                <div className="mb-4 flex justify-center">
                  <img
                    src={previewUrl}
                    alt="预览"
                    className="max-w-full max-h-64 border border-border rounded"
                  />
                </div>
              )}

              {/* 处理选项 */}
              <div className="space-y-4 mb-4">
                {/* 旋转 */}
                <div>
                  <label className="block text-sm text-text-secondary mb-1">旋转</label>
                  <div className="flex gap-2">
                    {([0, 90, 180, 270] as const).map((angle) => (
                      <button
                        key={angle}
                        onClick={() => handleOptionChange('rotate', angle)}
                        className={`px-3 py-1 rounded text-sm transition-colors ${
                          options.rotate === angle
                            ? 'bg-accent-primary text-text-primary'
                            : 'bg-background-secondary text-text-secondary'
                        }`}
                      >
                        {angle}°
                      </button>
                    ))}
                  </div>
                </div>

                {/* 反色 */}
                <label className="flex items-center gap-2 cursor-pointer">
                  <input
                    type="checkbox"
                    checked={options.invert}
                    onChange={(e) => handleOptionChange('invert', e.target.checked)}
                    className="accent-accent-primary"
                  />
                  <span className="text-sm">反色</span>
                </label>

                {/* 抖动/阈值 */}
                <div>
                  <label className="flex items-center gap-2 cursor-pointer mb-2">
                    <input
                      type="checkbox"
                      checked={options.dither}
                      onChange={(e) => handleOptionChange('dither', e.target.checked)}
                      className="accent-accent-primary"
                    />
                    <span className="text-sm">抖动算法（推荐）</span>
                  </label>
                  {!options.dither && (
                    <div>
                      <label className="block text-sm text-text-secondary mb-1">
                        阈值: {options.threshold}
                      </label>
                      <input
                        type="range"
                        min="0"
                        max="255"
                        value={options.threshold}
                        onChange={(e) => handleOptionChange('threshold', Number(e.target.value))}
                        className="w-full accent-accent-primary"
                      />
                    </div>
                  )}
                </div>
              </div>

              <button
                onClick={handleUpload}
                disabled={isLoading}
                className="btn-seal w-full disabled:opacity-50"
              >
                {isLoading ? '上传中...' : '上传图片'}
              </button>
            </>
          )}
        </div>

        {/* 图片列表 */}
        <div>
          <div className="flex justify-between items-center mb-4">
            <h2 className="text-lg font-bold">已上传图片</h2>
            <span className="text-sm text-text-secondary">{images.length}/{MAX_IMAGES}</span>
          </div>

          {images.length === 0 ? (
            <div className="card text-center text-text-secondary">暂无图片</div>
          ) : (
            <div className="grid grid-cols-2 gap-4">
              {images.map((image) => (
                <ImageCard key={image.id} image={image} onDelete={handleDelete} />
              ))}
            </div>
          )}
        </div>
      </div>
    </div>
  )
}

function ImageCard({
  image,
  onDelete,
}: {
  image: ImageItem
  onDelete: (id: number) => void
}) {
  return (
    <div className="card p-2">
      <img
        src={image.url}
        alt={`图片 ${image.id}`}
        className="w-full aspect-square object-cover rounded mb-2"
      />
      <div className="flex justify-between items-center text-sm">
        <span className="text-text-secondary">查看: {image.view_count}</span>
        <button
          onClick={() => onDelete(image.id)}
          className="text-accent-secondary hover:text-accent-secondary/80 transition-colors"
        >
          删除
        </button>
      </div>
    </div>
  )
}
