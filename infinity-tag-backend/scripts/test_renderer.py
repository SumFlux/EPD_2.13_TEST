import asyncio
import os
import sys
from datetime import date, datetime

# 0. 预设 Mock 环境变量，绕过 Settings 验证
os.environ["MYSQL_PASSWORD"] = "mock_password"
os.environ["AI_API_KEY"] = "mock_key"
os.environ["JWT_SECRET_KEY"] = "mock_secret"
os.environ["MYSQL_HOST"] = "localhost"

# 添加项目根目录到 python path
project_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
sys.path.append(project_root)

from app.core.renderer_engine import RendererEngine
from app.services.renderer_service import RendererService
from app.models.almanac import AlmanacHistory

def test_renderer():
    print("Testing RendererEngine...")

    # 1. 模拟一个黄历对象
    mock_almanac = AlmanacHistory(
        user_id=1,
        date=date.today(),
        lunar_date="甲辰年正月初一",
        ganzhi_year="甲辰",
        ganzhi_month="丙寅",
        ganzhi_day="戊戌",
        favorable=["祭祀", "祈福", "求嗣", "开光", "出行"],
        unfavorable=["嫁娶", "移徙", "入宅", "开市", "交易"],
        lucky_direction="正南",
        lucky_item="水晶",
        energy_level=88,
        commentary="天行健，君子以自强不息。今日宜静思己过，勿急躁冒进，待时而动。",
        generated_at=datetime.now()
    )

    # 2. 渲染预览图 (PNG)
    print("Rendering PNG preview...")
    try:
        png_bytes = RendererService.render_almanac_card(mock_almanac)
    except Exception as e:
        print(f"Error rendering PNG: {e}")
        import traceback
        traceback.print_exc()
        return

    # Use project_root to define output directory correctly
    output_dir = os.path.join(project_root, "tests", "output")
    os.makedirs(output_dir, exist_ok=True)

    png_path = os.path.join(output_dir, "preview.png")
    with open(png_path, "wb") as f:
        f.write(png_bytes)
    print(f"Saved preview to {png_path}")

    # 3. 转换位图 (BIN)
    print("Converting to EPD bitmap...")
    try:
        bitmap_bytes = RendererService.convert_to_epd_bitmap(png_bytes)
    except Exception as e:
        print(f"Error converting to bitmap: {e}")
        return

    bin_path = os.path.join(output_dir, "almanac.bin")
    with open(bin_path, "wb") as f:
        f.write(bitmap_bytes)
    print(f"Saved bitmap to {bin_path}")

    print(f"Bitmap size: {len(bitmap_bytes)} bytes")
    # 212 pixels width -> 27 bytes per row (26.5 bytes padded to 27)
    # 27 * 104 = 2808 bytes
    print(f"Expected approx size: {27 * 104} bytes")

if __name__ == "__main__":
    test_renderer()
