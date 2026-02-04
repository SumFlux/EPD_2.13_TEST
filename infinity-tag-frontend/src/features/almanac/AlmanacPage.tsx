import { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAlmanacStore } from '@/stores'
import { almanacApi } from '@/api'
import { formatDate, getEnergyColor, getEnergyLabel } from '@/utils'
import type { Almanac } from '@/types'

export default function AlmanacPage() {
  const navigate = useNavigate()
  const { today, history, setToday, setHistory, isLoading, setLoading, error, setError } = useAlmanacStore()
  const [viewMode, setViewMode] = useState<'today' | 'calendar'>('today')

  useEffect(() => {
    const fetchData = async () => {
      setLoading(true)
      try {
        const [todayData, historyData] = await Promise.all([
          almanacApi.generate(),
          almanacApi.history(30),
        ])
        setToday(todayData)
        setHistory(historyData)
      } catch {
        setError('加载失败，请重试')
      } finally {
        setLoading(false)
      }
    }
    fetchData()
  }, [setToday, setHistory, setLoading, setError])

  const handleDateClick = (date: string) => {
    navigate(`/almanac/${date}`)
  }

  if (isLoading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-text-secondary animate-pulse">加载中...</div>
      </div>
    )
  }

  return (
    <div className="min-h-screen p-4">
      <div className="max-w-2xl mx-auto">
        {/* 切换按钮 */}
        <div className="flex gap-2 mb-6">
          <button
            onClick={() => setViewMode('today')}
            className={`flex-1 py-2 rounded transition-colors ${
              viewMode === 'today'
                ? 'bg-accent-primary text-text-primary'
                : 'bg-background-secondary text-text-secondary'
            }`}
          >
            今日运势
          </button>
          <button
            onClick={() => setViewMode('calendar')}
            className={`flex-1 py-2 rounded transition-colors ${
              viewMode === 'calendar'
                ? 'bg-accent-primary text-text-primary'
                : 'bg-background-secondary text-text-secondary'
            }`}
          >
            历史回顾
          </button>
        </div>

        {error && (
          <div className="card mb-4 text-accent-secondary text-center">{error}</div>
        )}

        {viewMode === 'today' && today && (
          <TodayView almanac={today} />
        )}

        {viewMode === 'calendar' && (
          <CalendarView history={history} onDateClick={handleDateClick} />
        )}
      </div>
    </div>
  )
}

function TodayView({ almanac }: { almanac: Almanac }) {
  return (
    <div className="card space-y-6 animate-fade-in">
      {/* 日期信息 */}
      <div className="text-center">
        <div className="text-3xl font-bold mb-2">{almanac.lunar_date}</div>
        <div className="text-text-secondary">
          {almanac.ganzhi_year}年 {almanac.ganzhi_month}月 {almanac.ganzhi_day}日
        </div>
      </div>

      {/* 能量条 */}
      <div>
        <div className="flex justify-between text-sm mb-2">
          <span>今日能量</span>
          <span className="text-accent-primary">{getEnergyLabel(almanac.energy_level)}</span>
        </div>
        <div className="energy-bar">
          <div
            className={`energy-fill ${getEnergyColor(almanac.energy_level)}`}
            style={{ width: `${almanac.energy_level}%` }}
          />
        </div>
      </div>

      {/* 宜忌 */}
      <div className="grid grid-cols-2 gap-4">
        <div className="bg-background-secondary rounded p-4">
          <div className="text-energy-high font-bold mb-2">宜</div>
          <div className="text-sm">{almanac.favorable}</div>
        </div>
        <div className="bg-background-secondary rounded p-4">
          <div className="text-energy-low font-bold mb-2">忌</div>
          <div className="text-sm">{almanac.unfavorable}</div>
        </div>
      </div>

      {/* 吉方吉物 */}
      <div className="flex gap-4 text-sm">
        <div className="flex-1 bg-background-secondary rounded p-3 text-center">
          <div className="text-text-secondary mb-1">吉方</div>
          <div className="text-accent-primary">{almanac.lucky_direction}</div>
        </div>
        <div className="flex-1 bg-background-secondary rounded p-3 text-center">
          <div className="text-text-secondary mb-1">吉物</div>
          <div className="text-accent-primary">{almanac.lucky_item}</div>
        </div>
      </div>

      {/* 运势解读 */}
      <div>
        <div className="text-sm text-text-secondary mb-2">运势解读</div>
        <div className="text-sm leading-relaxed">{almanac.commentary}</div>
      </div>
    </div>
  )
}

function CalendarView({
  history,
  onDateClick,
}: {
  history: Almanac[]
  onDateClick: (date: string) => void
}) {
  const historyMap = new Map(history.map((a) => [a.date, a]))

  // 生成最近30天的日期
  const dates: string[] = []
  for (let i = 0; i < 30; i++) {
    const date = new Date()
    date.setDate(date.getDate() - i)
    dates.push(formatDate(date))
  }

  return (
    <div className="grid grid-cols-7 gap-2 animate-fade-in">
      {dates.map((date) => {
        const almanac = historyMap.get(date)
        const day = new Date(date).getDate()

        return (
          <button
            key={date}
            onClick={() => almanac && onDateClick(date)}
            disabled={!almanac}
            className={`
              aspect-square rounded flex flex-col items-center justify-center
              transition-colors
              ${almanac
                ? 'bg-background-card hover:bg-background-secondary cursor-pointer'
                : 'bg-background-secondary/50 cursor-not-allowed opacity-50'
              }
            `}
          >
            <span className="text-sm">{day}</span>
            {almanac && (
              <div
                className={`w-2 h-2 rounded-full mt-1 ${getEnergyColor(almanac.energy_level)}`}
              />
            )}
          </button>
        )
      })}
    </div>
  )
}
