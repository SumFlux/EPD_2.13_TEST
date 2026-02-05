import { useNavigate, useLocation } from 'react-router-dom'
import { useAdminStore } from '@/stores/adminStore'
import { LogOut, LayoutDashboard, Smartphone, Users, UploadCloud } from 'lucide-react'

interface AdminHeaderProps {
    title: string
}

export function AdminHeader({ title }: AdminHeaderProps) {
    const navigate = useNavigate()
    const location = useLocation()
    const { adminLogout } = useAdminStore()

    const navItems = [
        { label: '仪表盘', path: '/admin/dashboard', icon: LayoutDashboard },
        { label: '设备管理', path: '/admin/devices', icon: Smartphone },
        { label: '用户管理', path: '/admin/users', icon: Users },
        { label: '固件管理', path: '/admin/firmware', icon: UploadCloud },
    ]

    const handleLogout = () => {
        if (confirm('确定要退出登录吗？')) {
            adminLogout()
            navigate('/admin/login')
        }
    }

    return (
        <nav className="bg-white shadow-sm mb-6">
            <div className="max-w-7xl mx-auto px-4 py-3 flex justify-between items-center">
                <h1 className="text-xl font-bold text-gray-800 flex items-center gap-2">
                    {title}
                </h1>
                <div className="flex items-center space-x-1">
                    {navItems.map((item) => {
                        const isActive = location.pathname === item.path
                        return (
                            <button
                                key={item.path}
                                onClick={() => navigate(item.path)}
                                className={`flex items-center gap-1 px-3 py-2 rounded-md text-sm font-medium transition-colors ${isActive
                                        ? 'text-blue-600 bg-blue-50'
                                        : 'text-gray-600 hover:text-gray-900 hover:bg-gray-50'
                                    }`}
                            >
                                <item.icon className="w-4 h-4" />
                                {item.label}
                            </button>
                        )
                    })}

                    <div className="w-px h-6 bg-gray-200 mx-2"></div>

                    <button
                        onClick={handleLogout}
                        className="flex items-center gap-1 px-3 py-2 rounded-md text-sm font-medium text-red-600 hover:bg-red-50 hover:text-red-800 transition-colors"
                    >
                        <LogOut className="w-4 h-4" />
                        退出
                    </button>
                </div>
            </div>
        </nav>
    )
}
