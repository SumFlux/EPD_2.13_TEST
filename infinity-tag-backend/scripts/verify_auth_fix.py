import httpx
import time
import sys
import random
import string

BASE_URL = "http://localhost:8002/api/v1"

def wait_for_server():
    print("Waiting for server to start...")
    for _ in range(20):
        try:
            r = httpx.get(f"http://localhost:8002/health")
            if r.status_code == 200:
                print("Server is up!")
                return True
        except:
            pass
        time.sleep(1)
    print("Server failed to start.")
    return False

def verify_auth():
    # 1. Activate
    password = "test_password_123"
    print("\n[TEST] Testing /activate...")
    try:
        resp = httpx.post(f"{BASE_URL}/auth/activate", json={"password": password})
        if resp.status_code != 200:
            print(f"Activation failed: {resp.text}")
            return False

        data = resp.json()
        device_id = data.get("device_id")
        secret = data.get("device_secret")

        if not device_id:
            print("❌ Activation response missing device_id")
            return False

        if not secret:
             print("[FAIL] Activation response missing device_secret (It SHOULD be here)")
             return False

        print(f"[OK] Activation success. Device ID: {device_id}")
        print(f"[OK] Device Secret received: {secret[:5]}******")

        # 2. Login
        print("\n[TEST] Testing /login...")
        login_resp = httpx.post(f"{BASE_URL}/auth/login", json={"device_id": device_id, "password": password})

        if login_resp.status_code != 200:
            print(f"Login failed: {login_resp.text}")
            return False

        login_data = login_resp.json()

        # 3. Check for Secret Leak
        if "device_secret" in login_data:
            print("[FAIL] CRITICAL FAIL: /login response contains device_secret!")
            print(f"Response keys: {list(login_data.keys())}")
            return False

        print("[OK] Login response does NOT contain device_secret.")
        print(f"Login Response keys: {list(login_data.keys())}")
        return True

    except Exception as e:
        print(f"Error during verification: {e}")
        return False

if __name__ == "__main__":
    if wait_for_server():
        success = verify_auth()
        if success:
            print("\n[SUCCESS] Verification Passed!")
            sys.exit(0)
        else:
            print("\n[FAIL] Verification Failed!")
            sys.exit(1)
    else:
        sys.exit(1)
