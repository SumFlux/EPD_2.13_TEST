import requests
import argparse
import sys
import os

# 将项目根目录添加到 python path 以便导入 app 模块
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from app.config import settings
from app.core.security import create_access_token

def upload_firmware(base_url, firmware_path, version, description=None):
    # 1. 本地生成 Admin Token (无需调用登录接口，直接使用后端密钥签发)
    print(f"Generating admin token for user: {settings.ADMIN_USERNAME}...")
    try:
        # 构造 subject: admin:{username}
        subject = f"admin:{settings.ADMIN_USERNAME}"
        token = create_access_token(subject=subject)
        print("Token generated successfully (using .env configuration).")
    except Exception as e:
        print(f"Error generating token: {e}")
        sys.exit(1)

    # 2. Upload Firmware
    upload_url = f"{base_url}/api/v1/ota/upload"
    headers = {"Authorization": f"Bearer {token}"}
    
    data = {"version": version}
    if description:
        data["description"] = description
        
    files = {
        "file": (os.path.basename(firmware_path), open(firmware_path, "rb"), "application/octet-stream")
    }

    try:
        print(f"Uploading {firmware_path} (v{version})...")
        response = requests.post(upload_url, headers=headers, data=data, files=files)
        response.raise_for_status()
        result = response.json()
        print("\n✅ Firmware uploaded successfully!")
        print(f"ID: {result.get('id')}")
        print(f"Version: {result.get('version_str')} (Code: {result.get('version_code')})")
        print(f"Checksum: {result.get('checksum')}")
        print(f"Path: {result.get('file_path')}")
    except requests.exceptions.HTTPError as e:
        print(f"\n❌ Upload failed: {e}")
        if e.response is not None:
             print(f"Response: {e.response.text}")
    except Exception as e:
        print(f"\n❌ An error occurred: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Upload firmware to Infinity Tag Backend")
    parser.add_argument("file", help="Path to the firmware binary file (.bin)")
    parser.add_argument("version", help="Version string (e.g., 1.0.0.1)")
    parser.add_argument("--desc", help="Description of the firmware update")
    parser.add_argument("--url", default="http://localhost:8000", help="Backend API URL (default: http://localhost:8000)")
    
    # 移除 --pwd 参数，直接使用 .env 配置
    # parser.add_argument("--pwd", default="admin", help="Admin password (default: admin)")

    args = parser.parse_args()

    if not os.path.exists(args.file):
        print(f"Error: File '{args.file}' not found.")
        sys.exit(1)

    upload_firmware(args.url, args.file, args.version, args.desc)
