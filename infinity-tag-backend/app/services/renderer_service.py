from PIL import Image
from app.core.renderer_engine import RendererEngine
from app.models.almanac import AlmanacHistory
import io

class RendererService:
    """渲染业务服务"""

    @staticmethod
    def render_almanac_card(almanac: AlmanacHistory) -> bytes:
        """
        渲染黄历卡片 (250x122)
        优化版布局:
        |-----------------------------------|
        | 2026-02-04 正月初一 [丙午 庚寅 癸未] | (Header + Ganzhi)
        |-----------------------------------|
        |  [宜] 祭祀 祈福   |  [忌] 嫁娶 移徙  | (Split Columns)
        |  求嗣 开光 ...    |  入宅 开市 ...   |
        |-----------------------------------|
        |  "今日运势平稳..."                 | (Commentary, Max Space)
        |-----------------------------------|
        """
        engine = RendererEngine()

        # 1. Header: 公历 + 农历 + 干支 (合并显示)
        header_bg_height = 24
        engine.draw_rectangle((0, 0, engine.WIDTH, header_bg_height), fill=0) # 黑底

        date_str = almanac.date.strftime("%Y-%m-%d")
        # week_days = ["周一","周二","周三","周四","周五","周六","周日"]
        # week_day = week_days[almanac.date.weekday()]

        # 提取农历日 (去除年份，如 "甲辰年正月初一" -> "正月初一")
        lunar_full = almanac.lunar_date
        if "年" in lunar_full:
            lunar_short = lunar_full.split("年")[1]
        else:
            lunar_short = lunar_full

        # 干支精简: 丙午 庚寅 癸未 (去掉了年/月/日后缀)
        ganzhi_short = f"{almanac.ganzhi_year} {almanac.ganzhi_month} {almanac.ganzhi_day}"

        # 组合 Header 文本
        # 格式: 2026-02-04 正月初一 [丙午 庚寅 癸未]
        # 注意: 宽度有限，如果太长需要截断
        header_text = f"{date_str} {lunar_short} [{ganzhi_short}]"

        # 使用较小字体以容纳更多内容，或者自适应
        engine.draw_text((5, 4), header_text, size=12, invert=True) # 白字

        # 2. 宜忌 (Split Columns)
        # 上移起始 Y 坐标
        body_y_start = 28

        # 左右分栏
        col_width = engine.WIDTH // 2

        # 左栏: 宜
        favorable_list = almanac.favorable if almanac.favorable else []
        yi_title = "【宜】"
        engine.draw_text((5, body_y_start), yi_title, size=12)

        # 宜的内容换行显示
        yi_content = " ".join(favorable_list[:6]) # 取多一点
        engine.draw_text((5, body_y_start + 14), yi_content, size=12, max_width=col_width - 10)

        # 中轴线 (可选)
        # engine.draw_line((col_width, body_y_start), (col_width, 80))

        # 右栏: 忌
        unfavorable_list = almanac.unfavorable if almanac.unfavorable else []
        ji_title = "【忌】"
        engine.draw_text((col_width + 5, body_y_start), ji_title, size=12)

        ji_content = " ".join(unfavorable_list[:6])
        engine.draw_text((col_width + 5, body_y_start + 14), ji_content, size=12, max_width=col_width - 10)

        # 3. 批注与运势 (Bottom Area)
        # 给批注留出更多空间，从 y=75 开始 (总高 122)
        commentary_y = 75
        engine.draw_line((0, commentary_y), (engine.WIDTH, commentary_y))

        # 财神/幸运物 (一行显示)
        lucky_info = f"财神:{almanac.lucky_direction}  物:{almanac.lucky_item}  指数:{almanac.energy_level}"
        engine.draw_text((5, commentary_y + 2), lucky_info, size=12)

        # 批注内容 (剩余空间全部给它)
        comment = almanac.commentary or ""
        # 允许显示 2-3 行
        engine.draw_text((5, commentary_y + 16), comment, size=12, max_width=240)

        return engine.get_bytes()

    @staticmethod
    def convert_to_epd_bitmap(png_bytes: bytes) -> bytes:
        """
        将 PNG 转换为 ESP32 驱动所需的原始位图数据

        格式：212x104 像素，1-bit，按行对齐（Row-Aligned）
        - 每行 212 像素 = 27 字节（(212+7)/8，向上取整）
        - 总共 27 * 104 = 2808 字节
        - MSB first（位7是最左侧像素）
        - 0=黑色，1=白色
        
        重要：GxEPD2 的 drawBitmap 使用 Adafruit GFX 库，
        该库期望每行按字节对齐，不是紧密打包！
        """
        img = Image.open(io.BytesIO(png_bytes))
        img = img.convert('1')

        width, height = img.size

        # 验证尺寸
        if width != 212 or height != 104:
            raise ValueError(f"图片尺寸必须为 212x104，当前为 {width}x{height}")

        pixels = img.load()
        
        # 行对齐格式：每行占用 (width + 7) // 8 字节
        bytes_per_row = (width + 7) // 8  # 27 bytes for 212 pixels
        buffer = bytearray(bytes_per_row * height)  # 2808 bytes total

        for y in range(height):
            for x in range(width):
                pixel = pixels[x, y]
                # PIL '1' mode: 0=black, 255=white
                # EPD: 0=black, 1=white
                if pixel > 127:  # White pixel
                    byte_index = y * bytes_per_row + (x // 8)
                    bit_pos = 7 - (x % 8)
                    buffer[byte_index] |= (1 << bit_pos)

        return bytes(buffer)
