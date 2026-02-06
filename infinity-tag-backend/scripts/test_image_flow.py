import requests
import json
import os
from PIL import Image, ImageDraw
import io

# 配置
BASE_URL = "http://localhost:8000/api/v1"
TEST_DEVICE_ID = "TEST01"  # 测试设备ID
TEST_SECRET = "password123"   # 测试密钥

def create_test_image():
    """创建一个简单的彩色测试图片"""
    img = Image.new('RGB', (400, 300), color='red')
    d = ImageDraw.Draw(img)
    d.text((10, 10), "Hello EPD", fill='white')
    d.rectangle([50, 50, 150, 150], fill='blue')
    d.ellipse([200, 50, 300, 150], fill='green')

    # 保存到内存
    img_byte_arr = io.BytesIO()
    img.save(img_byte_arr, format='PNG')
    img_byte_arr.seek(0)
    return img_byte_arr

def run_test():
    print(f"[*] 连接后端: {BASE_URL}")

    # 1. 激活/注册设备 (如果不存在)
    print("[-] 尝试激活测试设备...")
    try:
        # 这里假设有一个激活接口，或者直接登录。
        # 如果是新设备需要先激活，这里简化流程，先尝试登录，失败则提示
        pass
    except Exception as e:
        print(f"[!] 警告: {e}")

    # 2. 登录获取 Token
    print("[-] 正在登录...")
    # 注意：这里需要根据实际的 Auth 接口调整
    # 假设是 POST /auth/token 或 /auth/login
    # 根据之前的代码 user.py/auth.py，通常是用 device_id + secret 换取 token
    # 这里为了演示，我们先尝试用 requests.Session

    session = requests.Session()

    # 模拟登录 Payload (根据实际 API 调整)
    login_data = {
        "device_id": TEST_DEVICE_ID,
        "password": TEST_SECRET  # 或 device_secret
    }

    # 尝试调用登录 (根据 auth.py 的实际实现)
    # 如果您还没有注册该设备，可能需要先手动在数据库插入或调用注册接口
    # 假设 endpoint 是 /auth/login
    try:
        resp = requests.post(f"{BASE_URL}/auth/login/access-token", data={"username": TEST_DEVICE_ID, "password": TEST_SECRET})
        if resp.status_code != 200:
            print(f"[x] 登录失败: {resp.text}")
            print("    提示: 请确保数据库中存在该测试设备，或先使用 /auth/activate 激活")
            return

        token = resp.json()["access_token"]
        headers = {"Authorization": f"Bearer {token}"}
        print(f"[+] 登录成功! Token: {token[:10]}...")
    except Exception as e:
        print(f"[x] 登录异常: {e}")
        return

    # 3. 上传图片
    print("[-] 正在上传测试图片...")
    files = {'file': ('test.png', create_test_image(), 'image/png')}
    resp = requests.post(f"{BASE_URL}/images/", headers=headers, files=files)

    if resp.status_code != 200:
        print(f"[x] 上传失败: {resp.text}")
        return

    data = resp.json()
    image_id = data['id']
    preview_url = data['url']
    print(f"[+] 上传成功! ID: {image_id}")
    print(f"[+] 预览链接: {preview_url}")
    print(f"    (请在浏览器打开 http://localhost:8000{preview_url} 查看抖动效果)")

    # 4. 获取 Bitmap (模拟设备端下载)
    print(f"[-] 正在下载设备端 Bitmap 数据 (ID: {image_id})...")
    resp = requests.get(f"{BASE_URL}/images/{image_id}/bitmap", headers=headers)

    if resp.status_code == 200:
        bitmap_size = len(resp.content)
        print(f"[+] Bitmap 下载成功! 大小: {bitmap_size} bytes")
        # 验证大小: 212 * 104 / 8 向上取整
        expected_size = ((212 * 104) + 7) // 8
        print(f"    (预期大小: 约 {expected_size} bytes)")
    else:
        print(f"[x] Bitmap 下载失败: {resp.status_code}")

if __name__ == "__main__":
    run_test()
