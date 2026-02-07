#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
TTF 字体转换工具
将 TTF 字体转换为 U8g2 格式的 C 数组

使用方法：
    python convert_font.py

依赖：
    pip install pillow
"""

from PIL import Image, ImageDraw, ImageFont
import os

# 配置
TTF_PATH = "../ttf/匯文仿宋.ttf"
OUTPUT_PATH = "../include/Fonts/HuiwenFangsong.h"
FONT_SIZE = 12  # 字体大小（像素）

# 需要转换的汉字（保持原始顺序，并在后面进行扩展）
CHINESE_CHARS = [
    # --- 原始字符 (保持不变，共 88 个) ---
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

    # --- 扩展：交互与状态 (UI Interaction) ---
    "入", "出", "进", "行", "作", "业", "完", "成", "等", "待", 
    "停", "止", "暂", "停", "继", "续", "重", "试", "刷", "新", 
    "删", "除", "编", "辑", "修", "改", "保", "存", "添", "加", 
    "搜", "索", "查", "看", "详", "情", "清", "空", "全", "选", 
    "上", "下", "左", "右", "前", "后", "内", "外", "里", "面",

    # --- 扩展：设备与硬件 (Hardware & System) ---
    "电", "量", "充", "池", "压", "流", "温", "度", "湿", "亮", 
    "屏", "幕", "模", "式", "蓝", "牙", "无", "线", "信", "号", 
    "强", "弱", "感", "应", "触", "摸", "存", "储", "空", "间", 
    "总", "额", "剩", "余", "状", "态", "警", "报", "告", "警", 
    "异", "常", "错", "误", "故", "障", "正", "常", "提", "示",

    # --- 扩展：数值、时间与单位 (Math & Time) ---
    "一", "三", "四", "五", "六", "七", "八", "九", "十", "百", 
    "千", "万", "点", "个", "只", "项", "页", "码", "年", "月", 
    "日", "时", "分", "秒", "周", "天", "早", "晚", "午", "前", 
    "今", "昨", "明", "后", "期", "间", "长", "短", "高", "低",

    # --- 扩展：通用高频词汇 (Commonly Used) ---
    "是", "否", "好", "的", "不", "要", "和", "与", "或", "者", 
    "能", "可", "以", "用", "于", "对", "象", "名", "称", "用", 
    "户", "主", "页", "菜单", "标", "题", "内", "容", "说", "明", 
    "使", "用", "方", "法", "帮", "助", "关", "于", "我", "你", 
    "他", "多", "少", "合", "格", "安", "全", "重", "要", "其"
]

# 去重并排序
CHINESE_CHARS = sorted(list(set(CHINESE_CHARS)))

def char_to_bitmap(font, char):
    """将单个字符转换为位图"""
    # 创建临时图像
    img = Image.new('1', (FONT_SIZE, FONT_SIZE), 1)
    draw = ImageDraw.Draw(img)

    # 绘制字符
    draw.text((0, 0), char, font=font, fill=0)

    # 转换为位数组
    pixels = list(img.getdata())

    # 转换为字节数组（每8个像素一个字节）
    bytes_data = []
    for y in range(FONT_SIZE):
        for x in range(0, FONT_SIZE, 8):
            byte = 0
            for bit in range(8):
                if x + bit < FONT_SIZE:
                    pixel_index = y * FONT_SIZE + x + bit
                    if pixels[pixel_index] == 0:  # 黑色
                        byte |= (1 << (7 - bit))
            bytes_data.append(byte)

    return bytes_data

def generate_font_header():
    """生成字体头文件"""
    print(f"正在加载字体: {TTF_PATH}")

    # 加载 TTF 字体
    font = ImageFont.truetype(TTF_PATH, FONT_SIZE)

    # 生成头文件内容
    header = f"""#ifndef HUIWEN_FANGSONG_H
#define HUIWEN_FANGSONG_H

#include <Arduino.h>

// 匯文仿宋字体 {FONT_SIZE}px
// 包含 {len(CHINESE_CHARS)} 个常用汉字

struct ChineseChar {{
    const char* utf8;      // UTF-8 编码
    const uint8_t* bitmap; // 位图数据 ({FONT_SIZE}x{FONT_SIZE})
}};

"""

    # 生成每个字符的位图数据
    bitmap_arrays = []
    char_mappings = []

    for i, char in enumerate(CHINESE_CHARS):
        print(f"转换字符: {char} ({i+1}/{len(CHINESE_CHARS)})")

        # 获取位图数据
        bitmap = char_to_bitmap(font, char)

        # 生成 C 数组
        array_name = f"bitmap_{i}"
        bitmap_str = ", ".join([f"0x{b:02X}" for b in bitmap])
        bitmap_arrays.append(f"static const uint8_t {array_name}[] = {{ {bitmap_str} }};")

        # UTF-8 编码
        utf8_bytes = char.encode('utf-8')
        utf8_str = "\\x" + "\\x".join([f"{b:02x}" for b in utf8_bytes])

        char_mappings.append(f'    {{ "{utf8_str}", {array_name} }}')

    # 组合头文件
    header += "\n".join(bitmap_arrays)
    header += "\n\n"
    header += f"static const ChineseChar HUIWEN_FANGSONG_CHARS[{len(CHINESE_CHARS)}] = {{\n"
    header += ",\n".join(char_mappings)
    header += "\n};\n\n"

    # 添加查找函数
    header += f"""
// 查找字符的位图数据
static inline const uint8_t* findChineseBitmap(const char* utf8Char) {{
    for (int i = 0; i < {len(CHINESE_CHARS)}; i++) {{
        if (strcmp(HUIWEN_FANGSONG_CHARS[i].utf8, utf8Char) == 0) {{
            return HUIWEN_FANGSONG_CHARS[i].bitmap;
        }}
    }}
    return nullptr;
}}

#endif // HUIWEN_FANGSONG_H
"""

    # 写入文件
    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    with open(OUTPUT_PATH, 'w', encoding='utf-8') as f:
        f.write(header)

    print(f"\nFont conversion completed!")
    print(f"Output: {OUTPUT_PATH}")
    print(f"Font size: {FONT_SIZE}px")
    print(f"Char count: {len(CHINESE_CHARS)}")
    print(f"Total size: {len(CHINESE_CHARS) * FONT_SIZE * FONT_SIZE // 8} bytes")

if __name__ == "__main__":
    generate_font_header()
