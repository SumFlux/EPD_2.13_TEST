import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAdminStore } from '@/stores/adminStore'
import { adminApi } from '@/api/admin'
import { logger } from '@/utils/logger'
import type { AdminUser } from '@/types/admin'

export function UsersPage() {
  const navigate = useNavigate()
  const { isAdminAuthenticated, users, totalUsers, setUsers, adminLogout } = useAdminStore()
  const [loading, setLoading] = useState(true)
  const [page, setPage] = useState(1)

  useEffect(() => {
    if (!isAdminAuthenticated) {
      navigate('/admin/login')
      return
    }

    fetchUsers()
  }, [isAdminAuthenticated, navigate, page])

  const fetchUsers = async () => {
    setLoading(true)
    try {
      const data = await adminApi.getUsers({ page, page_size: 20 })
      setUsers(data.users, data.total)
    } catch (err) {
      logger.error('获取用户列表失败', err)
    } finally {
      setLoading(false)
    }
  }

  const handleDisableUser = async (user: AdminUser) => {
    if (!confirm(`确定要禁用用户 ${user.device_id} 吗？`)) return
    try {
      await adminApi.disableUser(user.id)
      fetchUsers()
    } catch (err) {
      logger.error('禁用用户失败', err)
    }
  }

  return (
    <div className="min-h-screen bg-gray-100">
      <nav className="bg-white shadow-sm">
        <div className="max-w-7xl mx-auto px-4 py-3 flex justify-between items-center">
          <h1 className="text-xl font-bold text-gray-800">用户管理</h1>
          <div className="flex items-center space-x-4">
            <button onClick={() => navigate('/admin/dashboard')} className="text-gray-600 hover:text-gray-800">
              仪表盘
            </button>
            <button onClick={() => navigate('/admin/devices')} className="text-gray-600 hover:text-gray-800">
              设备管理
            </button>
            <button onClick={() => { adminLogout(); navigate('/admin/login') }} className="text-red-600 hover:text-red-800">
              退出
            </button>
          </div>
        </div>
      </nav>

      <main className="max-w-7xl mx-auto px-4 py-8">
        {loading ? (
          <div className="text-center py-8 text-gray-500">加载中...</div>
        ) : (
          <>
            <div className="bg-white rounded-lg shadow overflow-hidden">
              <table className="min-w-full divide-y divide-gray-200">
                <thead className="bg-gray-50">
                  <tr>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">ID</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">设备码</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">密码状态</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">激活时间</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">最后登录</th>
                    <th className="px-6 py-3 text-left text-xs font-medium text-gray-500 uppercase">操作</th>
                  </tr>
                </thead>
                <tbody className="bg-white divide-y divide-gray-200">
                  {users.map((user) => (
                    <tr key={user.id}>
                      <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-500">{user.id}</td>
                      <td className="px-6 py-4 whitespace-nowrap font-mono font-bold">{user.device_id}</td>
                      <td className="px-6 py-4 whitespace-nowrap">
                        <span className={`px-2 py-1 rounded-full text-xs font-medium ${
                          user.password_set ? 'bg-green-100 text-green-800' : 'bg-yellow-100 text-yellow-800'
                        }`}>
                          {user.password_set ? '已设置' : '未设置'}
                        </span>
                      </td>
                      <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-500">
                        {user.activated_at ? new Date(user.activated_at).toLocaleString() : '-'}
                      </td>
                      <td className="px-6 py-4 whitespace-nowrap text-sm text-gray-500">
                        {user.last_login_at ? new Date(user.last_login_at).toLocaleString() : '-'}
                      </td>
                      <td className="px-6 py-4 whitespace-nowrap text-sm">
                        <button
                          onClick={() => handleDisableUser(user)}
                          className="text-red-600 hover:text-red-800"
                        >
                          禁用
                        </button>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>

            <div className="mt-4 flex justify-between items-center">
              <div className="text-sm text-gray-500">共 {totalUsers} 条记录</div>
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
                  disabled={users.length < 20}
                  className="px-3 py-1 border rounded disabled:opacity-50"
                >
                  下一页
                </button>
              </div>
            </div>
          </>
        )}
      </main>
    </div>
  )
}

export default UsersPage
