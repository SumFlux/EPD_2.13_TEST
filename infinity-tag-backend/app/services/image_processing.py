import io
import numpy as np
from PIL import Image, ImageOps, ImageEnhance
from app.schemas.image import ImageProcessOptions, DitherAlgorithm

class ImageProcessor:
    """
    图片处理服务：处理用户上传的图片，适配 2.13寸 EPD (212x104)
    支持多种抖动算法：Floyd-Steinberg, Atkinson, Bayer, Threshold
    """

    TARGET_WIDTH = 212
    TARGET_HEIGHT = 104
    MAX_PIXELS = 4000000  # 限制最大像素 (2000x2000)，防止 DoS

    # Bayer 4x4 有序抖动矩阵
    # 用于将灰度图像转换为有序抖动的二值图像
    BAYER_MATRIX_SIZE: int = 4  # 4x4 矩阵
    BAYER_LEVELS: int = 16  # 4*4 = 16 个抖动级别
    BAYER_SCALE_FACTOR: float = 256.0 / 16  # 缩放因子，将 0-15 映射到 0-255 范围
    
    BAYER_MATRIX_4X4 = np.array([
        [ 0, 12,  3, 15],
        [ 8,  4, 11,  7],
        [ 2, 14,  1, 13],
        [10,  6,  9,  5]
    ]) * BAYER_SCALE_FACTOR  # 标准 Bayer 矩阵，缩放到 0-255 范围

    @classmethod
    def process_image(cls, image_bytes: bytes, options: ImageProcessOptions = None) -> bytes:
        """
        高级图片处理流程：
        1. 加载
        2. 旋转
        3. 裁剪 (Manual or Auto Aspect Fill)
        4. 缩放
        5. 预处理 (对比度, 锐化, Gamma, 反色)
        6. 二值化 (多种抖动算法可选)
        """
        # 输入验证
        if not image_bytes:
            raise ValueError("图片数据不能为空")
        
        if options is None:
            options = ImageProcessOptions()

        # 尝试加载图片，捕获无效格式异常
        try:
            img = Image.open(io.BytesIO(image_bytes))
            img.load()  # 强制加载图片数据，提前发现损坏的文件
        except Exception as e:
            raise ValueError(f"无效的图片格式: {str(e)}")

        # 安全检查: 防止 Decompression Bomb
        if img.width * img.height > cls.MAX_PIXELS:
            raise ValueError(f"图片尺寸过大 (最大允许 {cls.MAX_PIXELS} 像素)")

        # 1. 旋转
        if options.rotate in [90, 180, 270]:
            img = img.rotate(options.rotate, expand=True)

        # 2. 裁剪与缩放
        if options.crop_w and options.crop_h:
            img_w, img_h = img.size
            crop_x = max(0, options.crop_x or 0)
            crop_y = max(0, options.crop_y or 0)
            crop_w = min(options.crop_w, img_w - crop_x)
            crop_h = min(options.crop_h, img_h - crop_y)

            img = img.crop((crop_x, crop_y, crop_x + crop_w, crop_y + crop_h))
            img = img.resize((cls.TARGET_WIDTH, cls.TARGET_HEIGHT), Image.Resampling.LANCZOS)
        else:
            img = cls._aspect_fill(img, cls.TARGET_WIDTH, cls.TARGET_HEIGHT)

        # 3. 转灰度
        img = img.convert('L')

        # 4. 预处理增强 (关键步骤！)
        # 对比度增强
        if options.contrast != 1.0:
            enhancer = ImageEnhance.Contrast(img)
            img = enhancer.enhance(options.contrast)
        
        # 锐化增强
        if options.sharpness != 1.0:
            enhancer = ImageEnhance.Sharpness(img)
            img = enhancer.enhance(options.sharpness)
        
        # Gamma 校正
        if options.gamma != 1.0:
            img = img.point(lambda p: int(255 * ((p / 255) ** options.gamma)))

        # 5. 反色
        if options.invert:
            img = ImageOps.invert(img)

        # 6. 二值化 (根据选择的算法)
        img = cls._apply_dither(img, options)

        # 7. Save as PNG
        output = io.BytesIO()
        img.save(output, format="PNG")
        return output.getvalue()

    @classmethod
    def _apply_dither(cls, img: Image.Image, options: ImageProcessOptions) -> Image.Image:
        """根据选择的算法应用抖动"""
        algorithm: DitherAlgorithm = options.dither_algorithm
        threshold: int = options.threshold
        
        if algorithm == DitherAlgorithm.FLOYD_STEINBERG:
            # PIL 内置的 Floyd-Steinberg
            return img.convert('1')
        
        elif algorithm == DitherAlgorithm.ATKINSON:
            return cls._atkinson_dither(img)
        
        elif algorithm == DitherAlgorithm.BAYER:
            return cls._bayer_dither(img)
        
        elif algorithm == DitherAlgorithm.THRESHOLD:
            return img.point(lambda p: 255 if p > threshold else 0, mode='1')
        
        else:
            # 默认使用 Atkinson
            return cls._atkinson_dither(img)

    @classmethod
    def _atkinson_dither(cls, img: Image.Image) -> Image.Image:
        """
        Atkinson 抖动算法 (E-Ink 黄金标准)
        
        特点：只扩散 75% 的误差，丢弃 25%
        效果：高光区更纯白，阴影区更纯黑，过渡区保留抖动
        非常适合动漫/插画类图片
        """
        try:
            img_array = np.array(img, dtype=float)
            height, width = img_array.shape

            for y in range(height):
                for x in range(width):
                    old_pixel = img_array[y, x]
                    new_pixel = 255.0 if old_pixel > 128 else 0.0
                    img_array[y, x] = new_pixel
                    
                    error = old_pixel - new_pixel
                    
                    # Atkinson 误差扩散模式 (只扩散 6/8 = 75% 的误差)
                    #       X   1   1
                    #   1   1   1
                    #       1
                    # 除数为 8，但只有 6 个位置接收误差
                    
                    error_frac = error / 8.0
                    
                    if x + 1 < width:
                        img_array[y, x + 1] += error_frac
                    if x + 2 < width:
                        img_array[y, x + 2] += error_frac
                    if y + 1 < height:
                        if x > 0:
                            img_array[y + 1, x - 1] += error_frac
                        img_array[y + 1, x] += error_frac
                        if x + 1 < width:
                            img_array[y + 1, x + 1] += error_frac
                    if y + 2 < height:
                        img_array[y + 2, x] += error_frac

            # 确保值在有效范围内
            img_array = np.clip(img_array, 0, 255)
            return Image.fromarray(np.uint8(img_array)).convert('1')
        
        except MemoryError:
            raise ValueError("图片过大，Atkinson 抖动处理内存不足")
        except Exception as e:
            raise ValueError(f"Atkinson 抖动处理失败: {str(e)}")

    @classmethod
    def _bayer_dither(cls, img: Image.Image) -> Image.Image:
        """
        Bayer 4x4 有序抖动算法
        
        特点：使用固定阈值矩阵，产生规则的网点纹理
        效果：类似漫画网点或 Gameboy 屏幕的复古风格
        非常适合追求稳定、无随机噪点的场景
        """
        try:
            img_array = np.array(img, dtype=float)
            height, width = img_array.shape

            # 将 Bayer 矩阵铺满整张图
            tile_h = (height // 4) + 1
            tile_w = (width // 4) + 1
            threshold_map = np.tile(cls.BAYER_MATRIX_4X4, (tile_h, tile_w))[:height, :width]

            # 比对阈值：像素亮度 > 矩阵值则为白，否则为黑
            binary_array = np.where(img_array > threshold_map, 255, 0)

            return Image.fromarray(np.uint8(binary_array)).convert('1')
        
        except MemoryError:
            raise ValueError("图片过大，Bayer 抖动处理内存不足")
        except Exception as e:
            raise ValueError(f"Bayer 抖动处理失败: {str(e)}")

    @classmethod
    def _aspect_fill(cls, img: Image.Image, target_w: int, target_h: int) -> Image.Image:
        """等比缩放填充，并裁剪多余部分"""
        src_w, src_h = img.size

        ratio_w = target_w / src_w
        ratio_h = target_h / src_h
        ratio = max(ratio_w, ratio_h)

        new_w = int(src_w * ratio)
        new_h = int(src_h * ratio)

        img = img.resize((new_w, new_h), Image.Resampling.LANCZOS)

        left = (new_w - target_w) // 2
        top = (new_h - target_h) // 2
        right = left + target_w
        bottom = top + target_h

        return img.crop((left, top, right, bottom))

    @classmethod
    def get_epd_bitmap(cls, image_path: str) -> bytes:
        """
        读取处理好的图片文件，转换为 ESP32 可用的原始位图数据

        格式：212x104 像素，1-bit，按行对齐（Row-Aligned）
        - 每行 212 像素 = 27 字节（212/8=26.5，向上取整）
        - 总共 27 * 104 = 2808 字节
        - MSB first（位7是最左侧像素）
        - 0=黑色，1=白色
        
        重要：GxEPD2 的 drawBitmap 使用 Adafruit GFX 库，
        该库期望每行按字节对齐，不是紧密打包！
        """
        img = Image.open(image_path)
        img = img.convert('1')

        width, height = img.size

        if width != cls.TARGET_WIDTH or height != cls.TARGET_HEIGHT:
            raise ValueError(f"图片尺寸必须为 {cls.TARGET_WIDTH}x{cls.TARGET_HEIGHT}，当前为 {width}x{height}")

        pixels = img.load()
        
        bytes_per_row = (width + 7) // 8
        buffer = bytearray(bytes_per_row * height)

        for y in range(height):
            for x in range(width):
                pixel = pixels[x, y]
                if pixel > 127:
                    byte_index = y * bytes_per_row + (x // 8)
                    bit_pos = 7 - (x % 8)
                    buffer[byte_index] |= (1 << bit_pos)

        return bytes(buffer)
