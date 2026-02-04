"""
十神关系计算引擎
基于日主天干与其他天干的五行生克关系计算十神

十神定义：
- 比肩(BI_JIAN): 同我同阴阳
- 劫财(JIE_CAI): 同我异阴阳
- 食神(SHI_SHEN): 我生同阴阳
- 伤官(SHANG_GUAN): 我生异阴阳
- 正财(ZHENG_CAI): 我克异阴阳
- 偏财(PIAN_CAI): 我克同阴阳
- 正官(ZHENG_GUAN): 克我异阴阳
- 七杀(QI_SHA): 克我同阴阳
- 正印(ZHENG_YIN): 生我异阴阳
- 偏印(PIAN_YIN): 生我同阴阳
"""
from typing import Dict

# 天干
TIAN_GAN = ["甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"]

# 天干对应五行索引: 0=木, 1=火, 2=土, 3=金, 4=水
TIAN_GAN_WU_XING = [0, 0, 1, 1, 2, 2, 3, 3, 4, 4]

# 五行名称
WU_XING_NAMES = ["木", "火", "土", "金", "水"]

# 天干阴阳: 0=阳, 1=阴
TIAN_GAN_YIN_YANG = [0, 1, 0, 1, 0, 1, 0, 1, 0, 1]

# 五行生克关系
# 生: 木->火->土->金->水->木 (索引关系: 当前生下一个)
# 克: 木->土->水->火->金->木 (索引关系: 隔一个克)
WU_XING_SHENG = {0: 1, 1: 2, 2: 3, 3: 4, 4: 0}  # 我生谁
WU_XING_KE = {0: 2, 1: 3, 2: 4, 3: 0, 4: 1}     # 我克谁

# 反向关系
WU_XING_SHENG_WO = {1: 0, 2: 1, 3: 2, 4: 3, 0: 4}  # 谁生我
WU_XING_KE_WO = {2: 0, 3: 1, 4: 2, 0: 3, 1: 4}     # 谁克我


def get_tian_gan_index(gan: str) -> int:
    """获取天干索引"""
    if gan in TIAN_GAN:
        return TIAN_GAN.index(gan)
    raise ValueError(f"无效天干: {gan}")


def get_wu_xing(gan: str) -> int:
    """获取天干对应的五行索引"""
    idx = get_tian_gan_index(gan)
    return TIAN_GAN_WU_XING[idx]


def get_wu_xing_name(gan: str) -> str:
    """获取天干对应的五行名称"""
    return WU_XING_NAMES[get_wu_xing(gan)]


def get_yin_yang(gan: str) -> int:
    """获取天干阴阳 (0=阳, 1=阴)"""
    idx = get_tian_gan_index(gan)
    return TIAN_GAN_YIN_YANG[idx]


def is_same_yin_yang(gan1: str, gan2: str) -> bool:
    """判断两天干阴阳是否相同"""
    return get_yin_yang(gan1) == get_yin_yang(gan2)


def get_relation(day_master_wx: int, target_wx: int) -> str:
    """
    获取五行生克关系
    :param day_master_wx: 日主五行索引
    :param target_wx: 目标五行索引
    :return: 关系类型 ('同我', '我生', '我克', '克我', '生我')
    """
    if day_master_wx == target_wx:
        return "同我"
    if WU_XING_SHENG[day_master_wx] == target_wx:
        return "我生"
    if WU_XING_KE[day_master_wx] == target_wx:
        return "我克"
    if WU_XING_SHENG_WO[day_master_wx] == target_wx:
        return "生我"
    if WU_XING_KE_WO[day_master_wx] == target_wx:
        return "克我"
    return "未知"


def calculate_ten_god(day_master: str, target_gan: str) -> str:
    """
    计算日主与目标天干的十神关系

    :param day_master: 日主天干 (如 "甲")
    :param target_gan: 目标天干 (如 "丙")
    :return: 十神名称 (如 "食神")
    """
    # 提取天干（支持传入完整干支如"甲子"，只取天干）
    if len(day_master) > 1:
        day_master = day_master[0]
    if len(target_gan) > 1:
        target_gan = target_gan[0]

    dm_wx = get_wu_xing(day_master)
    tg_wx = get_wu_xing(target_gan)
    same_yy = is_same_yin_yang(day_master, target_gan)

    relation = get_relation(dm_wx, tg_wx)

    # 根据生克关系和阴阳确定十神
    ten_god_map = {
        ("同我", True): "比肩",
        ("同我", False): "劫财",
        ("我生", True): "食神",
        ("我生", False): "伤官",
        ("我克", True): "偏财",
        ("我克", False): "正财",
        ("克我", True): "七杀",
        ("克我", False): "正官",
        ("生我", True): "偏印",
        ("生我", False): "正印",
    }

    return ten_god_map.get((relation, same_yy), "未知")


def get_all_ten_gods(day_master: str) -> Dict[str, str]:
    """
    获取日主与所有天干的十神关系

    :param day_master: 日主天干
    :return: {天干: 十神} 的字典
    """
    result = {}
    for gan in TIAN_GAN:
        result[gan] = calculate_ten_god(day_master, gan)
    return result


def extract_day_master(bazi_day: str) -> str:
    """
    从日柱提取日主天干

    :param bazi_day: 日柱 (如 "甲子" 或 "甲")
    :return: 日主天干 (如 "甲")
    """
    if not bazi_day:
        raise ValueError("日柱不能为空")
    return bazi_day[0]


def get_ten_god_nature(ten_god: str) -> Dict[str, any]:
    """
    获取十神的性质特征

    :param ten_god: 十神名称
    :return: 包含性质描述的字典
    """
    natures = {
        "比肩": {
            "element": "同我",
            "nature": "助力",
            "keywords": ["合作", "竞争", "独立", "自我"],
            "favorable_energy": "中性",
            "description": "代表兄弟姐妹、朋友、竞争者"
        },
        "劫财": {
            "element": "同我",
            "nature": "争夺",
            "keywords": ["争斗", "破财", "冲动", "魄力"],
            "favorable_energy": "偏凶",
            "description": "代表竞争对手、小人、耗财"
        },
        "食神": {
            "element": "我生",
            "nature": "泄秀",
            "keywords": ["才华", "口福", "悠闲", "创作"],
            "favorable_energy": "吉",
            "description": "代表才艺、食禄、子女"
        },
        "伤官": {
            "element": "我生",
            "nature": "泄气",
            "keywords": ["聪明", "叛逆", "创新", "口舌"],
            "favorable_energy": "偏凶",
            "description": "代表口才、创意、也代表伤害、官司"
        },
        "正财": {
            "element": "我克",
            "nature": "正得",
            "keywords": ["稳定收入", "妻子", "务实", "节俭"],
            "favorable_energy": "吉",
            "description": "代表正当财富、妻子、稳定"
        },
        "偏财": {
            "element": "我克",
            "nature": "意外",
            "keywords": ["横财", "父亲", "慷慨", "投机"],
            "favorable_energy": "中性",
            "description": "代表意外之财、父亲、社交"
        },
        "正官": {
            "element": "克我",
            "nature": "约束",
            "keywords": ["事业", "丈夫", "规矩", "名声"],
            "favorable_energy": "吉",
            "description": "代表事业、丈夫、上司、法律"
        },
        "七杀": {
            "element": "克我",
            "nature": "压制",
            "keywords": ["权力", "小人", "魄力", "压力"],
            "favorable_energy": "偏凶",
            "description": "代表压力、权威、也代表小人、意外"
        },
        "正印": {
            "element": "生我",
            "nature": "生助",
            "keywords": ["学业", "母亲", "贵人", "文书"],
            "favorable_energy": "吉",
            "description": "代表学识、母亲、贵人相助"
        },
        "偏印": {
            "element": "生我",
            "nature": "偏助",
            "keywords": ["偏门学问", "继母", "孤独", "玄学"],
            "favorable_energy": "中性",
            "description": "代表偏门技艺、宗教、也代表孤独"
        }
    }
    return natures.get(ten_god, {
        "element": "未知",
        "nature": "未知",
        "keywords": [],
        "favorable_energy": "未知",
        "description": "未知十神"
    })
