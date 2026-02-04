import { useEffect, useState } from 'react'
import { useParams, useNavigate } from 'react-router-dom'
import { useAlmanacStore } from '@/stores'
import { almanacApi } from '@/api'
import { getEnergyColor, getEnergyLabel } from '@/utils'
import type { Almanac } from '@/types'

export default function AlmanacDetailPage() {
  const { date } = useParams<{ date: string }>()
  const navigate = useNavigate()
  const { history } = useAlmanacStore()
  const [almanac, setAlmanac] = useState<Almanac | null>(null)
  const [isLoading, setIsLoading] = useState(true)

  useEffect(() => {
    const fetchData = async () => {
      // 先从缓存中查找
      const cached = history.find((a) => a.date === date)
      if (cached) {
        setAlmanac(cached)
        setIsLoading(false)
        return
      }

      // 否则从服务器获取
      try {
        const data = await almanacApi.generate({ target_date: date })
        setAlmanac(data)
      } catch {
        navigate('/almanac')
      } finally {
        setIsLoading(false)
      }
    }

    if (date) {
      fetchData()
    }
  }, [date, history, navigate])

  if (isLoading) {
    return (
      <div className="min-h-screen flex items-center justify-center">
        <div className="text-text-secondary animate-pulse">加载中...</div>
      </div>
    )
  }

  if (!almanac) {
    return null
  }

  return (
    <div className="min-h-screen p-4">
      <div className="max-w-lg mx-auto">
        {/* 返回按钮 */}
        <button
          onClick={() => navigate('/almanac')}
          className="mb-4 text-text-secondary hover:text-text-primary transition-colors"
        >
          ← 返回
        </button>

        <div className="card space-y-6 animate-fade-in">
          {/* 日期信息 */}
          <div className="text-center">
            <div className="text-sm text-text-secondary mb-1">{date}</div>
            <div className="text-3xl font-bold mb-2">{almanac.lunar_date}</div>
            <div className="text-text-secondary">
              {almanac.ganzhi_year}年 {almanac.ganzhi_month}月 {almanac.ganzhi_day}日
            </div>
          </div>

          {/* 能量条 */}
          <div>
            <div className="flex justify-between text-sm mb-2">
              <span>当日能量</span>
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
      </div>
    </div>
  )
}
