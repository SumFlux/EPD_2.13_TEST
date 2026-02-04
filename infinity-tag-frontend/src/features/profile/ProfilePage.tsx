import { useState, useEffect } from 'react'
import { useProfileStore, useAlmanacStore } from '@/stores'
import { profileApi, almanacApi } from '@/api'
import type { ProfileRequest, Gender, Almanac } from '@/types'
import { HOUR_OPTIONS, OCCUPATION_OPTIONS } from '@/types'
import { getEnergyColor, getEnergyLabel } from '@/utils'

const currentYear = new Date().getFullYear()
const years = Array.from({ length: 100 }, (_, i) => currentYear - i)
const months = Array.from({ length: 12 }, (_, i) => i + 1)
const days = Array.from({ length: 31 }, (_, i) => i + 1)

export default function ProfilePage() {
  const { profile, setProfile, isLoading, setLoading, error, setError } = useProfileStore()
  const { history, setHistory } = useAlmanacStore()
  const [selectedAlmanac, setSelectedAlmanac] = useState<Almanac | null>(null)
  const [isEditing, setIsEditing] = useState(false)

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
    const fetchData = async () => {
      try {
        const profileData = await profileApi.get()
        setProfile(profileData)
      } catch {
        // 档案不存在，使用默认值
        setIsEditing(true)
      }

      // 获取历史记录
      try {
        const historyData = await almanacApi.history(30)
        setHistory(historyData)
      } catch {
        // 历史记录获取失败，忽略
      }
    }
    fetchData()
  }, [setProfile, setHistory])

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
      setIsEditing(false)
    } catch {
      setError('保存失败，请重试')
    } finally {
      setLoading(false)
    }
  }

  // 格式化日期显示
  const formatDisplayDate = (dateStr: string) => {
    const date = new Date(dateStr)
    const month = date.getMonth() + 1
    const day = date.getDate()
    const weekdays = ['周日', '周一', '周二', '周三', '周四', '周五', '周六']
    const weekday = weekdays[date.getDay()]
    return { month, day, weekday }
  }

  // 筛选有数据的历史记录
  const validHistory = history.filter(h => h && h.date)

  return (
    <div className="h-full overflow-y-auto">
      <div className="max-w-lg mx-auto p-4 space-y-6">
        {/* 档案卡片 */}
        <div className="card">
          <div className="flex items-center justify-between mb-4">
            <h2 className="text-lg font-bold">个人档案</h2>
            {profile && !isEditing && (
              <button
                onClick={() => setIsEditing(true)}
                className="text-sm text-accent-primary hover:opacity-80"
              >
                编辑
              </button>
            )}
          </div>

          {isEditing ? (
            <form onSubmit={handleSubmit} className="space-y-4">
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
                    className="input w-20"
                  >
                    {months.map((m) => (
                      <option key={m} value={m}>{m}月</option>
                    ))}
                  </select>
                  <select
                    value={form.birth_day}
                    onChange={(e) => handleChange('birth_day', Number(e.target.value))}
                    className="input w-20"
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
                  className="input min-h-[60px] resize-none"
                  rows={2}
                />
              </div>

              {error && (
                <p className="text-accent-secondary text-sm">{error}</p>
              )}

              <div className="flex gap-2">
                {profile && (
                  <button
                    type="button"
                    onClick={() => setIsEditing(false)}
                    className="flex-1 py-2 bg-background-secondary text-text-secondary rounded"
                  >
                    取消
                  </button>
                )}
                <button
                  type="submit"
                  disabled={isLoading}
                  className="flex-1 btn-seal disabled:opacity-50"
                >
                  {isLoading ? '保存中...' : '保存'}
                </button>
              </div>
            </form>
          ) : profile ? (
            <div className="space-y-2 text-sm">
              <div className="flex justify-between">
                <span className="text-text-secondary">昵称</span>
                <span>{profile.nickname}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">性别</span>
                <span>{profile.gender === 1 ? '男' : '女'}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-text-secondary">出生日期</span>
                <span>
                  {profile.birth_year}年{profile.birth_month}月{profile.birth_day}日
                  {profile.is_lunar ? ' (农历)' : ''}
                </span>
              </div>
              {profile.occupation && (
                <div className="flex justify-between">
                  <span className="text-text-secondary">职业</span>
                  <span>{profile.occupation}</span>
                </div>
              )}
            </div>
          ) : null}
        </div>

        {/* 历史回顾 - 仅当有数据时显示 */}
        {validHistory.length > 0 && (
          <div>
            <h2 className="text-lg font-bold mb-3">历史回顾</h2>
            <div className="space-y-3">
              {validHistory.map((almanac) => {
                const { month, day, weekday } = formatDisplayDate(almanac.date)
                return (
                  <button
                    key={almanac.date}
                    onClick={() => setSelectedAlmanac(almanac)}
                    className="w-full card hover:bg-background-secondary/80 transition-colors text-left"
                  >
                    <div className="flex items-center gap-4">
                      {/* 日期 */}
                      <div className="text-center min-w-[50px]">
                        <div className="text-2xl font-bold">{day}</div>
                        <div className="text-xs text-text-secondary">{month}月 {weekday}</div>
                      </div>

                      {/* 分隔线 */}
                      <div className="w-px h-12 bg-border/30" />

                      {/* 能量和农历 */}
                      <div className="flex-1">
                        <div className="text-sm mb-1">{almanac.lunar_date}</div>
                        <div className="flex items-center gap-2">
                          <div
                            className={`w-3 h-3 rounded-full ${getEnergyColor(almanac.energy_level)}`}
                          />
                          <span className="text-sm text-text-secondary">
                            {getEnergyLabel(almanac.energy_level)}
                          </span>
                        </div>
                      </div>

                      {/* 箭头 */}
                      <div className="text-text-secondary">›</div>
                    </div>
                  </button>
                )
              })}
            </div>
          </div>
        )}
      </div>

      {/* 详情弹窗 */}
      {selectedAlmanac && (
        <div
          className="fixed inset-0 bg-black/60 flex items-center justify-center p-4 z-50"
          onClick={() => setSelectedAlmanac(null)}
        >
          <div
            className="card w-full max-w-md max-h-[80vh] overflow-y-auto animate-fade-in"
            onClick={(e) => e.stopPropagation()}
          >
            {/* 弹窗头部 */}
            <div className="flex items-center justify-between mb-4">
              <h3 className="text-lg font-bold">{selectedAlmanac.date}</h3>
              <button
                onClick={() => setSelectedAlmanac(null)}
                className="text-text-secondary hover:text-text-primary text-xl"
              >
                ×
              </button>
            </div>

            <div className="space-y-4">
              {/* 日期信息 */}
              <div className="text-center">
                <div className="text-2xl font-bold mb-1">{selectedAlmanac.lunar_date}</div>
                <div className="text-sm text-text-secondary">
                  {selectedAlmanac.ganzhi_year}年 {selectedAlmanac.ganzhi_month}月 {selectedAlmanac.ganzhi_day}日
                </div>
              </div>

              {/* 能量条 */}
              <div>
                <div className="flex justify-between text-sm mb-2">
                  <span>当日能量</span>
                  <span className="text-accent-primary">{getEnergyLabel(selectedAlmanac.energy_level)}</span>
                </div>
                <div className="energy-bar">
                  <div
                    className={`energy-fill ${getEnergyColor(selectedAlmanac.energy_level)}`}
                    style={{ width: `${selectedAlmanac.energy_level}%` }}
                  />
                </div>
              </div>

              {/* 宜忌 */}
              <div className="grid grid-cols-2 gap-3">
                <div className="bg-background-secondary rounded p-3">
                  <div className="text-energy-high font-bold mb-1 text-sm">宜</div>
                  <div className="text-xs">{selectedAlmanac.favorable}</div>
                </div>
                <div className="bg-background-secondary rounded p-3">
                  <div className="text-energy-low font-bold mb-1 text-sm">忌</div>
                  <div className="text-xs">{selectedAlmanac.unfavorable}</div>
                </div>
              </div>

              {/* 吉方吉物 */}
              <div className="flex gap-3 text-sm">
                <div className="flex-1 bg-background-secondary rounded p-2 text-center">
                  <div className="text-text-secondary text-xs mb-1">吉方</div>
                  <div className="text-accent-primary text-sm">{selectedAlmanac.lucky_direction}</div>
                </div>
                <div className="flex-1 bg-background-secondary rounded p-2 text-center">
                  <div className="text-text-secondary text-xs mb-1">吉物</div>
                  <div className="text-accent-primary text-sm">{selectedAlmanac.lucky_item}</div>
                </div>
              </div>

              {/* 运势解读 */}
              <div>
                <div className="text-sm text-text-secondary mb-1">运势解读</div>
                <div className="text-sm leading-relaxed">{selectedAlmanac.commentary}</div>
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  )
}
