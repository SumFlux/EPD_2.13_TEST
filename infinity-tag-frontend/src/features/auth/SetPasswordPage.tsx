import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAuthStore } from '@/stores'
import apiClient from '@/api/client'
import type { SetPasswordResponse } from '@/types/admin'

export default function SetPasswordPage() {
  const navigate = useNavigate()
  const { setAuth } = useAuthStore()

  const [password, setPassword] = useState('')
  const [confirmPassword, setConfirmPassword] = useState('')
  const [error, setError] = useState('')
  const [loading, setLoading] = useState(false)
  const [deviceCode, setDeviceCode] = useState('')

  useEffect(() => {
    // 检查是否有临时token
    const tempToken = localStorage.getItem('temp_token')
    const pendingDeviceCode = localStorage.getItem('pending_device_code')

    if (!tempToken || !pendingDeviceCode) {
      navigate('/setup')
      return
    }

    setDeviceCode(pendingDeviceCode)
  }, [navigate])

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setError('')

    if (password !== confirmPassword) {
      setError('两次输入的密码不一致')
      return
    }

    if (password.length < 6) {
      setError('密码长度至少为6位')
      return
    }

    setLoading(true)

    try {
      const tempToken = localStorage.getItem('temp_token')

      const response = await apiClient.post<SetPasswordResponse>(
        '/auth/set-password',
        { new_password: password },
        {
          headers: {
            Authorization: `Bearer ${tempToken}`
          }
        }
      )

      // 清除临时数据
      localStorage.removeItem('temp_token')
      localStorage.removeItem('pending_device_code')

      // 设置认证状态
      setAuth({
        access_token: response.data.access_token,
        token_type: 'bearer',
        device_id: response.data.device_id,
        user_id: 0
      })

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
          <div className="w-16 h-16 bg-accent-primary/20 rounded-full flex items-center justify-center mx-auto mb-4">
            <svg className="w-8 h-8 text-accent-primary" fill="none" stroke="currentColor" viewBox="0 0 24 24">
              <path strokeLinecap="round" strokeLinejoin="round" strokeWidth={2} d="M5 13l4 4L19 7" />
            </svg>
          </div>
          <h1 className="text-2xl font-bold text-gradient mb-2">激活成功！</h1>
          <p className="text-text-secondary">请设置您的登录密码</p>
        </div>

        <div className="bg-background-secondary rounded-lg p-4 mb-6">
          <p className="text-sm text-text-secondary">
            设备码：<span className="font-mono font-bold text-text-primary">{deviceCode}</span>
          </p>
        </div>

        <form onSubmit={handleSubmit} className="space-y-4">
          <div>
            <label className="block text-sm text-text-secondary mb-1">
              设置密码
            </label>
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              className="input"
              placeholder="至少6位字符"
              minLength={6}
              required
            />
          </div>

          <div>
            <label className="block text-sm text-text-secondary mb-1">
              确认密码
            </label>
            <input
              type="password"
              value={confirmPassword}
              onChange={(e) => setConfirmPassword(e.target.value)}
              className="input"
              placeholder="再次输入密码"
              minLength={6}
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

        <p className="text-xs text-text-secondary text-center mt-6">
          设置完成后，使用设备码和此密码登录
        </p>
      </div>
    </div>
  )
}
