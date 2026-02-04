"""
认证 API 接口测试
测试 /api/v1/auth 下的所有端点
"""
import pytest
from httpx import AsyncClient


class TestActivateEndpoint:
    """测试 POST /api/v1/auth/activate 端点"""

    @pytest.mark.integration
    async def test_activate_success(self, client: AsyncClient, test_device):
        """测试正常激活流程"""
        response = await client.post(
            "/api/v1/auth/activate",
            json={
                "device_code": "ABC123",
                "init_password": "123456"
            }
        )

        assert response.status_code == 200
        data = response.json()
        assert data["success"] is True
        assert data["requires_password_setup"] is True
        assert "temp_token" in data
        assert data["device_code"] == "ABC123"

    @pytest.mark.integration
    async def test_activate_invalid_device_code(self, client: AsyncClient, test_device):
        """测试无效的设备码"""
        response = await client.post(
            "/api/v1/auth/activate",
            json={
                "device_code": "WRONG1",
                "init_password": "123456"
            }
        )

        assert response.status_code == 404
        assert "无效的设备码" in response.json()["detail"]

    @pytest.mark.integration
    async def test_activate_wrong_init_password(self, client: AsyncClient, test_device):
        """测试错误的初始密码"""
        response = await client.post(
            "/api/v1/auth/activate",
            json={
                "device_code": "ABC123",
                "init_password": "wrong1"
            }
        )

        assert response.status_code == 401
        assert "初始密码错误" in response.json()["detail"]

    @pytest.mark.integration
    async def test_activate_already_activated(self, client: AsyncClient, activated_device_and_user):
        """测试已激活的设备"""
        response = await client.post(
            "/api/v1/auth/activate",
            json={
                "device_code": "ABC123",
                "init_password": "123456"
            }
        )

        assert response.status_code == 400
        assert "已被激活" in response.json()["detail"]

    @pytest.mark.integration
    async def test_activate_validation_error(self, client: AsyncClient):
        """测试请求参数验证"""
        # 设备码长度不对
        response = await client.post(
            "/api/v1/auth/activate",
            json={
                "device_code": "AB",  # 太短
                "init_password": "123456"
            }
        )

        assert response.status_code == 422


class TestSetPasswordEndpoint:
    """测试 POST /api/v1/auth/set-password 端点"""

    @pytest.mark.integration
    async def test_set_password_success(self, client: AsyncClient, test_device):
        """测试设置密码成功"""
        # 先激活获取临时 token
        activate_response = await client.post(
            "/api/v1/auth/activate",
            json={
                "device_code": "ABC123",
                "init_password": "123456"
            }
        )
        temp_token = activate_response.json()["temp_token"]

        # 设置密码
        response = await client.post(
            "/api/v1/auth/set-password",
            json={"new_password": "my_secure_password"},
            headers={"Authorization": f"Bearer {temp_token}"}
        )

        assert response.status_code == 200
        data = response.json()
        assert "access_token" in data
        assert data["token_type"] == "bearer"
        assert data["device_id"] == "ABC123"

    @pytest.mark.integration
    async def test_set_password_without_token(self, client: AsyncClient):
        """测试无 token 设置密码"""
        response = await client.post(
            "/api/v1/auth/set-password",
            json={"new_password": "my_secure_password"}
        )

        assert response.status_code == 401

    @pytest.mark.integration
    async def test_set_password_invalid_token(self, client: AsyncClient):
        """测试无效 token"""
        response = await client.post(
            "/api/v1/auth/set-password",
            json={"new_password": "my_secure_password"},
            headers={"Authorization": "Bearer invalid_token"}
        )

        assert response.status_code == 401

    @pytest.mark.integration
    async def test_set_password_already_set(self, client: AsyncClient, test_device):
        """测试密码已设置后再次设置"""
        # 先激活
        activate_response = await client.post(
            "/api/v1/auth/activate",
            json={
                "device_code": "ABC123",
                "init_password": "123456"
            }
        )
        temp_token = activate_response.json()["temp_token"]

        # 第一次设置密码
        await client.post(
            "/api/v1/auth/set-password",
            json={"new_password": "my_secure_password"},
            headers={"Authorization": f"Bearer {temp_token}"}
        )

        # 再次尝试设置密码（应该失败，因为临时 token 对应的用户已设置密码）
        response = await client.post(
            "/api/v1/auth/set-password",
            json={"new_password": "another_password"},
            headers={"Authorization": f"Bearer {temp_token}"}
        )

        assert response.status_code == 400
        assert "密码已设置" in response.json()["detail"]


class TestLoginEndpoint:
    """测试 POST /api/v1/auth/login 端点"""

    @pytest.mark.integration
    async def test_login_success(self, client: AsyncClient, activated_device_and_user):
        """测试正常登录"""
        response = await client.post(
            "/api/v1/auth/login",
            json={
                "device_code": "ABC123",
                "password": "user_password"
            }
        )

        assert response.status_code == 200
        data = response.json()
        assert "access_token" in data
        assert data["token_type"] == "bearer"

    @pytest.mark.integration
    async def test_login_wrong_password(self, client: AsyncClient, activated_device_and_user):
        """测试密码错误"""
        response = await client.post(
            "/api/v1/auth/login",
            json={
                "device_code": "ABC123",
                "password": "wrong_password"
            }
        )

        assert response.status_code == 401
        assert "设备码或密码错误" in response.json()["detail"]

    @pytest.mark.integration
    async def test_login_nonexistent_device(self, client: AsyncClient):
        """测试设备不存在"""
        response = await client.post(
            "/api/v1/auth/login",
            json={
                "device_code": "NODEV1",
                "password": "any_password"
            }
        )

        assert response.status_code == 401

    @pytest.mark.integration
    async def test_login_password_not_set(self, client: AsyncClient, test_device):
        """测试用户未设置密码"""
        # 先激活但不设置密码
        await client.post(
            "/api/v1/auth/activate",
            json={
                "device_code": "ABC123",
                "init_password": "123456"
            }
        )

        # 尝试登录
        response = await client.post(
            "/api/v1/auth/login",
            json={
                "device_code": "ABC123",
                "password": "any_password"
            }
        )

        assert response.status_code == 400
        assert "请先完成激活并设置密码" in response.json()["detail"]


class TestMeEndpoint:
    """测试 GET /api/v1/auth/me 端点"""

    @pytest.mark.integration
    async def test_me_success(self, client: AsyncClient, activated_device_and_user):
        """测试获取当前用户信息"""
        # 先登录获取 token
        login_response = await client.post(
            "/api/v1/auth/login",
            json={
                "device_code": "ABC123",
                "password": "user_password"
            }
        )
        access_token = login_response.json()["access_token"]

        # 获取用户信息
        response = await client.get(
            "/api/v1/auth/me",
            headers={"Authorization": f"Bearer {access_token}"}
        )

        assert response.status_code == 200
        data = response.json()
        assert data["device_id"] == "ABC123"
        assert data["password_set"] is True

    @pytest.mark.integration
    async def test_me_without_token(self, client: AsyncClient):
        """测试无 token 访问"""
        response = await client.get("/api/v1/auth/me")

        # HTTPBearer 返回 401 或 403，取决于配置
        assert response.status_code in (401, 403)

    @pytest.mark.integration
    async def test_me_invalid_token(self, client: AsyncClient):
        """测试无效 token"""
        response = await client.get(
            "/api/v1/auth/me",
            headers={"Authorization": "Bearer invalid_token"}
        )

        assert response.status_code == 403
