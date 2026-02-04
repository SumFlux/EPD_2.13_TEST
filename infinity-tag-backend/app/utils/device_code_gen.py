"""
设备码和初始密码生成算法
使用 HMAC-SHA256 从 ESP32 UUID 生成确定性的设备码和初始密码
"""
import hmac
import hashlib
import random
from app.config import settings


def generate_device_code(uuid: str) -> str:
    """
    算法A：UUID → 6位设备码
    使用 DEVICE_CODE_SALT 作为 HMAC 密钥

    :param uuid: ESP32 芯片 UUID
    :return: 6位大写字母+数字的设备码
    """
    digest = hmac.new(
        settings.DEVICE_CODE_SALT.encode(),
        uuid.encode(),
        hashlib.sha256
    ).hexdigest()
    # 取前6位，转大写
    return digest[:6].upper()


def generate_init_password(uuid: str) -> str:
    """
    算法B：UUID → 6位初始密码
    使用 INIT_PWD_SALT 作为 HMAC 密钥

    :param uuid: ESP32 芯片 UUID
    :return: 6位大写字母+数字的初始密码
    """
    digest = hmac.new(
        settings.INIT_PWD_SALT.encode(),
        uuid.encode(),
        hashlib.sha256
    ).hexdigest()
    # 取6位，转大写
    return digest[:6].upper()


def generate_device_id() -> str:
    """
    生成随机 6 位设备短码（兼容旧代码）
    格式：K9X2M4 (不含分隔符存储)
    字符集：排除易混淆字符 (I, 1, O, 0)
    """
    chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"
    return "".join(random.choices(chars, k=6))
