import { useState } from 'react'
import { adminApi } from '@/api/admin'
import { logger } from '@/utils/logger'
import { Loader2, UploadCloud, AlertCircle, CheckCircle2 } from 'lucide-react'
import type { FirmwareResponse } from '@/types/admin'
import { AdminHeader } from './AdminHeader'

export function FirmwarePage() {
    // Navigate and adminLogout handled by AdminHeader

    const [file, setFile] = useState<File | null>(null)
    const [version, setVersion] = useState('')
    const [description, setDescription] = useState('')
    const [loading, setLoading] = useState(false)
    const [error, setError] = useState('')
    const [successResult, setSuccessResult] = useState<FirmwareResponse | null>(null)

    // 简单的版本号校验: A.B.C.D
    const validateVersion = (v: string) => {
        return /^(\d{1,2})\.(\d{1,2})\.(\d{1,2})\.(\d{1,2})$/.test(v)
    }

    const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
        if (e.target.files && e.target.files[0]) {
            setFile(e.target.files[0])
            setError('')
        }
    }

    const handleUpload = async (e: React.FormEvent) => {
        e.preventDefault()
        if (!file) {
            setError('请选择固件文件 (.bin)')
            return
        }
        if (!version || !validateVersion(version)) {
            setError('版本号格式错误，必须为 A.B.C.D (如 1.0.0.1)')
            return
        }

        setLoading(true)
        setError('')
        setSuccessResult(null)

        try {
            const formData = new FormData()
            formData.append('file', file)
            formData.append('version', version)
            if (description) {
                formData.append('description', description)
            }

            const result = await adminApi.uploadFirmware(formData)
            setSuccessResult(result)
            // 清空表单
            setFile(null)
            setVersion('')
            setDescription('')
        } catch (err: any) {
            const msg = err.response?.data?.detail || err.message || '上传失败'
            setError(msg)
            logger.error('上传固件失败', err)
        } finally {
            setLoading(false)
        }
    }

    return (
        <div className="min-h-screen bg-gray-100">
            <AdminHeader title="固件管理 (OTA)" />

            <main className="max-w-3xl mx-auto px-4 py-8">
                <div className="bg-white rounded-lg shadow p-6">
                    <div className="mb-6 pb-6 border-b border-gray-100">
                        <h2 className="text-lg font-semibold text-gray-800 flex items-center gap-2">
                            <UploadCloud className="w-5 h-5 text-blue-600" />
                            上传新固件
                        </h2>
                        <p className="text-sm text-gray-500 mt-1">
                            请上传编译好的 .bin 文件。版本号必须严格递增，否则设备将拒绝更新。
                        </p>
                    </div>

                    <form onSubmit={handleUpload} className="space-y-6">
                        {/* 错误提示 */}
                        {error && (
                            <div className="bg-red-50 text-red-700 p-4 rounded-md flex items-start gap-2 text-sm">
                                <AlertCircle className="w-5 h-5 flex-shrink-0" />
                                <span>{error}</span>
                            </div>
                        )}

                        {/* 成功提示 */}
                        {successResult && (
                            <div className="bg-green-50 text-green-800 p-4 rounded-md space-y-2">
                                <div className="flex items-center gap-2 font-medium">
                                    <CheckCircle2 className="w-5 h-5" />
                                    上传成功！
                                </div>
                                <div className="text-sm pl-7 space-y-1 text-green-700">
                                    <p>版本: <span className="font-mono">{successResult.version_str}</span> (Code: {successResult.version_code})</p>
                                    <p>校验和: <span className="font-mono text-xs">{successResult.checksum}</span></p>
                                    <p>路径: <span className="font-mono text-xs">{successResult.file_path}</span></p>
                                </div>
                            </div>
                        )}

                        {/* 版本号 */}
                        <div>
                            <label className="block text-sm font-medium text-gray-700 mb-1">
                                版本号 <span className="text-red-500">*</span>
                            </label>
                            <input
                                type="text"
                                value={version}
                                onChange={(e) => setVersion(e.target.value)}
                                placeholder="例如: 1.0.0.1"
                                className="w-full px-3 py-2 border border-gray-300 rounded-md focus:ring-blue-500 focus:border-blue-500"
                                disabled={loading}
                            />
                            <p className="text-xs text-gray-400 mt-1">格式要求: A.B.C.D (4位数字, 0-99)</p>
                        </div>

                        {/* 说明 */}
                        <div>
                            <label className="block text-sm font-medium text-gray-700 mb-1">
                                更新说明 (可选)
                            </label>
                            <textarea
                                value={description}
                                onChange={(e) => setDescription(e.target.value)}
                                rows={3}
                                className="w-full px-3 py-2 border border-gray-300 rounded-md focus:ring-blue-500 focus:border-blue-500"
                                placeholder="本次更新的内容..."
                                disabled={loading}
                            />
                        </div>

                        {/* 文件选择 */}
                        <div>
                            <label className="block text-sm font-medium text-gray-700 mb-1">
                                固件文件 (.bin) <span className="text-red-500">*</span>
                            </label>
                            <div className="mt-1 flex justify-center px-6 pt-5 pb-6 border-2 border-gray-300 border-dashed rounded-md hover:bg-gray-50 transition-colors">
                                <div className="space-y-1 text-center">
                                    <UploadCloud className="mx-auto h-12 w-12 text-gray-400" />
                                    <div className="flex text-sm text-gray-600">
                                        <label
                                            htmlFor="file-upload"
                                            className="relative cursor-pointer bg-white rounded-md font-medium text-blue-600 hover:text-blue-500 focus-within:outline-none focus-within:ring-2 focus-within:ring-offset-2 focus-within:ring-blue-500"
                                        >
                                            <span>选择文件</span>
                                            <input
                                                id="file-upload"
                                                name="file-upload"
                                                type="file"
                                                accept=".bin"
                                                className="sr-only"
                                                onChange={handleFileChange}
                                                disabled={loading}
                                            />
                                        </label>
                                        <p className="pl-1">或拖拽文件到这里</p>
                                    </div>
                                    <p className="text-xs text-gray-500">
                                        BIN up to 10MB
                                    </p>
                                    {file && (
                                        <p className="text-sm font-medium text-blue-600 mt-2">
                                            已选择: {file.name} ({(file.size / 1024).toFixed(1)} KB)
                                        </p>
                                    )}
                                </div>
                            </div>
                        </div>

                        {/* 提交按钮 */}
                        <div className="flex justify-end pt-4">
                            <button
                                type="submit"
                                disabled={loading}
                                className="w-full flex justify-center py-2 px-4 border border-transparent rounded-md shadow-sm text-sm font-medium text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:opacity-50 disabled:cursor-not-allowed"
                            >
                                {loading ? (
                                    <>
                                        <Loader2 className="animate-spin -ml-1 mr-2 h-4 w-4" />
                                        上传中...
                                    </>
                                ) : (
                                    '开始上传'
                                )}
                            </button>
                        </div>

                    </form>
                </div>
            </main>
        </div>
    )
}
