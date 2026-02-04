import io
from PIL import Image, ImageOps
from app.schemas.image import ImageProcessOptions

class ImageProcessor:
    """
    图片处理服务：处理用户上传的图片，适配 2.13寸 EPD (250x122)
    """

    TARGET_WIDTH = 250
    TARGET_HEIGHT = 122
    MAX_PIXELS = 4000000  # 限制最大像素 (2000x2000)，防止 DoS

    @classmethod
    def process_image(cls, image_bytes: bytes, options: ImageProcessOptions = None) -> bytes:
        """
        高级图片处理流程：
        1. 加载
        2. 旋转
        3. 裁剪 (Manual or Auto Aspect Fill)
        4. 缩放
        5. 预处理 (反色)
        6. 二值化 (Dither or Threshold)
        """
        if options is None:
            options = ImageProcessOptions()

        img = Image.open(io.BytesIO(image_bytes))

        # 安全检查: 防止 Decompression Bomb
        if img.width * img.height > cls.MAX_PIXELS:
            raise ValueError(f"图片尺寸过大 (最大允许 {cls.MAX_PIXELS} 像素)")

        # 1. 旋转
        if options.rotate in [90, 180, 270]:
            img = img.rotate(options.rotate, expand=True)

        # 2. 裁剪与缩放
        if options.crop_w and options.crop_h:
            # 手动裁剪模式：先裁后缩放
            # 确保坐标在图片范围内
            img_w, img_h = img.size
            crop_x = max(0, options.crop_x or 0)
            crop_y = max(0, options.crop_y or 0)
            crop_w = min(options.crop_w, img_w - crop_x)
            crop_h = min(options.crop_h, img_h - crop_y)

            img = img.crop((crop_x, crop_y, crop_x + crop_w, crop_y + crop_h))
            # 强制缩放到目标尺寸
            img = img.resize((cls.TARGET_WIDTH, cls.TARGET_HEIGHT), Image.Resampling.LANCZOS)
        else:
            # 自动模式：Aspect Fill
            img = cls._aspect_fill(img, cls.TARGET_WIDTH, cls.TARGET_HEIGHT)

        # 3. 转灰度
        img = img.convert('L')

        # 4. 反色
        if options.invert:
            img = ImageOps.invert(img)

        # 5. 二值化
        if options.dither:
            # Floyd-Steinberg Dithering
            img = img.convert('1')
        else:
            # Thresholding
            # point 方法：如果像素值 > threshold 则为 255 (白)，否则 0 (黑)
            threshold = options.threshold
            img = img.point(lambda p: 255 if p > threshold else 0, mode='1')

        # 6. Save as PNG
        output = io.BytesIO()
        img.save(output, format="PNG")
        return output.getvalue()

    @classmethod
    def _aspect_fill(cls, img: Image.Image, target_w: int, target_h: int) -> Image.Image:
        """
        等比缩放填充，并裁剪多余部分
        """
        src_w, src_h = img.size

        # 计算缩放比例
        ratio_w = target_w / src_w
        ratio_h = target_h / src_h
        ratio = max(ratio_w, ratio_h)

        new_w = int(src_w * ratio)
        new_h = int(src_h * ratio)

        # 缩放
        img = img.resize((new_w, new_h), Image.Resampling.LANCZOS)

        # 裁剪中心
        left = (new_w - target_w) // 2
        top = (new_h - target_h) // 2
        right = left + target_w
        bottom = top + target_h

        return img.crop((left, top, right, bottom))

    @classmethod
    def get_epd_bitmap(cls, image_path: str) -> bytes:
        """
        读取处理好的图片文件，转换为 ESP32 可用的原始位图数据
        """
        img = Image.open(image_path)
        img = img.convert('1')

        width, height = img.size
        pixels = img.load()
        buffer = bytearray()

        for y in range(height):
            byte_val = 0
            for x in range(width):
                pixel = pixels[x, y]
                # 0=Black, 255=White. EPD: 0=Black, 1=White
                bit = 1 if pixel > 127 else 0

                # Packing: MSB first
                bit_pos = 7 - (x % 8)
                if bit:
                    byte_val |= (1 << bit_pos)

                if (x % 8) == 7:
                    buffer.append(byte_val)
                    byte_val = 0

            # Handle padding if width is not multiple of 8
            if (width % 8) != 0:
                buffer.append(byte_val)

        return bytes(buffer)
