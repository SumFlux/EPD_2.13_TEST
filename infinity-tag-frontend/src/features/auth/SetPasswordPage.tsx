import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAuthStore } from '@/stores'
import { authApi } from '@/api/auth'

export default function SetPasswordPage() {
  const navigate = useNavigate()
  const { isAuthenticated, setAuthFromSetPassword } = useAuthStore()

  const [password, setPassword] = useState('')
  const [confirmPassword, setConfirmPassword] = useState('')
  const [error, setError] = useState('')
  const [loading, setLoading] = useState(false)
  const [deviceCode, setDeviceCode] = useState('')

  useEffect(() => {
    // 检查是否有临时token
    const tempToken = localStorage.getItem('temp_token')
    const tokenExpires = localStorage.getItem('temp_token_expires')
    const pendingDeviceCode = localStorage.getItem('pending_device_code')

    // 检查 token 是否过期
    if (tokenExpires && Date.now() > Number(tokenExpires)) {
      localStorage.removeItem('temp_token')
      localStorage.removeItem('temp_token_expires')
      localStorage.removeItem('pending_device_code')
      navigate('/setup')
      return
    }

    if (!tempToken) {
      // 没有临时token，跳回激活页
      navigate('/setup')
      return
    }

    if (pendingDeviceCode) {
      setDeviceCode(pendingDeviceCode)
    }
  }, [navigate])

  useEffect(() => {
    if (isAuthenticated) {
      navigate('/')
    }
  }, [isAuthenticated, navigate])

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setError('')

    // 验证密码
    if (password.length < 6) {
      setError('密码至少需要6位')
      return
    }

    if (password !== confirmPassword) {
      setError('两次输入的密码不一致')
      return
    }

    setLoading(true)

    try {
      const tempToken = localStorage.getItem('temp_token')
      if (!tempToken) {
        setError('会话已过期，请重新激活')
        navigate('/setup')
        return
      }

      const response = await authApi.setPassword({ new_password: password }, tempToken)

      // 清理临时数据
      localStorage.removeItem('temp_token')
      localStorage.removeItem('temp_token_expires')
      localStorage.removeItem('pending_device_code')

      // 保存认证信息
      setAuthFromSetPassword(response)

      // 跳转到主页
      navigate('/')
    } catch (err: unknown) {
      const axiosError = err as { response?: { data?: { detail?: string } } }
      setError(axiosError.response?.data?.detail || '设置密码失败，请重试')
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="min-h-screen flex items-center justify-center p-4">
      <div className="card w-full max-w-md">
        <div className="text-center mb-8">
          <h1 className="text-2xl font-bold text-gradient mb-2">设置密码</h1>
          <p className="text-text-secondary">
            设备 <span className="font-mono font-bold">{deviceCode}</span> 激活成功
          </p>
          <p className="text-text-secondary text-sm mt-1">
            请设置您的登录密码
          </p>
        </div>

        <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label htmlFor="new-password" className="block text-sm text-text-secondary mb-1">
              新密码
            </label>
            <input
              id="new-password"
              type="password"
              autoComplete="new-password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              className="input"
              placeholder="至少6位"
              required
              minLength={6}
            />
          </div>

          <div>
            <label htmlFor="confirm-password" className="block text-sm text-text-secondary mb-1">
              确认密码
            </label>
            <input
              id="confirm-password"
              type="password"
              autoComplete="new-password"
              value={confirmPassword}
              onChange={(e) => setConfirmPassword(e.target.value)}
              className="input"
              placeholder="再次输入密码"
              required
            />
          </div>

          {error && (
            <p className="text-accent-secondary text-sm text-center">{error}</p>
          )}

          <button
            type="submit"
            disabled={loading}
            className="btn-seal w-full disabled:opacity-50"
          >
            {loading ? '设置中...' : '完成设置'}
          </button>
        </form>

        <p className="text-xs text-text-secondary text-center mt-4">
          此密码用于日后登录，请妥善保管
        </p>
      </div>
    </div>
  )
}
