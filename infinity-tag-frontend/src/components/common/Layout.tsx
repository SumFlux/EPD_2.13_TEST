import { Outlet, NavLink, useNavigate } from 'react-router-dom'
import { useAuthStore } from '@/stores'

const navItems = [
  { path: '/almanac', label: '运势' },
  { path: '/images', label: '图片' },
]

export default function Layout() {
  const navigate = useNavigate()
  const { logout } = useAuthStore()

  const handleLogout = () => {
    if (confirm('确定要退出登录吗？')) {
      logout()
      navigate('/setup')
    }
  }

  return (
    <div className="h-screen flex flex-col overflow-hidden">
      {/* 顶部导航 - 固定高度 */}
      <header className="flex-shrink-0 bg-background-secondary border-b border-border/30">
        <div className="max-w-2xl mx-auto px-4 py-3 flex items-center justify-between">
          <h1 className="text-lg font-bold text-gradient">无止便签</h1>
          <button
            onClick={handleLogout}
            className="text-sm text-text-secondary hover:text-text-primary transition-colors"
          >
            退出
          </button>
        </div>
      </header>

      {/* 主内容 - 填充剩余空间，内部滚动 */}
      <main className="flex-1 overflow-y-auto">
        <Outlet />
      </main>

      {/* 底部导航 - 固定在底部 */}
      <nav className="flex-shrink-0 bg-background-secondary border-t border-border/30">
        <div className="max-w-2xl mx-auto px-4">
          <div className="flex">
            {navItems.map((item) => (
              <NavLink
                key={item.path}
                to={item.path}
                className={({ isActive }) =>
                  `flex-1 py-4 text-center text-sm transition-colors ${
                    isActive
                      ? 'text-accent-primary border-t-2 border-accent-primary -mt-[2px]'
                      : 'text-text-secondary hover:text-text-primary'
                  }`
                }
              >
                {item.label}
              </NavLink>
            ))}
          </div>
        </div>
      </nav>
    </div>
  )
}
