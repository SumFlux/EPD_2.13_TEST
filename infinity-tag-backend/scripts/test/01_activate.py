"""
测试步骤 1: 设备激活
模拟设备首次连接，获取 Access Token
"""
import httpx
import sys
import os

# 将当前目录加入路径，以便导入 helper
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from helper import BASE_URL, print_step, print_success, print_error, print_json, save_token, Colors

def test_activate():
    print_step("步骤 1: 设备激活 (POST /auth/activate)")

    # 模拟一个设备码 (6位) 和 硬件ID
    payload = {
        "device_code": "TEST01",
        "device_uuid": "test-uuid-123456789"
    }

    print(f"发送请求: {payload}")

    try:
        response = httpx.post(f"{BASE_URL}/auth/activate", json=payload)

        if response.status_code == 200:
            data = response.json()
            print_success("激活成功！")
            print_json(data)

            # 保存 Token 供后续步骤使用
            token = data["data"]["access_token"]
            save_token(token)
        else:
            print_error(f"激活失败: {response.status_code}")
            print(response.text)

    except Exception as e:
        print_error(f"请求异常: {e}")
        print(f"{Colors.WARNING}提示: 请确保后端服务已启动 (uvicorn app.main:app){Colors.ENDC}")

if __name__ == "__main__":
    test_activate()
