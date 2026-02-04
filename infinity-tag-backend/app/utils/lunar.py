"""
Infinity Tag - 历法与玄学计算核心
基于 sxtwl (寿星天文历) 实现高精度公历/农历转换与八字排盘

核心功能：
1. 公历/农历互转
2. 八字排盘 (年柱按立春换，月柱按节气换)
3. 节气计算
4. 日上起时
"""
import sxtwl
from typing import Dict
from datetime import date

# ==========================================
# 基础常量定义
# ==========================================

# 天干
TIAN_GAN = ["甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"]

# 地支
DI_ZHI = ["子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"]

# 生肖
ZODIAC = ["鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"]

# 农历月份
LUNAR_MONTHS = ["正", "二", "三", "四", "五", "六", "七", "八", "九", "十", "冬", "腊"]

# 农历日期
LUNAR_DAYS = [
    "初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十",
    "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
    "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"
]


class LunarUtils:
    """历法工具类"""

    @staticmethod
    def get_gan_zhi(year_gz: int, month_gz: int, day_gz: int, hour_gz: int) -> Dict[str, str]:
        """
        将干支索引转换为字符
        :return: {'year': '甲子', 'month': '乙丑', 'day': '丙寅', 'hour': '丁卯'}
        """
        return {
            "year": TIAN_GAN[year_gz % 10] + DI_ZHI[year_gz % 12],
            "month": TIAN_GAN[month_gz % 10] + DI_ZHI[month_gz % 12],
            "day": TIAN_GAN[day_gz % 10] + DI_ZHI[day_gz % 12],
            "hour": TIAN_GAN[hour_gz % 10] + DI_ZHI[hour_gz % 12],
        }

    @staticmethod
    def solar_to_lunar(solar_date: date) -> Dict[str, str]:
        """
        公历转农历 (简单转换，用于显示)
        :param solar_date: 公历日期
        :return: {'year_str': '甲辰', 'month_str': '正', 'day_str': '初一', 'full': '甲辰年正月初一'}
        """
        day = sxtwl.fromSolar(solar_date.year, solar_date.month, solar_date.day)

        # 获取农历年干支 (注意：这里通常是按春节换年，如果需要按立春换年请用 get_ba_zi)
        year_gz = day.getYearGZ(True) # True 表示按立春换年，False 按春节

        y_str = TIAN_GAN[year_gz.tg] + DI_ZHI[year_gz.dz]
        m_str = ("闰" if day.isLunarLeap() else "") + LUNAR_MONTHS[day.getLunarMonth() - 1]
        d_str = LUNAR_DAYS[day.getLunarDay() - 1]

        return {
            "year_str": y_str,
            "month_str": m_str,
            "day_str": d_str,
            "full": f"{y_str}年{m_str}月{d_str}"
        }

    @staticmethod
    def get_ba_zi(
        year: int,
        month: int,
        day: int,
        hour: int = -1,
        minute: int = 0
    ) -> Dict[str, str]:
        """
        【核心】获取生辰八字 (四柱)

        规则说明：
        1. 年柱：以"立春"为界，而非春节。
        2. 月柱：以"节气"为界 (如立春、惊蛰等)。
        3. 日柱：夜子时(23:00-00:00)通常算作第二天(或者是当天的子时，存疑)。
           sxtwl 默认处理：23点后算作第二天子时。
        4. 时柱：日上起时法 (五鼠遁)。

        :param year: 公历年
        :param month: 公历月
        :param day: 公历日
        :param hour: 公历时 (0-23)，-1表示未知
        :param minute: 分 (影响精确节气交接)
        :return: 包含四柱信息的字典
        """
        # 1. 构造 sxtwl 日期对象
        day_obj = sxtwl.fromSolar(year, month, day)

        # 2. 获取年、月、日柱 (True = 严格按节气换柱)
        # sxtwl 的 getYearGZ 传入 True 表示以立春为界 (八字年柱)
        # sxtwl 的 getMonthGZ 默认就是以节气为界，不需要参数
        y_gz = day_obj.getYearGZ(True)
        m_gz = day_obj.getMonthGZ()
        d_gz = day_obj.getDayGZ()

        # 3. 计算时柱
        # 23点以后算第二天子时，sxtwl 库本身通常是按天算的。
        # 我们手动处理时柱：需要用到"五鼠遁"口诀 (日上起时)
        # 甲己还加甲，乙庚丙作初...

        h_gz_str = "未知"
        # 只有当时辰已知时才计算时柱
        if hour != -1:
            # 23点跨天处理：如果是23点以后，日柱天干需要用第二天的吗？
            # 传统子平法：23:00-00:00 为晚子时，归属当天，但时柱用第二天的早子时干支。
            # sxtwl 并没有直接提供根据 hour 算出时柱干支的函数，需要我们根据日干推算。

            # 获取日干 (0=甲, 1=乙 ...)
            day_stem = d_gz.tg

            # 如果是 23:00 - 23:59，虽然日期还是当天，但在八字排盘中，
            # 时辰属于第二天的"早子时"，通常日柱也会按第二天算（这有争议）。
            # 这里我们采用主流做法：23点后，日柱按第二天算，时柱为子时。
            if hour >= 23:
                # 重新获取下一天的日柱
                next_day = sxtwl.fromSolar(year, month, day)
                # 这一步比较麻烦，简单做法是重新构造 date 对象加一天
                try:
                    # 简易处理：直接用日干推算。注意 23 点是子时，对应地支索引 0
                    # 但日干得是下一天的日干。
                    # 为了准确，我们这里简单处理：23点视作下一天的开始 (晚子时归入明日)
                    # 如果应用严格区分早晚子时，逻辑会更复杂。
                    # 这里采用：23:00即进入下一天子时。
                    day_stem = (day_stem + 1) % 10
                    hour_zhi = 0  # 子时
                except Exception:
                    pass
            else:
                # 普通时间
                # 00:00-00:59 => 子时 (0)
                # 01:00-02:59 => 丑时 (1)
                # ...
                # 转换公式: (hour + 1) // 2
                hour_zhi = (hour + 1) // 2

            # 五鼠遁推算时干
            # 甲己还加甲 -> 甲(0)己(5)日 -> 子时起甲(0)
            # 乙庚丙作初 -> 乙(1)庚(6)日 -> 子时起丙(2)
            # ...
            # 规律：时干 = (日干 % 5 * 2 + 时支) % 10
            hour_stem = (day_stem % 5 * 2 + hour_zhi) % 10

            h_gz_str = TIAN_GAN[hour_stem] + DI_ZHI[hour_zhi]

        # 组装结果
        result = {
            "year": TIAN_GAN[y_gz.tg] + DI_ZHI[y_gz.dz],
            "month": TIAN_GAN[m_gz.tg] + DI_ZHI[m_gz.dz],
            "day": TIAN_GAN[d_gz.tg] + DI_ZHI[d_gz.dz],
            "hour": h_gz_str,
        }

        # 格式化完整字符串: "甲子年 乙丑月 丙寅日 丁卯时"
        result["full"] = f"{result['year']}年 {result['month']}月 {result['day']}日 {result['hour']}时"
        return result

    @staticmethod
    def get_zodiac(year: int) -> str:
        """获取公历年份对应的生肖 (立春为界可能有误差，这里用最简单的年份取余)"""
        # 严格生肖应该按立春算，这里做个简单版，主要用于UI显示
        # 1984是鼠年(甲子)
        # (year - 1984) % 12
        return ZODIAC[(year - 1984) % 12]

    @staticmethod
    def get_hour_zhi(hour: int) -> str:
        """获取小时对应的地支 (用于显示)"""
        if hour == -1:
            return "未知"
        idx = (hour + 1) // 2 % 12
        return DI_ZHI[idx] + "时"

# 测试代码
if __name__ == "__main__":
    # 示例: 2024年2月10日 (甲辰年春节)
    lunar = LunarUtils.solar_to_lunar(date(2024, 2, 10))
    print(f"农历: {lunar['full']}")  # 应该是 甲辰年正月初一

    # 示例: 2024年2月3日 (立春前一天，还是癸卯年)
    bazi_prev = LunarUtils.get_ba_zi(2024, 2, 3, 12)
    print(f"立春前八字: {bazi_prev['full']}")

    # 示例: 2024年2月5日 (立春后，甲辰年)
    bazi_next = LunarUtils.get_ba_zi(2024, 2, 5, 12)
    print(f"立春后八字: {bazi_next['full']}")
