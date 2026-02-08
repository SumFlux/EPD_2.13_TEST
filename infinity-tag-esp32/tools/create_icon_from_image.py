#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从图片生成卡片图标工具

功能：
1. 读取任意图片（PNG、JPG、BMP、SVG等）
2. 等比缩放到48x48像素
3. 转换为黑白单色
4. 生成预览图（放大8倍，方便查看）
5. 生成.bin文件（用于ESP32）

使用方法：
    python create_icon_from_image.py input.png output_name
    python create_icon_from_image.py input.svg output_name
    python create_icon_from_image.py --svg-code "SVG代码" output_name

依赖：
    pip install pillow cairosvg
"""

from PIL import Image, ImageDraw, ImageFont, ImageOps
import os
import sys
import tempfile

# SVG 支持库使用延迟导入，避免在处理PNG时导入失败
SVG_SUPPORT = None  # None表示未检测，True/False表示检测结果
SVG_METHOD = None


def svg_to_png(svg_input, output_size=512, is_file=True):
    """
    将SVG转换为PNG

    Args:
        svg_input: SVG文件路径或SVG代码字符串
        output_size: 输出PNG的尺寸
        is_file: True表示svg_input是文件路径，False表示是SVG代码

    Returns:
        PIL Image对象
    """
    global SVG_SUPPORT, SVG_METHOD

    # 延迟检测SVG支持（仅在需要时检测）
    if SVG_SUPPORT is None:
        SVG_SUPPORT = False

        # 优先尝试 svglib（Windows 友好）
        try:
            from svglib.svglib import svg2rlg
            from reportlab.graphics import renderPM
            SVG_SUPPORT = True
            SVG_METHOD = 'svglib'
        except (ImportError, OSError):
            pass

        # 备用方案：cairosvg（需要 GTK+ 运行时）
        if not SVG_SUPPORT:
            try:
                import cairosvg
                SVG_SUPPORT = True
                SVG_METHOD = 'cairosvg'
            except (ImportError, OSError):
                pass

    if not SVG_SUPPORT:
        print("❌ 错误: SVG支持未启用")
        print("   Windows用户需要安装 GTK+ 运行时")
        print("   下载地址: https://github.com/tschoonj/GTK-for-Windows-Runtime-Environment-Installer")
        return None

    try:
        if SVG_METHOD == 'svglib':
            from svglib.svglib import svg2rlg
            from reportlab.graphics import renderPM

            # 使用 svglib + reportlab（Windows 友好）
            if is_file:
                drawing = svg2rlg(svg_input)
            else:
                # svglib 需要文件路径，所以创建临时文件
                with tempfile.NamedTemporaryFile(mode='w', suffix='.svg', delete=False, encoding='utf-8') as tmp:
                    tmp.write(svg_input)
                    tmp_svg_path = tmp.name
                drawing = svg2rlg(tmp_svg_path)
                os.unlink(tmp_svg_path)

            if drawing is None:
                print("❌ 错误: SVG解析失败")
                return None

            # 缩放到目标尺寸
            scale_x = output_size / drawing.width
            scale_y = output_size / drawing.height
            scale = min(scale_x, scale_y)
            drawing.width = drawing.width * scale
            drawing.height = drawing.height * scale
            drawing.scale(scale, scale)

            # 渲染为PNG
            with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as tmp:
                tmp_path = tmp.name

            renderPM.drawToFile(drawing, tmp_path, fmt='PNG')

            # 读取PNG
            img = Image.open(tmp_path)
            img = img.copy()
            os.unlink(tmp_path)

            return img

        elif SVG_METHOD == 'cairosvg':
            import cairosvg

            # 使用 cairosvg（需要 GTK+ 运行时）
            with tempfile.NamedTemporaryFile(suffix='.png', delete=False) as tmp:
                tmp_path = tmp.name

            if is_file:
                cairosvg.svg2png(
                    url=svg_input,
                    write_to=tmp_path,
                    output_width=output_size,
                    output_height=output_size
                )
            else:
                cairosvg.svg2png(
                    bytestring=svg_input.encode('utf-8'),
                    write_to=tmp_path,
                    output_width=output_size,
                    output_height=output_size
                )

            img = Image.open(tmp_path)
            img = img.copy()
            os.unlink(tmp_path)

            return img

    except Exception as e:
        print(f"❌ 错误: SVG转换失败: {e}")
        import traceback
        traceback.print_exc()
        return None


def create_icon_from_image(input_path, output_name, size=48, threshold=128, add_border=True, svg_code=None):
    """
    从图片创建图标

    Args:
        input_path: 输入图片路径（如果svg_code不为None则忽略）
        output_name: 输出文件名（不含扩展名）
        size: 图标尺寸（默认48x48）
        threshold: 黑白阈值（0-255，默认128）
        add_border: 是否添加边框（默认True）
        svg_code: SVG代码字符串（可选）

    Returns:
        (bin_path, preview_path): 生成的文件路径
    """
    print(f"\n{'='*60}")
    print(f"从图片生成图标")
    print(f"{'='*60}")

    if svg_code:
        print(f"输入类型: SVG代码")
        print(f"SVG长度: {len(svg_code)} 字符")
    else:
        print(f"输入图片: {input_path}")

    print(f"输出名称: {output_name}")
    print(f"图标尺寸: {size}x{size}")
    print(f"黑白阈值: {threshold}")
    print(f"添加边框: {add_border}")
    print(f"{'='*60}\n")

    # 检测项目根目录（tools脚本需要向上一级）
    script_dir = os.path.dirname(os.path.abspath(__file__))
    if os.path.basename(script_dir) == 'tools':
        # 脚本在 tools/ 目录中，输出到上级目录的 data/icons/
        project_root = os.path.dirname(script_dir)
        icons_dir = os.path.join(project_root, 'data', 'icons')
        preview_dir = os.path.join(project_root, 'data', 'icons', 'preview')
    else:
        # 脚本在其他位置，输出到当前目录的 data/icons/
        icons_dir = 'data/icons'
        preview_dir = 'data/icons/preview'

    # 创建输出目录
    os.makedirs(icons_dir, exist_ok=True)
    os.makedirs(preview_dir, exist_ok=True)

    print(f"输出目录: {os.path.abspath(icons_dir)}\n")

    # 1. 读取图片
    print(f"[1/5] 读取图片...")

    if svg_code:
        # 从SVG代码生成
        img = svg_to_png(svg_code, output_size=512, is_file=False)
        if img is None:
            return None, None
        print(f"      SVG转换成功")
        print(f"      尺寸: {img.size[0]}x{img.size[1]}")
        print(f"      模式: {img.mode}")

    elif input_path.lower().endswith('.svg'):
        # 从SVG文件生成
        if not os.path.exists(input_path):
            print(f"❌ 错误: 输入文件不存在: {input_path}")
            return None, None

        img = svg_to_png(input_path, output_size=512, is_file=True)
        if img is None:
            return None, None
        print(f"      SVG文件转换成功")
        print(f"      尺寸: {img.size[0]}x{img.size[1]}")
        print(f"      模式: {img.mode}")

    else:
        # 从普通图片文件生成
        if not os.path.exists(input_path):
            print(f"❌ 错误: 输入文件不存在: {input_path}")
            return None, None

        try:
            img = Image.open(input_path)
            print(f"      原始尺寸: {img.size[0]}x{img.size[1]}")
            print(f"      原始模式: {img.mode}")
        except Exception as e:
            print(f"❌ 错误: 无法读取图片: {e}")
            return None, None

    # 2. 转换为RGBA（处理透明度）
    print(f"[2/5] 处理透明度...")
    if img.mode == 'RGBA':
        # 创建白色背景
        background = Image.new('RGB', img.size, (255, 255, 255))
        background.paste(img, mask=img.split()[3])  # 使用alpha通道作为mask
        img = background
    elif img.mode != 'RGB':
        img = img.convert('RGB')

    # 3. 等比缩放到目标尺寸
    print(f"[3/5] 等比缩放到 {size}x{size}...")

    # 计算缩放比例（保持宽高比）
    img.thumbnail((size, size), Image.Resampling.LANCZOS)

    # 创建白色背景，将缩放后的图片居中
    icon = Image.new('RGB', (size, size), (255, 255, 255))
    offset_x = (size - img.size[0]) // 2
    offset_y = (size - img.size[1]) // 2
    icon.paste(img, (offset_x, offset_y))

    print(f"      缩放后尺寸: {img.size[0]}x{img.size[1]}")
    print(f"      居中偏移: ({offset_x}, {offset_y})")

    # 4. 转换为黑白单色
    print(f"[4/5] 转换为黑白单色...")

    # 先转换为灰度
    icon_gray = icon.convert('L')

    # 应用阈值转换为1-bit黑白
    icon_bw = icon_gray.point(lambda x: 0 if x < threshold else 255, '1')

    # 添加边框（可选）
    if add_border:
        draw = ImageDraw.Draw(icon_bw)
        draw.rectangle([0, 0, size-1, size-1], outline=0, width=2)
        print(f"      已添加边框")

    # 5. 生成预览图（放大8倍）
    print(f"[5/5] 生成预览图和bin文件...")
    preview = icon_bw.resize((size * 8, size * 8), Image.NEAREST)
    preview_path = os.path.join(preview_dir, f'{output_name}_preview.png')
    preview.save(preview_path)
    print(f"      预览图: {preview_path}")

    # 6. 保存为二进制文件
    pixels = list(icon_bw.getdata())

    # 转换为字节数组（每8个像素一个字节）
    bytes_data = []
    for i in range(0, len(pixels), 8):
        byte = 0
        for j in range(8):
            if i + j < len(pixels):
                # 0=黑色，1=白色
                if pixels[i + j] == 0:
                    byte |= (1 << j)
        bytes_data.append(byte)

    # 写入bin文件
    bin_path = os.path.join(icons_dir, f'card_{output_name}.bin')
    with open(bin_path, 'wb') as f:
        f.write(bytes(bytes_data))

    print(f"      bin文件: {bin_path} ({len(bytes_data)} bytes)")

    print(f"\n{'='*60}")
    print(f"图标生成完成!")
    print(f"{'='*60}")
    print(f"预览图: {preview_path}")
    print(f"bin文件: {bin_path}")
    print(f"\n上传到设备:")
    print(f"  cd .. && pio run -t uploadfs")
    print(f"{'='*60}\n")

    return bin_path, preview_path


def show_usage():
    """显示使用说明"""
    svg_status = "✅ 已启用" if SVG_SUPPORT else "❌ 未启用 (需要: pip install cairosvg)"

    print(f"""
╔══════════════════════════════════════════════════════════════╗
║           从图片生成卡片图标工具                              ║
║           SVG支持: {svg_status:40s} ║
╚══════════════════════════════════════════════════════════════╝

使用方法:
  # 从图片文件生成
  python create_icon_from_image.py <输入图片> <输出名称> [选项]

  # 从SVG代码生成
  python create_icon_from_image.py --svg-code "<SVG代码>" <输出名称> [选项]

  # 从SVG文件生成
  python create_icon_from_image.py icon.svg <输出名称> [选项]

参数:
  输入图片    - 图片文件路径 (支持 PNG, JPG, BMP, SVG 等)
  输出名称    - 输出文件名 (不含扩展名)

选项:
  --svg-code "<SVG代码>"  直接使用SVG代码生成图标
  --size N                图标尺寸 (默认: 48)
  --threshold N           黑白阈值 0-255 (默认: 128, 越小越多黑色)
  --no-border             不添加边框

示例:
  # 从PNG生成
  python create_icon_from_image.py icon.png settings

  # 从SVG文件生成
  python create_icon_from_image.py icon.svg settings

  # 从SVG代码生成
  python create_icon_from_image.py --svg-code '<svg>...</svg>' settings

  # 从SVG代码生成（从文件读取）
  python create_icon_from_image.py --svg-code "$(cat icon.svg)" settings

  # 自定义阈值（更多黑色）
  python create_icon_from_image.py icon.svg settings --threshold 100

  # 自定义阈值（更多白色）
  python create_icon_from_image.py icon.svg settings --threshold 180

  # 不添加边框
  python create_icon_from_image.py icon.svg settings --no-border

输出文件:
  data/icons/card_<输出名称>.bin           - ESP32使用的二进制文件
  data/icons/preview/<输出名称>_preview.png - 预览图 (放大8倍)

SVG支持:
  如果要使用SVG功能，需要安装 cairosvg:
    pip install cairosvg

  注意: Windows用户可能还需要安装 GTK+ 运行时
        下载地址: https://github.com/tschoonj/GTK-for-Windows-Runtime-Environment-Installer

提示:
  - 建议使用高对比度的图片
  - 简单的图标效果最好
  - SVG图标会先转换为512x512的PNG，然后缩放到目标尺寸
  - 可以通过调整 threshold 参数来控制黑白效果
  - 预览图可以帮助你查看最终效果
""")


def main():
    """主函数"""
    # 解析命令行参数
    if len(sys.argv) < 2:
        show_usage()
        sys.exit(1)

    # 检查是否是帮助命令
    if sys.argv[1] in ['--help', '-h']:
        show_usage()
        sys.exit(0)

    # 解析参数
    svg_code = None
    input_path = None
    output_name = None

    # 检查是否使用 --svg-code
    if '--svg-code' in sys.argv:
        idx = sys.argv.index('--svg-code')
        if idx + 2 >= len(sys.argv):
            print("❌ 错误: --svg-code 需要提供 SVG代码 和 输出名称")
            show_usage()
            sys.exit(1)
        svg_code = sys.argv[idx + 1]
        output_name = sys.argv[idx + 2]
        # 移除这些参数
        sys.argv = sys.argv[:idx] + sys.argv[idx+3:]
    else:
        if len(sys.argv) < 3:
            show_usage()
            sys.exit(1)
        input_path = sys.argv[1]
        output_name = sys.argv[2]

    # 解析可选参数
    size = 48
    threshold = 128
    add_border = True

    i = 3 if not svg_code else 1
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == '--size' and i + 1 < len(sys.argv):
            size = int(sys.argv[i + 1])
            i += 2
        elif arg == '--threshold' and i + 1 < len(sys.argv):
            threshold = int(sys.argv[i + 1])
            i += 2
        elif arg == '--no-border':
            add_border = False
            i += 1
        else:
            print(f"❌ 未知参数: {arg}")
            show_usage()
            sys.exit(1)

    # 生成图标
    bin_path, preview_path = create_icon_from_image(
        input_path,
        output_name,
        size=size,
        threshold=threshold,
        add_border=add_border,
        svg_code=svg_code
    )

    if bin_path and preview_path:
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == '__main__':
    main()


def show_usage():
    """显示使用说明"""
    print("""
╔══════════════════════════════════════════════════════════════╗
║           从图片生成卡片图标工具                              ║
╚══════════════════════════════════════════════════════════════╝

使用方法:
  python create_icon_from_image.py <输入图片> <输出名称> [选项]

参数:
  输入图片    - 图片文件路径 (支持 PNG, JPG, BMP 等)
  输出名称    - 输出文件名 (不含扩展名)

选项:
  --size N        图标尺寸 (默认: 48)
  --threshold N   黑白阈值 0-255 (默认: 128, 越小越多黑色)
  --no-border     不添加边框

示例:
  # 基本使用
  python create_icon_from_image.py icon.png settings

  # 自定义阈值（更多黑色）
  python create_icon_from_image.py icon.png settings --threshold 100

  # 自定义阈值（更多白色）
  python create_icon_from_image.py icon.png settings --threshold 180

  # 不添加边框
  python create_icon_from_image.py icon.png settings --no-border

  # 自定义尺寸
  python create_icon_from_image.py icon.png settings --size 64

输出文件:
  data/icons/card_<输出名称>.bin           - ESP32使用的二进制文件
  data/icons/preview/<输出名称>_preview.png - 预览图 (放大8倍)

提示:
  - 建议使用高对比度的图片
  - 简单的图标效果最好
  - 可以通过调整 threshold 参数来控制黑白效果
  - 预览图可以帮助你查看最终效果
""")


def main():
    """主函数"""
    # 解析命令行参数
    if len(sys.argv) < 3:
        show_usage()
        sys.exit(1)

    input_path = sys.argv[1]
    output_name = sys.argv[2]

    # 解析可选参数
    size = 48
    threshold = 128
    add_border = True

    i = 3
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == '--size' and i + 1 < len(sys.argv):
            size = int(sys.argv[i + 1])
            i += 2
        elif arg == '--threshold' and i + 1 < len(sys.argv):
            threshold = int(sys.argv[i + 1])
            i += 2
        elif arg == '--no-border':
            add_border = False
            i += 1
        elif arg == '--help' or arg == '-h':
            show_usage()
            sys.exit(0)
        else:
            print(f"❌ 未知参数: {arg}")
            show_usage()
            sys.exit(1)

    # 生成图标
    bin_path, preview_path = create_icon_from_image(
        input_path,
        output_name,
        size=size,
        threshold=threshold,
        add_border=add_border
    )

    if bin_path and preview_path:
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == '__main__':
    main()
