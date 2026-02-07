import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAdminStore } from '@/stores/adminStore'
import { adminApi } from '@/api/admin'
import { logger } from '@/utils/logger'
import { AdminHeader } from './AdminHeader'

export function DashboardPage() {
  const navigate = useNavigate()
  const { isAdminAuthenticated, stats, setStats } = useAdminStore()
  const [loading, setLoading] = useState(true)

  useEffect(() => {
    if (!isAdminAuthenticated) {
      navigate('/admin/login')
      return
    }

    const fetchStats = async () => {
      try {
        const data = await adminApi.getStats()
        setStats(data)
      } catch (err) {
        logger.error('获取统计数据失败', err)
      } finally {
        setLoading(false)
      }
    }

    fetchStats()
  }, [isAdminAuthenticated, navigate, setStats])

  // handleLogout moved to AdminHeader component

  if (loading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-gray-500">加载中...</div>
      </div>
    )
  }

  return (
    <div className="min-h-screen bg-gray-100">
      <AdminHeader title="管理员仪表盘" />

      <main className="max-w-7xl mx-auto px-4 py-8">
        <h2 className="text-2xl font-bold text-gray-800 mb-6">仪表盘</h2>

        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-medium text-gray-600">设备总数</h3>
            <p className="text-3xl font-bold text-gray-800 mt-2">
              {stats?.total_devices || 0}
            </p>
          </div>

          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-medium text-gray-600">待激活设备</h3>
            <p className="text-3xl font-bold text-yellow-600 mt-2">
              {stats?.pending_devices || 0}
            </p>
          </div>

          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-medium text-gray-600">已激活设备</h3>
            <p className="text-3xl font-bold text-green-600 mt-2">
              {stats?.activated_devices || 0}
            </p>
          </div>

          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-medium text-gray-600">已禁用设备</h3>
            <p className="text-3xl font-bold text-red-600 mt-2">
              {stats?.disabled_devices || 0}
            </p>
          </div>

          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-medium text-gray-600">用户总数</h3>
            <p className="text-3xl font-bold text-gray-800 mt-2">
              {stats?.total_users || 0}
            </p>
          </div>

          <div className="bg-white rounded-lg shadow p-6">
            <h3 className="text-lg font-medium text-gray-600">已设置密码</h3>
            <p className="text-3xl font-bold text-blue-600 mt-2">
              {stats?.users_with_password || 0}
            </p>
          </div>
        </div>
      </main>
    </div>
  )
}

export default DashboardPage
