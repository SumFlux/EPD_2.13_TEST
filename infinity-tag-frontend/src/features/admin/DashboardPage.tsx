import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAdminStore } from '@/stores/adminStore'
import { adminApi } from '@/api/admin'
import { logger } from '@/utils/logger'

export function DashboardPage() {
  const navigate = useNavigate()
  const { isAdminAuthenticated, stats, setStats, adminLogout } = useAdminStore()
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

  const handleLogout = () => {
    adminLogout()
    navigate('/admin/login')
  }

  if (loading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-gray-500">加载中...</div>
      </div>
    )
  }

  return (
    <div className="min-h-screen bg-gray-100">
      <nav className="bg-white shadow-sm">
        <div className="max-w-7xl mx-auto px-4 py-3 flex justify-between items-center">
          <h1 className="text-xl font-bold text-gray-800">Infinity Tag 管理后台</h1>
          <div className="flex items-center space-x-4">
            <button
              onClick={() => navigate('/admin/devices')}
              className="text-gray-600 hover:text-gray-800"
            >
              设备管理
            </button>
            <button
              onClick={() => navigate('/admin/users')}
              className="text-gray-600 hover:text-gray-800"
            >
              用户管理
            </button>
            <button
              onClick={handleLogout}
              className="text-red-600 hover:text-red-800"
            >
              退出
            </button>
          </div>
        </div>
      </nav>

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
