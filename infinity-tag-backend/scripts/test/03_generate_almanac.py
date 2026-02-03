"""
测试步骤 3: 生成今日黄历
基于用户档案生成个性化黄历
"""
import httpx
import sys
import os
from datetime import date

# 将当前目录加入路径，以便导入 helper
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from helper import BASE_URL, print_step, print_success, print_error, print_json, load_token

def test_generate_almanac():
    print_step("步骤 3: 生成今日黄历 (POST /almanac/generate)")

    token = load_token()
    headers = {"Authorization": f"Bearer {token}"}

    # 请求今日黄历
    today = date.today().isoformat()
    payload = {
        "target_date": today
    }

    print(f"发送请求: {payload}")

    try:
        # 注意: 这里根据后端实现，可能需要较长时间等待 AI 生成
        print("正在请求 AI 生成黄历，请耐心等待 (可能需要 10-30 秒)...")
        response = httpx.post(
            f"{BASE_URL}/almanac/generate",
            json=payload,
            headers=headers,
            timeout=60.0  # 设置较长的超时时间
        )

        if response.status_code == 200:
            data = response.json()
            if data.get("success"):
                print_success("黄历生成成功！")
                print_json(data)
            else:
                print_error(f"黄历生成失败: {data.get('message')}")
        else:
            print_error(f"请求失败: {response.status_code}")
            print(response.text)

    except Exception as e:
        print_error(f"请求异常: {e}")

if __name__ == "__main__":
    test_generate_almanac()
