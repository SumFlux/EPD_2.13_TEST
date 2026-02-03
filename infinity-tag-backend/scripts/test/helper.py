"""
API 测试工具库
"""
import httpx
import json
import sys
from typing import Dict, Any, Optional

BASE_URL = "http://localhost:8000/api/v1"

# 颜色输出
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def print_step(msg: str):
    print(f"\n{Colors.HEADER}➤ {msg}{Colors.ENDC}")

def print_success(msg: str):
    print(f"{Colors.OKGREEN}✅ {msg}{Colors.ENDC}")

def print_error(msg: str):
    print(f"{Colors.FAIL}❌ {msg}{Colors.ENDC}")

def print_json(data: Any):
    print(json.dumps(data, indent=2, ensure_ascii=False))

def load_token() -> str:
    """从临时文件读取 Token"""
    try:
        with open("scripts/test/.token", "r") as f:
            return f.read().strip()
    except FileNotFoundError:
        print_error("未找到 Token，请先运行 01_activate_device.py")
        sys.exit(1)

def save_token(token: str):
    """保存 Token 到临时文件"""
    with open("scripts/test/.token", "w") as f:
        f.write(token)
    print_success("Token 已保存至 scripts/test/.token")
