"""
测试步骤 2: 设置用户档案
设置用户的生辰八字，用于个性化黄历生成
"""
import httpx
import sys
import os

# 将当前目录加入路径，以便导入 helper
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from helper import BASE_URL, print_step, print_success, print_error, print_json, load_token

def test_set_profile():
    print_step("步骤 2: 设置用户档案 (POST /user/profile)")

    token = load_token()
    headers = {"Authorization": f"Bearer {token}"}

    # 构造档案数据 (UserProfileCreate)
    # 使用一个测试用的生辰: 1990年1月1日 12点 (午时)
    payload = {
        "nickname": "测试用户",
        "gender": 1,  # 1男 0女
        "birth_year": 2002,
        "birth_month": 2,
        "birth_day": 28,
        "birth_hour": 11,
        "is_lunar": False,
        "occupation": "程序员",  # 替换 mbti
        "notes": "自动化测试账户"
    }

    print(f"发送请求: {payload}")

    try:
        response = httpx.post(
            f"{BASE_URL}/user/profile",
            json=payload,
            headers=headers
        )

        if response.status_code == 200:
            data = response.json()
            if data.get("success"):
                print_success("档案设置成功！")
                print_json(data)
            else:
                print_error(f"档案设置失败: {data.get('message')}")
        else:
            print_error(f"请求失败: {response.status_code}")
            print(response.text)

    except Exception as e:
        print_error(f"请求异常: {e}")

if __name__ == "__main__":
    test_set_profile()
