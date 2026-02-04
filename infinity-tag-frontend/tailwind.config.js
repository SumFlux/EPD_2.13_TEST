/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  darkMode: 'class',
  theme: {
    extend: {
      colors: {
        // 主色系 - 玄学深色主题
        background: {
          DEFAULT: '#0D0D0D', // 玄黑
          secondary: '#1A1A1A', // 墨灰
          card: '#262626', // 深灰
        },
        accent: {
          primary: '#B87333', // 古铜色
          secondary: '#FF4D4D', // 朱砂红
        },
        text: {
          primary: '#F5F5F0', // 象牙白
          secondary: '#A3A3A3', // 灰白
        },
        border: {
          DEFAULT: '#8B6914', // 暗金
        },
        // 能量等级颜色
        energy: {
          low: '#FF4D4D',
          medium: '#FFB347',
          high: '#4ADE80',
        },
      },
      fontFamily: {
        serif: ['var(--font-cn)', 'Noto Serif SC', 'serif'],
        mono: ['var(--font-en)', 'JetBrains Mono', 'monospace'],
      },
      animation: {
        'fade-in': 'fadeIn 0.3s ease-in-out',
        'slide-up': 'slideUp 0.3s ease-out',
        'pulse-slow': 'pulse 3s cubic-bezier(0.4, 0, 0.6, 1) infinite',
      },
      keyframes: {
        fadeIn: {
          '0%': { opacity: '0' },
          '100%': { opacity: '1' },
        },
        slideUp: {
          '0%': { opacity: '0', transform: 'translateY(10px)' },
          '100%': { opacity: '1', transform: 'translateY(0)' },
        },
      },
      borderRadius: {
        'seal': '4px', // 印章风格圆角
      },
    },
  },
  plugins: [],
}
