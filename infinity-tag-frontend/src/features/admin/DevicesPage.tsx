import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { QRCodeSVG } from 'qrcode.react'
import { useAdminStore } from '@/stores/adminStore'
import { adminApi } from '@/api/admin'
import { logger } from '@/utils/logger'
import type { Device, DeviceCreateResponse } from '@/types/admin'
import { AdminHeader } from './AdminHeader'

export function DevicesPage() {
  const navigate = useNavigate()
  const { isAdminAuthenticated, devices, totalDevices, setDevices } = useAdminStore()
  const [loading, setLoading] = useState(true)
  const [page, setPage] = useState(1)
  const [statusFilter, setStatusFilter] = useState<string>('')

  // Create Modal
  const [showCreateModal, setShowCreateModal] = useState(false)
  const [newUuid, setNewUuid] = useState('')
  const [batchName, setBatchName] = useState('')
  const [createResult, setCreateResult] = useState<DeviceCreateResponse | null>(null)
  const [createError, setCreateError] = useState('')

  // QR Modal (List View)
  const [showQrModal, setShowQrModal] = useState(false)
  const [selectedDeviceQr, setSelectedDeviceQr] = useState<string>('')
  const [selectedDeviceCode, setSelectedDeviceCode] = useState<string>('')

  useEffect(() => {
    if (!isAdminAuthenticated) {
      navigate('/admin/login')
      return
    }

    fetchDevices()
  }, [isAdminAuthenticated, navigate, page, statusFilter])

  const fetchDevices = async () => {
    setLoading(true)
    try {
      const data = await adminApi.getDevices({
        page,
        page_size: 20,
        status: statusFilter || undefined
      })
      setDevices(data.devices, data.total)
    } catch (err) {
      logger.error('获取设备列表失败', err)
    } finally {
      setLoading(false)
    }
  }

  const handleCreateDevice = async () => {
    if (!newUuid.trim()) return
    setCreateError('')
    setCreateResult(null)

    try {
      const result = await adminApi.createDevice({
        uuid: newUuid.trim(),
        batch_name: batchName.trim() || undefined
      })
      setCreateResult(result)
      fetchDevices()
    } catch (err: unknown) {
      const errorMessage = err instanceof Error ? err.message : '创建失败'
      setCreateError(errorMessage)
    }
  }

  const handleDisableDevice = async (device: Device) => {
    if (!confirm(`确定要禁用设备 ${device.device_code} 吗？`)) return
    try {
      await adminApi.disableDevice(device.id)
      fetchDevices()
    } catch (err) {
      logger.error('禁用设备失败', err)
    }
  }

  const handleResetDevice = async (device: Device) => {
    if (!confirm(`确定要重置设备 ${device.device_code} 吗？这将解除激活并恢复初始密码。`)) return
    try {
      await adminApi.resetDevice(device.id)
      fetchDevices()
    } catch (err) {
      logger.error('重置设备失败', err)
    }
  }

  const handleDeleteDevice = async (device: Device) => {
    if (!confirm(`确定要删除设备 ${device.device_code} 吗？此操作不可恢复。`)) return
    try {
      await adminApi.deleteDevice(device.id)
      fetchDevices()
    } catch (err) {
      logger.error('删除设备失败', err)
    }
  }

  const handleShowQr = (device: Device) => {
    const url = `https://talisman.app/setup?code=${device.device_code}`
    setSelectedDeviceQr(url)
    setSelectedDeviceCode(device.device_code)
    setShowQrModal(true)
  }

  const getStatusBadge = (status: string) => {
    const styles: Record<string, string> = {
      pending: 'bg-yellow-100 text-yellow-800',
      activated: 'bg-green-100 text-green-800',
      disabled: 'bg-red-100 text-red-800'
    }
    const labels: Record<string, string> = {
      pending: '待激活',
      activated: '已激活',
      disabled: '已禁用'
    }
    return (
      <span className={`px-2 py-1 rounded-full text-xs font-medium ${styles[status] || ''}`}>
        {labels[status] || status}
      </span>
    )
  }

  return (
    <div className="min-h-screen bg-gray-100">
      <AdminHeader title="设备管理" />

      <main className="max-w-7xl mx-auto px-4 py-8">
        <div className="flex justify-between items-center mb-6">
          <div className="flex items-center space-x-4">
            <select
              value={statusFilter}
              onChange={(e) => { setStatusFilter(e.target.value); setPage(1) }}
              className="px-3 py-2 border border-gray-300 rounded-md"
            >
              <option value="">全部状态</option>
              <option value="pending">待激活</option>
              <option value="activated">已激活</option>
              <option value="disabled">已禁用</option>
            </select>
          </div>
          <button
            onClick={() => setShowCreateModal(true)}
            className="px-4 py-2 bg-blue-600 text-white rounded-md hover:bg-blue-700"
          >
            录入设备
          </button>
        </div>

        {loading ? (
          <div className="text-center py-8 text-gray-500">加载中...</div>
        ) : (
          <>
            <div className="bg-white rounded-lg shadow overflow-hidden">
              <table className="min-w-full divide-y divide-gray-200">
                <thead className="bg-gray-50">
                  <tr>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">设备码</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">UUID</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">状态</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">批次</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">创建时间</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">操作</th>
                  </tr>
                </thead>
                <tbody className="bg-white divide-y divide-gray-200">
                  {devices.map((device) => (
                    <tr key={device.id}>
                      <td className="px-6 py-4 whitespace-nowrap font-mono font-bold">{device.device_code}</td>
                      <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-500 font-mono">{device.uuid}</td>
                      <td className="px-6 py-4 whitespace-nowrap">{getStatusBadge(device.status)}</td>
                      <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-500">{device.batch_name || '-'}</td>
                      <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-500">
                        {new Date(device.created_at).toLocaleString()}
                      </td>
                      <td className="px-6 py-4 whitespace-nowrap text-sm space-x-2">
                        {device.status === 'pending' && (
                          <button onClick={() => handleShowQr(device)} className="text-blue-600 hover:text-blue-800">
                            二维码
                          </button>
                        )}
                        {device.status === 'activated' && (
                          <button onClick={() => handleResetDevice(device)} className="text-yellow-600 hover:text-yellow-800">
                            重置
                          </button>
                        )}
                        {device.status !== 'disabled' && (
                          <button onClick={() => handleDisableDevice(device)} className="text-red-600 hover:text-red-800">
                            禁用
                          </button>
                        )}
                        <button onClick={() => handleDeleteDevice(device)} className="text-gray-600 hover:text-gray-800">
                          删除
                        </button>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>

            <div className="mt-4 flex justify-between items-center">
              <div className="text-sm text-gray-500">共 {totalDevices} 条记录</div>
              <div className="flex space-x-2">
                <button
                  onClick={() => setPage(p => Math.max(1, p - 1))}
                  disabled={page === 1}
                  className="px-3 py-1 border rounded disabled:opacity-50"
                >
                  上一页
                </button>
                <span className="px-3 py-1">第 {page} 页</span>
                <button
                  onClick={() => setPage(p => p + 1)}
                  disabled={devices.length < 20}
                  className="px-3 py-1 border rounded disabled:opacity-50"
                >
                  下一页
                </button>
              </div>
            </div>
          </>
        )}

        {/* Create Modal */}
        {showCreateModal && (
          <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <div className="bg-white rounded-lg p-6 w-full max-w-md">
              <h3 className="text-lg font-bold mb-4">录入设备</h3>

              <div className="space-y-4">
                <div>
                  <label className="block text-sm font-medium text-gray-700 mb-1">ESP32 UUID</label>
                  <input
                    type="text"
                    value={newUuid}
                    onChange={(e) => setNewUuid(e.target.value)}
                    className="w-full px-3 py-2 border border-gray-300 rounded-md"
                    placeholder="输入芯片UUID"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 mb-1">批次名称（可选）</label>
                  <input
                    type="text"
                    value={batchName}
                    onChange={(e) => setBatchName(e.target.value)}
                    className="w-full px-3 py-2 border border-gray-300 rounded-md"
                    placeholder="如：2026-02批次"
                  />
                </div>

                {createError && (
                  <div className="text-red-500 text-sm">{createError}</div>
                )}

                {createResult && (
                  <div className="bg-green-50 border border-green-200 rounded-md p-4">
                    <p className="text-green-800 font-medium mb-2">创建成功！</p>
                    <p className="text-sm"><span className="font-medium">设备码：</span>{createResult.device_code}</p>
                    <p className="text-sm"><span className="font-medium">初始密码：</span>{createResult.init_password}</p>
                    <div className="mt-4 flex flex-col items-center p-2 bg-white rounded border">
                      <QRCodeSVG
                        value={`https://talisman.app/setup?code=${createResult.device_code}&pwd=${createResult.init_password}`}
                        size={160}
                      />
                      <p className="text-xs text-gray-500 mt-2">扫码一键激活 (含密码)</p>
                    </div>
                  </div>
                )}
              </div>

              <div className="mt-6 flex justify-end space-x-3">
                <button
                  onClick={() => {
                    setShowCreateModal(false)
                    setNewUuid('')
                    setBatchName('')
                    setCreateResult(null)
                    setCreateError('')
                  }}
                  className="px-4 py-2 border border-gray-300 rounded-md hover:bg-gray-50"
                >
                  关闭
                </button>
                {!createResult && (
                  <button
                    onClick={handleCreateDevice}
                    className="px-4 py-2 bg-blue-600 text-white rounded-md hover:bg-blue-700"
                  >
                    创建
                  </button>
                )}
              </div>
            </div>
          </div>
        )}

        {/* QR Code Modal (Simple) */}
        {showQrModal && (
          <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <div className="bg-white rounded-lg p-6 w-full max-w-sm text-center">
              <h3 className="text-lg font-bold mb-4">设备二维码</h3>
              <p className="text-sm text-gray-600 mb-4 font-mono font-bold">{selectedDeviceCode}</p>

              <div className="flex justify-center p-4 bg-white border rounded mb-4">
                <QRCodeSVG value={selectedDeviceQr} size={200} />
              </div>

              <div className="bg-yellow-50 p-3 rounded text-xs text-yellow-800 text-left mb-6">
                注意：此二维码仅包含连接信息。用户扫码后，仍需手动输入打印在设备/贴纸上的初始密码。
              </div>

              <button
                onClick={() => setShowQrModal(false)}
                className="w-full px-4 py-2 bg-gray-100 text-gray-800 rounded-md hover:bg-gray-200"
              >
                关闭
              </button>
            </div>
          </div>
        )}

      </main>
    </div>
  )
}

export default DevicesPage
