import { useEffect, useState, useRef } from 'react'
import { useImagesStore } from '@/stores'
import { imagesApi } from '@/api'
import type { ImageOptions, DitherAlgorithm } from '@/types'
import { DITHER_ALGORITHMS } from '@/types'

const MAX_IMAGES = 5

export default function ImagesPage() {
  const { images, setImages, addImage, removeImage, reorderImages, isLoading, setLoading, error, setError } = useImagesStore()
  const [previewUrl, setPreviewUrl] = useState<string | null>(null)
  const [selectedFile, setSelectedFile] = useState<File | null>(null)
  const [isReordering, setIsReordering] = useState(false)
  const [showAdvanced, setShowAdvanced] = useState(false)
  const [options, setOptions] = useState<ImageOptions>({
    rotate: 0,
    invert: false,
    dither_algorithm: 'atkinson',
    threshold: 128,
    contrast: 1.3,
    sharpness: 1.5,
    gamma: 1.2,
  })
  const fileInputRef = useRef<HTMLInputElement>(null)
  const previewTimeoutRef = useRef<number | undefined>(undefined)

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

  // 通用移动函数：direction 为 -1 表示上移，1 表示下移
  const handleMove = async (index: number, direction: -1 | 1) => {
    const targetIndex = index + direction
    if (targetIndex < 0 || targetIndex >= images.length) return

    setIsReordering(true)
    const newOrder = [...images]
    const temp = newOrder[index]
    newOrder[index] = newOrder[targetIndex]
    newOrder[targetIndex] = temp
    const imageIds = newOrder.map(img => img.id)

    try {
      await imagesApi.reorder(imageIds)
      reorderImages(imageIds)
    } catch {
      setError('排序失败')
    } finally {
      setIsReordering(false)
    }
  }

  const handleOptionChange = <K extends keyof ImageOptions>(key: K, value: ImageOptions[K]) => {
    setOptions((prev) => ({ ...prev, [key]: value }))
  }

  return (
    <div className="h-full overflow-y-auto">
      <div className="max-w-lg mx-auto p-4">
        {error && (
          <div className="card mb-4 text-accent-secondary text-center">{error}</div>
        )}

        {/* 上传区域 */}
        <div className="card mb-4">
          <div className="flex justify-between items-center mb-3">
            <h2 className="font-bold">上传图片</h2>
            <span className="text-sm text-text-secondary">{images.length}/{MAX_IMAGES}</span>
          </div>

          <div className="mb-3">
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
                <div className="mb-3 flex justify-center">
                  <img
                    src={previewUrl}
                    alt="预览"
                    className="max-w-full border border-border rounded"
                  />
                </div>
              )}

              {/* 处理选项 */}
              <div className="space-y-3 mb-3">
                {/* 旋转 */}
                <div>
                  <label className="block text-sm text-text-secondary mb-1">旋转</label>
                  <div className="flex gap-2">
                    {([0, 90, 180, 270] as const).map((angle) => (
                      <button
                        key={angle}
                        onClick={() => handleOptionChange('rotate', angle)}
                        className={`px-3 py-1 rounded text-sm transition-colors ${options.rotate === angle
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

                {/* 抖动算法选择 */}
                <div>
                  <label className="block text-sm text-text-secondary mb-1">抖动算法</label>
                  <select
                    value={options.dither_algorithm}
                    onChange={(e) => handleOptionChange('dither_algorithm', e.target.value as DitherAlgorithm)}
                    className="w-full px-3 py-2 rounded bg-background-secondary text-text-primary border border-border"
                  >
                    {DITHER_ALGORITHMS.map((algo) => (
                      <option key={algo.value} value={algo.value}>
                        {algo.label} - {algo.description}
                      </option>
                    ))}
                  </select>
                </div>

                {/* 阈值 (仅在 threshold 算法时显示) */}
                {options.dither_algorithm === 'threshold' && (
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

                {/* 高级选项折叠 */}
                <button
                  onClick={() => setShowAdvanced(!showAdvanced)}
                  className="text-sm text-accent-primary hover:underline"
                >
                  {showAdvanced ? '▼ 收起高级选项' : '▶ 展开高级选项'}
                </button>

                {showAdvanced && (
                  <div className="space-y-3 p-3 bg-background-secondary rounded">
                    {/* 对比度 */}
                    <div>
                      <label className="block text-sm text-text-secondary mb-1">
                        对比度: {options.contrast?.toFixed(1)}
                      </label>
                      <input
                        type="range"
                        min="0.5"
                        max="3.0"
                        step="0.1"
                        value={options.contrast}
                        onChange={(e) => handleOptionChange('contrast', Number(e.target.value))}
                        className="w-full accent-accent-primary"
                      />
                    </div>

                    {/* 锐化 */}
                    <div>
                      <label className="block text-sm text-text-secondary mb-1">
                        锐化: {options.sharpness?.toFixed(1)}
                      </label>
                      <input
                        type="range"
                        min="0"
                        max="5.0"
                        step="0.1"
                        value={options.sharpness}
                        onChange={(e) => handleOptionChange('sharpness', Number(e.target.value))}
                        className="w-full accent-accent-primary"
                      />
                    </div>

                    {/* Gamma */}
                    <div>
                      <label className="block text-sm text-text-secondary mb-1">
                        Gamma: {options.gamma?.toFixed(1)}
                      </label>
                      <input
                        type="range"
                        min="0.3"
                        max="3.0"
                        step="0.1"
                        value={options.gamma}
                        onChange={(e) => handleOptionChange('gamma', Number(e.target.value))}
                        className="w-full accent-accent-primary"
                      />
                    </div>
                  </div>
                )}
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

        {/* 图片列表 - 单列原比例显示 */}
        {images.length > 0 && (
          <div className="space-y-4">
            <h2 className="font-bold">已上传图片</h2>
            {images.map((image, index) => (
              <div key={image.id} className="card p-3">
                {/* 原比例图片 */}
                <img
                  src={image.url}
                  alt={`图片 ${index + 1}`}
                  className="w-full rounded mb-3"
                />

                {/* 操作按钮 */}
                <div className="flex items-center justify-between">
                  <div className="flex gap-2">
                    {/* 上移 */}
                    <button
                      onClick={() => handleMove(index, -1)}
                      disabled={index === 0 || isReordering}
                      className="px-3 py-1 rounded text-sm bg-background-secondary disabled:opacity-30 hover:bg-background-secondary/80 transition-colors"
                      title="上移"
                    >
                      ↑ 上移
                    </button>
                    {/* 下移 */}
                    <button
                      onClick={() => handleMove(index, 1)}
                      disabled={index === images.length - 1 || isReordering}
                      className="px-3 py-1 rounded text-sm bg-background-secondary disabled:opacity-30 hover:bg-background-secondary/80 transition-colors"
                      title="下移"
                    >
                      ↓ 下移
                    </button>
                  </div>

                  <div className="flex items-center gap-3">
                    <span className="text-xs text-text-secondary">
                      #{index + 1} · 查看 {image.view_count}
                    </span>
                    <button
                      onClick={() => handleDelete(image.id)}
                      className="px-3 py-1 rounded text-sm bg-accent-secondary/20 text-accent-secondary hover:bg-accent-secondary/30 transition-colors"
                    >
                      删除
                    </button>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}

        {images.length === 0 && !selectedFile && (
          <div className="card text-center text-text-secondary">
            暂无图片，点击上方按钮上传
          </div>
        )}
      </div>
    </div>
  )
}
