import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAlmanacStore, useProfileStore } from '@/stores'
import { almanacApi, profileApi } from '@/api'
import { getEnergyColor, getEnergyLabel } from '@/utils'
import type { Almanac, ProfileRequest, Gender } from '@/types'
import { HOUR_OPTIONS, OCCUPATION_OPTIONS } from '@/types'

const currentYear = new Date().getFullYear()
const years = Array.from({ length: 100 }, (_, i) => currentYear - i)
const months = Array.from({ length: 12 }, (_, i) => i + 1)
const days = Array.from({ length: 31 }, (_, i) => i + 1)

export default function AlmanacPage() {
  const navigate = useNavigate()
  const { today, history, setToday, setHistory, isLoading, setLoading, error, setError } = useAlmanacStore()
  const { profile, setProfile } = useProfileStore()
  const [hasProfile, setHasProfile] = useState<boolean | null>(null)
  const [selectedAlmanac, setSelectedAlmanac] = useState<Almanac | null>(null)
  const [showProfileForm, setShowProfileForm] = useState(false)
  const [profileLoading, setProfileLoading] = useState(false)
  const [profileError, setProfileError] = useState<string | null>(null)

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
      setLoading(true)
      setError(null)
      try {
        // 先检查是否有档案
        const profileData = await profileApi.getProfile()
        if (!profileData || !profileData.birth_year) {
          setHasProfile(false)
          setShowProfileForm(true)
          setLoading(false)
          return
        }
        setHasProfile(true)
        setProfile(profileData)

        // 有档案才获取黄历
        const [todayData, historyData] = await Promise.all([
          almanacApi.generate(),
          almanacApi.history(30),
        ])
        setToday(todayData)
        setHistory(historyData)
      } catch (err: unknown) {
        const axiosError = err as { response?: { status?: number; data?: { detail?: string } } }
        if (axiosError.response?.status === 400 &&
            axiosError.response?.data?.detail?.includes('档案')) {
          setHasProfile(false)
          setShowProfileForm(true)
        } else {
          setError('加载失败，请重试')
        }
      } finally {
        setLoading(false)
      }
    }
    fetchData()
  }, [setToday, setHistory, setLoading, setError, setProfile])

  const handleChange = (field: keyof ProfileRequest, value: unknown) => {
    setForm((prev) => ({ ...prev, [field]: value }))
  }

  const handleProfileSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setProfileLoading(true)
    setProfileError(null)

    try {
      const data = await profileApi.save(form)
      setProfile(data)
      setHasProfile(true)
      setShowProfileForm(false)

      // 保存后获取运势
      const [todayData, historyData] = await Promise.all([
        almanacApi.generate(),
        almanacApi.history(30),
      ])
      setToday(todayData)
      setHistory(historyData)
    } catch {
      setProfileError('保存失败，请重试')
    } finally {
      setProfileLoading(false)
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

  // 筛选有数据的历史记录（排除今天，因为今天已在上方完整显示）
  const todayStr = new Date().toISOString().split('T')[0]
  const validHistory = history.filter(h => h && h.date && h.date !== todayStr)

  if (isLoading) {
    return (
      <div className="h-full flex items-center justify-center">
        <div className="text-text-secondary animate-pulse">加载中...</div>
      </div>
    )
  }

  // 未填写档案时显示档案表单
  if (hasProfile === false || showProfileForm) {
    return (
      <div className="h-full overflow-y-auto">
        <div className="max-w-lg mx-auto p-4">
          <div className="card">
            <div className="flex items-center justify-between mb-4">
              <h2 className="text-lg font-bold">
                {hasProfile === false ? '完善个人档案' : '编辑档案'}
              </h2>
              {hasProfile && (
                <button
                  onClick={() => setShowProfileForm(false)}
                  className="text-sm text-text-secondary"
                >
                  取消
                </button>
              )}
            </div>
            {hasProfile === false && (
              <p className="text-text-secondary text-sm mb-4">
                为了生成专属于您的个性化运势，我们需要了解您的出生信息。
              </p>
            )}

            <form onSubmit={handleProfileSubmit} className="space-y-4">
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

              {profileError && (
                <p className="text-accent-secondary text-sm">{profileError}</p>
              )}

              <button
                type="submit"
                disabled={profileLoading}
                className="w-full btn-seal disabled:opacity-50"
              >
                {profileLoading ? '保存中...' : '保存并查看运势'}
              </button>
            </form>
          </div>
        </div>
      </div>
    )
  }

  return (
    <div className="h-full overflow-y-auto">
      <div className="max-w-lg mx-auto p-4 space-y-4">
        {error && (
          <div className="card text-accent-secondary text-center">{error}</div>
        )}

        {/* 今日运势 */}
        {today && (
          <div className="card space-y-4 animate-fade-in">
            {/* 日期信息 */}
            <div className="text-center">
              <div className="text-2xl font-bold mb-1">{today.lunar_date}</div>
              <div className="text-sm text-text-secondary">
                {today.ganzhi_year}年 {today.ganzhi_month}月 {today.ganzhi_day}日
              </div>
            </div>

            {/* 能量条 */}
            <div>
              <div className="flex justify-between text-sm mb-2">
                <span>今日能量</span>
                <span className="text-accent-primary">{getEnergyLabel(today.energy_level)}</span>
              </div>
              <div className="energy-bar">
                <div
                  className={`energy-fill ${getEnergyColor(today.energy_level)}`}
                  style={{ width: `${today.energy_level}%` }}
                />
              </div>
            </div>

            {/* 宜忌 */}
            <div className="grid grid-cols-2 gap-3">
              <div className="bg-background-secondary rounded p-3">
                <div className="text-energy-high font-bold mb-1 text-sm">宜</div>
                <div className="text-xs">{today.favorable}</div>
              </div>
              <div className="bg-background-secondary rounded p-3">
                <div className="text-energy-low font-bold mb-1 text-sm">忌</div>
                <div className="text-xs">{today.unfavorable}</div>
              </div>
            </div>

            {/* 吉方吉物 */}
            <div className="flex gap-3 text-sm">
              <div className="flex-1 bg-background-secondary rounded p-2 text-center">
                <div className="text-text-secondary text-xs mb-1">吉方</div>
                <div className="text-accent-primary">{today.lucky_direction}</div>
              </div>
              <div className="flex-1 bg-background-secondary rounded p-2 text-center">
                <div className="text-text-secondary text-xs mb-1">吉物</div>
                <div className="text-accent-primary">{today.lucky_item}</div>
              </div>
            </div>

            {/* 运势解读 */}
            <div>
              <div className="text-sm text-text-secondary mb-1">运势解读</div>
              <div className="text-sm leading-relaxed">{today.commentary}</div>
            </div>
          </div>
        )}

        {/* 档案摘要 */}
        {profile && (
          <div className="card">
            <div className="flex items-center justify-between mb-2">
              <h3 className="font-bold text-sm">我的档案</h3>
              <button
                onClick={() => setShowProfileForm(true)}
                className="text-xs text-accent-primary"
              >
                编辑
              </button>
            </div>
            <div className="text-xs text-text-secondary">
              {profile.nickname} · {profile.gender === 1 ? '男' : '女'} ·
              {profile.birth_year}年{profile.birth_month}月{profile.birth_day}日
              {profile.occupation && ` · ${profile.occupation}`}
            </div>
          </div>
        )}

        {/* 历史回顾 */}
        {validHistory.length > 0 && (
          <div>
            <h3 className="font-bold mb-3">历史回顾</h3>
            <div className="space-y-2">
              {validHistory.map((almanac) => {
                const { month, day, weekday } = formatDisplayDate(almanac.date)
                return (
                  <button
                    key={almanac.date}
                    onClick={() => setSelectedAlmanac(almanac)}
                    className="w-full card py-3 hover:bg-background-secondary/80 transition-colors text-left"
                  >
                    <div className="flex items-center gap-3">
                      <div className="text-center min-w-[40px]">
                        <div className="text-xl font-bold">{day}</div>
                        <div className="text-xs text-text-secondary">{month}月</div>
                      </div>
                      <div className="w-px h-10 bg-border/30" />
                      <div className="flex-1">
                        <div className="text-sm">{almanac.lunar_date}</div>
                        <div className="flex items-center gap-2">
                          <div className={`w-2 h-2 rounded-full ${getEnergyColor(almanac.energy_level)}`} />
                          <span className="text-xs text-text-secondary">
                            {getEnergyLabel(almanac.energy_level)} · {weekday}
                          </span>
                        </div>
                      </div>
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
              <div className="text-center">
                <div className="text-2xl font-bold mb-1">{selectedAlmanac.lunar_date}</div>
                <div className="text-sm text-text-secondary">
                  {selectedAlmanac.ganzhi_year}年 {selectedAlmanac.ganzhi_month}月 {selectedAlmanac.ganzhi_day}日
                </div>
              </div>

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
