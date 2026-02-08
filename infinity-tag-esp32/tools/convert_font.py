#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
TTF 字体转换工具（专业版）
将 TTF 字体转换为 U8g2 格式的 C 数组，支持基线对齐

特性：
- 基线对齐（使用字体 ascent/descent 信息）
- 自动读取 txt 字库文件
- 支持多字库合并
- 生成调试预览图

使用方法：
    python convert_font.py

依赖：
    pip install pillow
"""

from PIL import Image, ImageDraw, ImageFont
import os
import sys

# ==================== 配置区 ====================

# 字体文件路径
TTF_PATH = "../ttf/RenOuFangSong-16.ttf"

# 输出文件路径
OUTPUT_PATH = "../include/Fonts/HuiwenFangsong.h"

# 字体大小（像素）
FONT_SIZE = 16

# 字库文件列表（相对于脚本的路径）
CHARSET_FILES = [
    "../ttf/常用字/level-1.txt",  # 一级常用字
    # "../ttf/常用字/level-2.txt",  # 二级常用字（可选）
    # "../ttf/常用字/level-3.txt",  # 三级常用字（可选）
]

# 额外的自定义字符（会与字库文件合并）
EXTRA_CHARS = [
    # UI 相关
    "设", "置", "系", "统", "配", "网", "固", "件", "信", "息",
    "检", "查", "更", "新", "恢", "复", "出", "厂", "声", "音",
    "开", "关", "版", "本", "确", "认", "取", "消", "成", "功",
    "失", "败", "连", "接", "断", "开", "正", "在", "加", "载",
    "请", "稍", "候", "分", "类", "卡", "片", "选", "择", "返", "回",
    "已", "最", "发", "现", "大", "小", "密", "码", "扫", "描",
    "二", "维", "码", "启", "动", "步", "骤", "初", "始", "化",
    "硬", "文", "核", "心", "络", "就", "绪", "离", "线", "所",
    "有", "数", "据", "将", "被", "清", "除", "中", "按", "键",
    "长", "无", "构", "建",
]

# 英文字母、数字、常用符号（12x12 位图）
ASCII_CHARS = [
    # 数字
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    # 大写字母
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
    # 小写字母
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
    # 常用符号
    " ", ".", ",", ":", ";", "!", "?", "-", "_", "/", "\\",
    "(", ")", "[", "]", "{", "}", "<", ">",
    "+", "=", "*", "&", "%", "$", "#", "@",
    "'", "\"", "`", "~", "|",
]

# 是否生成调试预览图（保存到 debug/ 目录）//True
GENERATE_DEBUG_IMAGES = False

# 是否显示详细日志
VERBOSE = False

# ==================== 核心函数 ====================

def load_charset_from_files(file_paths):
    """从多个 txt 文件加载字符集"""
    chars = set()

    for file_path in file_paths:
        if not os.path.exists(file_path):
            print(f"[WARNING] 字库文件不存在: {file_path}")
            continue

        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
                # 每行一个字符，去除空白
                for line in content.splitlines():
                    char = line.strip()
                    if char and len(char) == 1:  # 只保留单个字符
                        chars.add(char)

            print(f"[OK] 加载字库: {file_path} ({len(chars)} 字符)")

        except Exception as e:
            print(f"[ERROR] 读取字库文件失败: {file_path}")
            print(f"        错误: {e}")

    return chars


def char_to_bitmap_baseline_aligned(font, char, font_size):
    """
    将单个字符转换为位图（使用基线对齐）

    原理：
    1. 获取字体的 ascent（基线到顶部）和 descent（基线到底部）
    2. 计算字符的实际边界框
    3. 将字符对齐到统一的基线位置
    """

    # 获取字体度量信息
    ascent, descent = font.getmetrics()

    # 创建临时图像用于测量字符边界
    temp_img = Image.new('1', (font_size * 3, font_size * 3), 1)
    temp_draw = ImageDraw.Draw(temp_img)

    # 获取字符的实际边界框 (left, top, right, bottom)
    bbox = temp_draw.textbbox((0, 0), char, font=font)
    char_width = bbox[2] - bbox[0]
    char_height = bbox[3] - bbox[1]

    # 计算基线对齐的偏移
    # 水平居中
    offset_x = (font_size - char_width) // 2 - bbox[0]

    # 垂直对齐：确保字符完整显示
    # 优先保证字符不被截断，而不是严格对齐基线
    offset_y = -bbox[1]  # 从字符顶部开始绘制

    # 如果字符高度超过画布，缩放调整（但这不应该发生在16px字体）
    if char_height > font_size:
        # 字符太高，需要向上移动以显示更多内容
        # 优先保留字符的主体部分（上部和中部）
        offset_y = max(0, font_size - char_height)
    elif offset_y < 0:
        offset_y = 0
    elif offset_y + char_height > font_size:
        # 字符会超出底部，向上调整
        offset_y = font_size - char_height

    # 创建最终图像
    img = Image.new('1', (font_size, font_size), 1)
    draw = ImageDraw.Draw(img)

    # 绘制字符（基线对齐）
    draw.text((offset_x, offset_y), char, font=font, fill=0)

    # 转换为位数组
    pixels = list(img.getdata())

    # 转换为字节数组（每8个像素一个字节，按行扫描）
    bytes_data = []
    for y in range(font_size):
        for x in range(0, font_size, 8):
            byte = 0
            for bit in range(8):
                if x + bit < font_size:
                    pixel_index = y * font_size + x + bit
                    if pixels[pixel_index] == 0:  # 黑色像素
                        byte |= (1 << (7 - bit))
            bytes_data.append(byte)

    return bytes_data, img


def generate_font_header(chars, font_path, output_path, font_size, generate_debug=False):
    """生成字体头文件"""

    print(f"\n{'='*60}")
    print(f"开始转换字体")
    print(f"{'='*60}")
    print(f"字体文件: {font_path}")
    print(f"输出文件: {output_path}")
    print(f"字体大小: {font_size}px")
    print(f"字符数量: {len(chars)}")
    print(f"{'='*60}\n")

    # 加载 TTF 字体
    try:
        font = ImageFont.truetype(font_path, font_size)
    except Exception as e:
        print(f"[ERROR] 加载字体失败: {e}")
        sys.exit(1)

    # 获取字体度量信息
    ascent, descent = font.getmetrics()
    print(f"字体度量: ascent={ascent}, descent={descent}")

    # 创建调试目录
    if generate_debug:
        debug_dir = "debug"
        os.makedirs(debug_dir, exist_ok=True)
        print(f"调试图像将保存到: {debug_dir}/\n")

    # 生成头文件头部
    header = f"""#ifndef HUIWEN_FANGSONG_H
#define HUIWEN_FANGSONG_H

#include <Arduino.h>

// 匯文仿宋字体 {font_size}px (基线对齐)
// 字符数量: {len(chars)}
// 字体度量: ascent={ascent}, descent={descent}
// 生成时间: {__import__('datetime').datetime.now().strftime('%Y-%m-%d %H:%M:%S')}

struct ChineseChar {{
    const char* utf8;      // UTF-8 编码
    const uint8_t* bitmap; // 位图数据 ({font_size}x{font_size}, 每行 {font_size//8} 字节)
}};

"""

    # 生成每个字符的位图数据
    bitmap_arrays = []
    char_mappings = []

    for i, char in enumerate(chars):
        if VERBOSE:
            print(f"[{i+1}/{len(chars)}] 转换字符: {char}")

        # 获取位图数据
        bitmap, img = char_to_bitmap_baseline_aligned(font, char, font_size)

        # 生成 C 数组
        array_name = f"bitmap_{i}"
        bitmap_str = ", ".join([f"0x{b:02X}" for b in bitmap])
        bitmap_arrays.append(f"static const uint8_t {array_name}[] = {{ {bitmap_str} }};")

        # UTF-8 编码
        utf8_bytes = char.encode('utf-8')
        utf8_str = "\\x" + "\\x".join([f"{b:02x}" for b in utf8_bytes])

        # 注意：不在注释中添加逗号，避免 C++ 语法错误
        char_mappings.append(f'    {{ "{utf8_str}", {array_name} }}  // {char}')

        # 保存调试图像
        if generate_debug:
            # 清理文件名中的非法字符
            safe_char = char
            # Windows 文件系统非法字符: \ / : * ? " < > |
            illegal_chars = ['\\', '/', ':', '*', '?', '"', '<', '>', '|']
            for illegal in illegal_chars:
                safe_char = safe_char.replace(illegal, '_')

            debug_path = os.path.join(debug_dir, f"{i:04d}_{safe_char}.png")
            # 放大图像以便查看
            debug_img = img.resize((font_size * 8, font_size * 8), Image.NEAREST)
            debug_img.save(debug_path)

    # 组合头文件
    header += "\n".join(bitmap_arrays)
    header += "\n\n"
    header += f"static const ChineseChar HUIWEN_FANGSONG_CHARS[{len(chars)}] = {{\n"

    # 添加字符映射，除了最后一个元素，其他都在 } 后面加逗号
    for i, mapping in enumerate(char_mappings):
        if i < len(char_mappings) - 1:
            # 在 } 后面、注释前面插入逗号
            header += mapping.replace(" }  //", " },  //") + "\n"
        else:
            # 最后一个元素不加逗号
            header += mapping + "\n"

    header += "};\n\n"

    # 添加查找函数
    header += f"""
// 查找字符的位图数据
// 参数: utf8Char - UTF-8 编码的字符（3字节）
// 返回: 位图数据指针，如果未找到返回 nullptr
static inline const uint8_t* findChineseBitmap(const char* utf8Char) {{
    for (int i = 0; i < {len(chars)}; i++) {{
        if (strcmp(HUIWEN_FANGSONG_CHARS[i].utf8, utf8Char) == 0) {{
            return HUIWEN_FANGSONG_CHARS[i].bitmap;
        }}
    }}
    return nullptr;
}}

// 获取字符总数
static inline int getChineseCharCount() {{
    return {len(chars)};
}}

#endif // HUIWEN_FANGSONG_H
"""

    # 写入文件
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(header)

    # 输出统计信息
    print(f"\n{'='*60}")
    print(f"[OK] 字体转换完成!")
    print(f"{'='*60}")
    print(f"输出文件: {output_path}")
    print(f"字体大小: {font_size}px")
    print(f"字符数量: {len(chars)}")
    print(f"总大小: {len(chars) * font_size * font_size // 8} 字节 ({len(chars) * font_size * font_size // 8 / 1024:.2f} KB)")
    print(f"{'='*60}\n")


def main():
    """主函数"""

    # 1. 从字库文件加载字符
    chars_from_files = load_charset_from_files(CHARSET_FILES)

    # 2. 合并额外字符和ASCII字符
    all_chars = chars_from_files.union(set(EXTRA_CHARS)).union(set(ASCII_CHARS))

    # 3. 去重并排序
    sorted_chars = sorted(list(all_chars))

    print(f"\n字符统计:")
    print(f"  - 从字库文件加载: {len(chars_from_files)} 字符")
    print(f"  - 额外自定义字符: {len(EXTRA_CHARS)} 字符")
    print(f"  - ASCII字符: {len(ASCII_CHARS)} 字符")
    print(f"  - 去重后总计: {len(sorted_chars)} 字符")

    if len(sorted_chars) == 0:
        print("\n[ERROR] 没有可转换的字符!")
        print("        请检查字库文件路径或添加自定义字符。")
        sys.exit(1)

    # 4. 生成字体头文件
    generate_font_header(
        chars=sorted_chars,
        font_path=TTF_PATH,
        output_path=OUTPUT_PATH,
        font_size=FONT_SIZE,
        generate_debug=GENERATE_DEBUG_IMAGES
    )


if __name__ == "__main__":
    main()
