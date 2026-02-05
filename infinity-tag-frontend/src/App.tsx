import { BrowserRouter, Routes, Route, Navigate } from 'react-router-dom'
import { QueryClient, QueryClientProvider } from '@tanstack/react-query'
import { useAuthStore } from '@/stores'
import { SetupPage, SetPasswordPage } from '@/features/auth'
import { ProfilePage } from '@/features/profile'
import { AlmanacPage, AlmanacDetailPage } from '@/features/almanac'
import { ImagesPage } from '@/features/images'
import { AdminLoginPage, DashboardPage, DevicesPage, UsersPage, FirmwarePage } from '@/features/admin'
import Layout from '@/components/common/Layout'

const queryClient = new QueryClient({
  defaultOptions: {
    queries: {
      staleTime: 5 * 60 * 1000, // 5分钟
      retry: 1,
    },
  },
})

function ProtectedRoute({ children }: { children: React.ReactNode }) {
  const { isAuthenticated } = useAuthStore()

  if (!isAuthenticated) {
    return <Navigate to="/setup" replace />
  }

  return <>{children}</>
}

function App() {
  return (
    <QueryClientProvider client={queryClient}>
      <BrowserRouter>
        <Routes>
          {/* 公开路由 */}
          <Route path="/setup" element={<SetupPage />} />
          <Route path="/setup/password" element={<SetPasswordPage />} />

          {/* 管理员路由 */}
          <Route path="/admin/login" element={<AdminLoginPage />} />
          <Route path="/admin/dashboard" element={<DashboardPage />} />
          <Route path="/admin/devices" element={<DevicesPage />} />
          <Route path="/admin/users" element={<UsersPage />} />
          <Route path="/admin/firmware" element={<FirmwarePage />} />

          {/* 受保护路由 */}
          <Route
            path="/"
            element={
              <ProtectedRoute>
                <Layout />
              </ProtectedRoute>
            }
          >
            <Route index element={<Navigate to="/almanac" replace />} />
            <Route path="profile" element={<ProfilePage />} />
            <Route path="almanac" element={<AlmanacPage />} />
            <Route path="almanac/:date" element={<AlmanacDetailPage />} />
            <Route path="images" element={<ImagesPage />} />
          </Route>

          {/* 404 */}
          <Route path="*" element={<Navigate to="/" replace />} />
        </Routes>
      </BrowserRouter>
    </QueryClientProvider>
  )
}

export default App
