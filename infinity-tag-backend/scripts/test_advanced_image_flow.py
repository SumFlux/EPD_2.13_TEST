import requests
import json
import os
from PIL import Image, ImageDraw
import io
import time

# 配置
BASE_URL = "http://localhost:8000/api/v1"
TEST_DEVICE_ID = "sumhello"  # 测试设备ID
TEST_SECRET = "123456"   # 测试密钥

def create_complex_test_image():
    """创建一个包含文字、形状和渐变的复杂测试图片"""
    width, height = 400, 300
    img = Image.new('RGB', (width, height), color='white')
    d = ImageDraw.Draw(img)

    # 绘制渐变背景
    for i in range(height):
        r = int(255 * (i / height))
        g = int(255 * (1 - i / height))
        b = 128
        d.line([(0, i), (width, i)], fill=(r, g, b))

    # 绘制形状
    d.rectangle([50, 50, 150, 150], fill='red', outline='black')
    d.ellipse([200, 50, 300, 150], fill='blue', outline='white')

    # 绘制文字
    d.text((10, 10), "Test Image", fill='black')
    d.text((10, 200), "PREVIEW TEST", fill='white')

    # 保存到内存
    img_byte_arr = io.BytesIO()
    img.save(img_byte_arr, format='PNG')
    img_byte_arr.seek(0)
    return img_byte_arr

def login():
    print(f"[*] 连接后端: {BASE_URL}")
    print("[-] 正在登录...")
    try:
        # 使用 JSON 格式请求 /auth/login
        payload = {
            "device_id": TEST_DEVICE_ID,
            "password": TEST_SECRET
        }
        resp = requests.post(f"{BASE_URL}/auth/login", json=payload)

        if resp.status_code != 200:
            print(f"[x] 登录失败: {resp.text}")
            return None

        token = resp.json()["access_token"]
        print(f"[+] 登录成功! Token: {token[:10]}...")
        return {"Authorization": f"Bearer {token}"}
    except Exception as e:
        print(f"[x] 登录异常: {e}")
        return None

def test_preview(headers):
    print("\n[-] 测试预览接口 (/images/preview)...")

    img_data = create_complex_test_image()

    # 测试场景 1: 旋转 90 度 + 反色
    options = {
        "rotate": 90,
        "invert": True,
        "dither": True
    }

    files = {
        'file': ('test_preview.png', img_data, 'image/png'),
        'options': (None, json.dumps(options), 'application/json')
    }

    try:
        start_time = time.time()
        # 修改：使用 standard data parameter 传递 form fields
        files = {'file': ('test_preview.png', img_data, 'image/png')}
        data = {'options': json.dumps(options)}

        resp = requests.post(f"{BASE_URL}/images/preview", files=files, data=data)
        elapsed = (time.time() - start_time) * 1000

        if resp.status_code == 200:
            print(f"[+] 预览成功! 耗时: {elapsed:.2f}ms")
            # 保存预览图到本地查看
            with open("preview_result.png", "wb") as f:
                f.write(resp.content)
            print("    已保存预览结果到 'preview_result.png' (请打开查看是否旋转且反色)")
        else:
            print(f"[x] 预览失败: {resp.text}")
    except Exception as e:
        print(f"[x] 预览请求异常: {e}")

def test_upload_with_options(headers):
    print("\n[-] 测试带参数上传 (/images/)...")

    img_data = create_complex_test_image()
    img_data.seek(0) # 重置指针

    # 测试场景 2: 阈值模式 (Threshold=200) + 裁剪
    options = {
        "dither": False,
        "threshold": 200,
        "crop_x": 50,
        "crop_y": 50,
        "crop_w": 200,
        "crop_h": 100
    }

    files = {
        'file': ('test_upload.png', img_data, 'image/png'),
        'options': (None, json.dumps(options), 'application/json')
    }

    try:
        # 修改：使用 standard data parameter 传递 form fields
        files = {'file': ('test_upload.png', img_data, 'image/png')}
        data = {'options': json.dumps(options)}

        resp = requests.post(f"{BASE_URL}/images/", headers=headers, files=files, data=data)

        if resp.status_code == 200:
            data = resp.json()
            print(f"[+] 上传成功! ID: {data['id']}")
            print(f"[+] 最终图片 URL: {data['url']}")
            print("    (请在浏览器查看，确认是否应用了裁剪和阈值处理)")
        else:
            print(f"[x] 上传失败: {resp.text}")
    except Exception as e:
        print(f"[x] 上传请求异常: {e}")

if __name__ == "__main__":
    headers = login()
    if headers:
        test_preview(headers)
        test_upload_with_options(headers)
