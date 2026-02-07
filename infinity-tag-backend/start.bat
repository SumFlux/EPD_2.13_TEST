@echo off
echo ========================================
echo 启动后端服务 (端口 8001)
echo ========================================

cd /d "%~dp0"

echo 检查端口占用...
netstat -ano | findstr :8001 >nul
if %errorlevel% equ 0 (
    echo 警告: 端口 8001 已被占用！
    echo 请先停止占用该端口的进程。
    pause
    exit /b 1
)

echo 启动服务...
uvicorn app.main:app --reload --host 0.0.0.0 --port 8001

pause
