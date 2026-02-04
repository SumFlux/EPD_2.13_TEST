import { useState, useEffect } from 'react'
import { useNavigate, useSearchParams } from 'react-router-dom'
import { useAuthStore } from '@/stores'
import { authApi } from '@/api/auth'
import type { ActivateResponse, LoginResponse } from '@/types'

export default function SetupPage() {
  const navigate = useNavigate()
  const [searchParams] = useSearchParams()
  const { isAuthenticated, setAuthFromLogin } = useAuthStore()

  const [mode, setMode] = useState<'activate' | 'login'>('activate')
  const [deviceCode, setDeviceCode] = useState('')
  const [initPassword, setInitPassword] = useState('')
  const [userPassword, setUserPassword] = useState('')
  const [error, setError] = useState('')
  const [loading, setLoading] = useState(false)
  const [showAutoActivatePrompt, setShowAutoActivatePrompt] = useState(false)
  const [pendingActivation, setPendingActivation] = useState<{ code: string; pwd: string } | null>(null)

  // 解析URL参数，支持扫码自动填充
  useEffect(() => {
    const codeFromUrl = searchParams.get('code')
    const pwdFromUrl = searchParams.get('pwd')

    if (codeFromUrl) {
      setDeviceCode(codeFromUrl)
    }
    if (pwdFromUrl) {
      setInitPassword(pwdFromUrl)
    }

    // 如果两个参数都有，显示确认提示而非自动激活
    if (codeFromUrl && pwdFromUrl) {
      setPendingActivation({ code: codeFromUrl, pwd: pwdFromUrl })
      setShowAutoActivatePrompt(true)
    }
  }, [searchParams])

  // 如果已登录，跳转到主页
  useEffect(() => {
    if (isAuthenticated) {
      navigate('/')
    }
  }, [isAuthenticated, navigate])

  const handleActivateSuccess = (response: ActivateResponse) => {
    if (response.requires_password_setup) {
      // 存储临时token，跳转到设置密码页
      localStorage.setItem('temp_token', response.temp_token)
      localStorage.setItem('temp_token_expires', String(Date.now() + 10 * 60 * 1000)) // 10分钟过期
      localStorage.setItem('pending_device_code', response.device_code)
      navigate('/setup/password')
    }
  }

  const handleAutoActivate = async (code: string, pwd: string) => {
    setLoading(true)
    setError('')

    try {
      const response = await authApi.activate({
        device_code: code,
        init_password: pwd
      })
      handleActivateSuccess(response)
    } catch {
      // 如果激活失败，可能是已激活，切换到登录模式
      setMode('login')
      setError('该设备可能已激活，请使用密码登录')
    } finally {
      setLoading(false)
    }
  }

  const handleActivate = async (e: React.FormEvent) => {
    e.preventDefault()
    setError('')
    setLoading(true)

    try {
      const response = await authApi.activate({
        device_code: deviceCode,
        init_password: initPassword
      })
      handleActivateSuccess(response)
    } catch (err: unknown) {
      const axiosError = err as { response?: { data?: { detail?: string } } }
      setError(axiosError.response?.data?.detail || '激活失败，请检查设备码和初始密码')
    } finally {
      setLoading(false)
    }
  }

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault()
    setError('')
    setLoading(true)

    try {
      const response: LoginResponse = await authApi.login({
        device_code: deviceCode,
        password: userPassword
      })

      setAuthFromLogin(response)
      navigate('/')
    } catch (err: unknown) {
      const axiosError = err as { response?: { data?: { detail?: string } } }
      setError(axiosError.response?.data?.detail || '登录失败，请检查设备码和密码')
    } finally {
      setLoading(false)
    }
  }

  const handleConfirmAutoActivate = () => {
    if (pendingActivation) {
      setShowAutoActivatePrompt(false)
      handleAutoActivate(pendingActivation.code, pendingActivation.pwd)
    }
  }

  const handleCancelAutoActivate = () => {
    setShowAutoActivatePrompt(false)
    setPendingActivation(null)
  }

  return (
    <div className="min-h-screen flex items-center justify-center p-4">
      {/* 自动激活确认提示 */}
      {showAutoActivatePrompt && pendingActivation && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
          <div className="card w-full max-w-sm">
            <h3 className="text-lg font-bold mb-4 text-center">确认激活设备</h3>
            <p className="text-text-secondary text-sm mb-4 text-center">
              检测到设备码 <span className="font-mono font-bold">{pendingActivation.code}</span>，是否立即激活？
            </p>
            <div className="flex gap-3">
              <button
                type="button"
                onClick={handleCancelAutoActivate}
                className="flex-1 py-2 border border-gray-300 rounded hover:bg-gray-50 transition-colors"
              >
                取消
              </button>
              <button
                type="button"
                onClick={handleConfirmAutoActivate}
                className="flex-1 py-2 bg-accent-primary text-text-primary rounded hover:opacity-90 transition-colors"
              >
                确认激活
              </button>
            </div>
          </div>
        </div>
      )}

      <div className="card w-full max-w-md">
        <div className="text-center mb-8">
          <h1 className="text-2xl font-bold text-gradient mb-2">无止便签</h1>
          <p className="text-text-secondary">Infinity Tag</p>
        </div>

        {/* 模式切换 */}
        <div className="flex gap-2 mb-6">
          <button
            type="button"
            onClick={() => setMode('activate')}
            className={`flex-1 py-2 rounded transition-colors ${
              mode === 'activate'
                ? 'bg-accent-primary text-text-primary'
                : 'bg-background-secondary text-text-secondary'
            }`}
          >
            首次激活
          </button>
          <button
            type="button"
            onClick={() => setMode('login')}
            className={`flex-1 py-2 rounded transition-colors ${
              mode === 'login'
                ? 'bg-accent-primary text-text-primary'
                : 'bg-background-secondary text-text-secondary'
            }`}
          >
            登录
          </button>
        </div>

        {mode === 'activate' ? (
          <form onSubmit={handleActivate} className="space-y-4">
            <div>
              <label className="block text-sm text-text-secondary mb-1">
                设备码
              </label>
              <input
                type="text"
                value={deviceCode}
                onChange={(e) => setDeviceCode(e.target.value.toUpperCase())}
                maxLength={6}
                className="input font-mono text-lg tracking-widest text-center"
                placeholder="ABC123"
                required
              />
            </div>

            <div>
              <label className="block text-sm text-text-secondary mb-1">
                初始密码
              </label>
              <input
                type="text"
                value={initPassword}
                onChange={(e) => setInitPassword(e.target.value.toUpperCase())}
                maxLength={6}
                className="input font-mono text-lg tracking-widest text-center"
                placeholder="XYZ789"
                required
              />
              <p className="text-xs text-text-secondary mt-1">请查看设备墨水屏上显示的初始密码</p>
            </div>

            {error && (
              <p className="text-accent-secondary text-sm text-center">{error}</p>
            )}

            <button
              type="submit"
              disabled={loading}
              className="btn-seal w-full disabled:opacity-50"
            >
              {loading ? '激活中...' : '激活设备'}
            </button>
          </form>
        ) : (
          <form onSubmit={handleLogin} className="space-y-4">
            <div>
              <label className="block text-sm text-text-secondary mb-1">
                设备码
              </label>
              <input
                type="text"
                value={deviceCode}
                onChange={(e) => setDeviceCode(e.target.value.toUpperCase())}
                maxLength={6}
                className="input font-mono text-lg tracking-widest text-center"
                placeholder="ABC123"
                required
              />
            </div>

            <div>
              <label className="block text-sm text-text-secondary mb-1">
                密码
              </label>
              <input
                type="password"
                value={userPassword}
                onChange={(e) => setUserPassword(e.target.value)}
                className="input"
                placeholder="您设置的密码"
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
              {loading ? '登录中...' : '登录'}
            </button>
          </form>
        )}
      </div>
    </div>
  )
}
