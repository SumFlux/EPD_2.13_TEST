import { useState, useEffect } from 'react'
import { useNavigate } from 'react-router-dom'
import { useProfileStore } from '@/stores'
import { profileApi } from '@/api'
import type { ProfileRequest, Gender } from '@/types'
import { HOUR_OPTIONS, OCCUPATION_OPTIONS } from '@/types'

const currentYear = new Date().getFullYear()
const years = Array.from({ length: 100 }, (_, i) => currentYear - i)
const months = Array.from({ length: 12 }, (_, i) => i + 1)
const days = Array.from({ length: 31 }, (_, i) => i + 1)

export default function ProfilePage() {
  const navigate = useNavigate()
  const { profile, setProfile, isLoading, setLoading, error, setError } = useProfileStore()

  const [form, setForm] = useState<ProfileRequest>({
    nickname: '',
    gender: 1,
    birth_year: 1990,
    birth_month: 1,
    birth_day: 1,
    birth_hour: -1,
    is_lunar: false,
    occupation: '',
    notes: '',
  })

  useEffect(() => {
    if (profile) {
      setForm({
        nickname: profile.nickname,
        gender: profile.gender,
        birth_year: profile.birth_year,
        birth_month: profile.birth_month,
        birth_day: profile.birth_day,
        birth_hour: profile.birth_hour,
        is_lunar: profile.is_lunar,
        occupation: profile.occupation || '',
        notes: profile.notes || '',
      })
    }
  }, [profile])

  useEffect(() => {
    const fetchProfile = async () => {
      try {
        const data = await profileApi.get()
        setProfile(data)
      } catch {
        // 档案不存在，使用默认值
      }
    }
    fetchProfile()
  }, [setProfile])

  const handleChange = (field: keyof ProfileRequest, value: unknown) => {
    setForm((prev) => ({ ...prev, [field]: value }))
  }

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setLoading(true)
    setError(null)

    try {
      const data = await profileApi.save(form)
      setProfile(data)
      navigate('/almanac')
    } catch {
      setError('保存失败，请重试')
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="min-h-screen p-4">
      <div className="max-w-lg mx-auto">
        <h1 className="text-2xl font-bold text-center mb-6">用户档案</h1>

        <form onSubmit={handleSubmit} className="card space-y-6">
          {/* 昵称 */}
          <div>
            <label className="block text-sm text-text-secondary mb-1">昵称</label>
            <input
              type="text"
              value={form.nickname}
              onChange={(e) => handleChange('nickname', e.target.value)}
              placeholder="请输入昵称"
              className="input"
              required
            />
          </div>

          {/* 性别 */}
          <div>
            <label className="block text-sm text-text-secondary mb-1">性别</label>
            <div className="flex gap-4">
              {[
                { value: 1, label: '男' },
                { value: 0, label: '女' },
              ].map((option) => (
                <label key={option.value} className="flex items-center gap-2 cursor-pointer">
                  <input
                    type="radio"
                    name="gender"
                    checked={form.gender === option.value}
                    onChange={() => handleChange('gender', option.value as Gender)}
                    className="accent-accent-primary"
                  />
                  <span>{option.label}</span>
                </label>
              ))}
            </div>
          </div>

          {/* 出生日期 */}
          <div>
            <label className="block text-sm text-text-secondary mb-1">出生日期</label>
            <div className="flex gap-2">
              <select
                value={form.birth_year}
                onChange={(e) => handleChange('birth_year', Number(e.target.value))}
                className="input flex-1"
              >
                {years.map((y) => (
                  <option key={y} value={y}>{y}年</option>
                ))}
              </select>
              <select
                value={form.birth_month}
                onChange={(e) => handleChange('birth_month', Number(e.target.value))}
                className="input w-24"
              >
                {months.map((m) => (
                  <option key={m} value={m}>{m}月</option>
                ))}
              </select>
              <select
                value={form.birth_day}
                onChange={(e) => handleChange('birth_day', Number(e.target.value))}
                className="input w-24"
              >
                {days.map((d) => (
                  <option key={d} value={d}>{d}日</option>
                ))}
              </select>
            </div>
            <label className="flex items-center gap-2 mt-2 cursor-pointer">
              <input
                type="checkbox"
                checked={form.is_lunar}
                onChange={(e) => handleChange('is_lunar', e.target.checked)}
                className="accent-accent-primary"
              />
              <span className="text-sm text-text-secondary">农历</span>
            </label>
          </div>

          {/* 出生时辰 */}
          <div>
            <label className="block text-sm text-text-secondary mb-1">出生时辰</label>
            <select
              value={form.birth_hour}
              onChange={(e) => handleChange('birth_hour', Number(e.target.value))}
              className="input"
            >
              {HOUR_OPTIONS.map((h) => (
                <option key={h.value} value={h.value}>
                  {h.label} {h.range && `(${h.range})`}
                </option>
              ))}
            </select>
          </div>

          {/* 职业 */}
          <div>
            <label className="block text-sm text-text-secondary mb-1">职业</label>
            <select
              value={form.occupation}
              onChange={(e) => handleChange('occupation', e.target.value)}
              className="input"
            >
              <option value="">请选择职业</option>
              {OCCUPATION_OPTIONS.map((o) => (
                <option key={o} value={o}>{o}</option>
              ))}
            </select>
          </div>

          {/* 备注 */}
          <div>
            <label className="block text-sm text-text-secondary mb-1">关注点/备注</label>
            <textarea
              value={form.notes}
              onChange={(e) => handleChange('notes', e.target.value)}
              placeholder="例如：近期关注事业发展、感情问题等"
              className="input min-h-[80px] resize-none"
              rows={3}
            />
          </div>

          {error && (
            <p className="text-accent-secondary text-sm">{error}</p>
          )}

          <button
            type="submit"
            disabled={isLoading}
            className="btn-seal w-full disabled:opacity-50"
          >
            {isLoading ? '保存中...' : '保存档案'}
          </button>
        </form>
      </div>
    </div>
  )
}
