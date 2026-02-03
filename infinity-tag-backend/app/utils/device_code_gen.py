import random
import string

def generate_device_id() -> str:
    """
    生成 6 位设备短码
    格式：K9X-2M4 (不含分隔符存储)
    字符集：排除易混淆字符 (I, 1, O, 0)
    """
    chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"
    return "".join(random.choices(chars, k=6))
