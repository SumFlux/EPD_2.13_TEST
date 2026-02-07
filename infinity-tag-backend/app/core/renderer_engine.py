import os
from typing import Tuple, List, Optional
from PIL import Image, ImageDraw, ImageFont
from app.config import settings

class RendererEngine:
    """
    墨水屏渲染引擎
    适配 2.13寸 EPD (212x104)
    """

    # 屏幕参数
    WIDTH = 212
    HEIGHT = 104

    # 字体配置
    DEFAULT_FONT_NAME = "ChangBanDianSong-12.ttf"

    def __init__(self):
        # 创建白色背景画布 (mode='1' 为二值图像, 1=白, 0=黑)
        # 在 EPD 中，通常 0xFF 是白，0x00 是黑
        self.image = Image.new('1', (self.WIDTH, self.HEIGHT), 255)
        self.draw = ImageDraw.Draw(self.image)
        self.fonts = {}

        # 动态获取字体目录：从 settings 获取，避免硬编码相对路径
        self.font_dir = settings.FONT_DIR

    def _get_font(self, size: int, font_name: str = None) -> ImageFont.FreeTypeFont:
        """获取缓存的字体对象"""
        font_name = font_name or self.DEFAULT_FONT_NAME
        # 安全性: 只取文件名，防止路径遍历
        font_name = os.path.basename(font_name)

        key = f"{font_name}_{size}"

        if key not in self.fonts:
            font_path = os.path.join(self.font_dir, font_name)
            try:
                # 尝试加载指定字体
                font = ImageFont.truetype(font_path, size)
            except IOError:
                # 如果找不到，尝试系统字体或默认字体
                print(f"[WARN] Font not found: {font_path}, using default.")
                try:
                    # Linux/Docker 常用中文字体路径
                    font = ImageFont.truetype("/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf", size)
                except IOError:
                    # 最后的兜底
                    font = ImageFont.load_default()
            self.fonts[key] = font

        return self.fonts[key]

    def draw_text(
        self,
        xy: Tuple[int, int],
        text: str,
        size: int = 16,
        max_width: int = 0,
        align: str = "left",
        invert: bool = False
    ):
        """
        绘制文本 (支持自动换行)
        :param xy: 起始坐标 (x, y)
        :param text: 文本内容
        :param size: 字体大小
        :param max_width: 最大宽度，>0 则启用自动换行
        :param invert: 是否反色 (白底黑字 -> 黑底白字)
        """
        font = self._get_font(size)

        # 颜色: 0=黑, 255=白
        # fill_color = 255 if invert else 0

        if max_width > 0:
            lines = self._wrap_text(text, font, max_width)
        else:
            lines = [text]

        x, y = xy
        # 行高设定为字体大小的 1.2 倍
        line_height = int(size * 1.2)

        for line in lines:
            # 获取文本的边界框 (left, top, right, bottom)
            # 注意: textbbox 是 PIL 9.2.0+ 的方法
            if hasattr(self.draw, "textbbox"):
                bbox = self.draw.textbbox((x, y), line, font=font)
            else:
                # 兼容旧版本
                w, h = font.getsize(line)
                bbox = (x, y, x + w, y + h)

            # 简单的反色背景处理
            if invert:
                # 画黑底
                # 稍微扩大一点背景框，看起来更舒服
                padding = 1
                bg_bbox = (bbox[0], bbox[1], bbox[2], bbox[3] + padding)
                self.draw.rectangle(bg_bbox, fill=0)
                # 画白字
                self.draw.text((x, y), line, font=font, fill=255)
            else:
                # 画黑字
                self.draw.text((x, y), line, font=font, fill=0)

            y += line_height

    def _wrap_text(self, text: str, font: ImageFont.FreeTypeFont, max_width: int) -> List[str]:
        """简单的自动换行逻辑"""
        lines = []
        if not text:
            return lines

        current_line = ""
        for char in text:
            test_line = current_line + char
            # textlength 是 PIL 9.2+ 的方法
            if hasattr(self.draw, 'textlength'):
                w = self.draw.textlength(test_line, font=font)
            else:
                w = font.getsize(test_line)[0]

            if w <= max_width:
                current_line = test_line
            else:
                lines.append(current_line)
                current_line = char
        if current_line:
            lines.append(current_line)
        return lines

    def draw_line(self, start: Tuple[int, int], end: Tuple[int, int], width: int = 1):
        """绘制线条"""
        self.draw.line([start, end], fill=0, width=width)

    def draw_rectangle(self, xy: Tuple[int, int, int, int], fill: Optional[int] = None, outline: int = 0):
        """绘制矩形 xy=(left, top, right, bottom)"""
        self.draw.rectangle(xy, fill=fill, outline=outline)

    def get_image(self) -> Image.Image:
        """获取当前画布对象"""
        return self.image

    def get_bytes(self) -> bytes:
        """
        转换为 PNG 字节流
        """
        import io
        buf = io.BytesIO()
        self.image.save(buf, format="PNG")
        return buf.getvalue()
