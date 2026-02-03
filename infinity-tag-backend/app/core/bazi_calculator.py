import sxtwl
from datetime import datetime

class BaziCalculator:
    """
    八字计算引擎
    基于 sxtwl (寿星天文历)
    """

    Gan = ["甲", "乙", "丙", "丁", "戊", "己", "庚", "辛", "壬", "癸"]
    Zhi = ["子", "丑", "寅", "卯", "辰", "巳", "午", "未", "申", "酉", "戌", "亥"]

    @classmethod
    def calculate(cls, birth_time: datetime) -> dict:
        """
        计算八字
        :param birth_time: 出生时间 (公历)
        :return: {'year': '甲子', 'month': '丙寅', 'day': '癸丑', 'hour': '壬戌'}
        """
        # sxtwl 接受的年份需考虑天文历法的特性，这里直接使用
        day = sxtwl.fromSolar(
            birth_time.year,
            birth_time.month,
            birth_time.day
        )

        # 计算年柱
        year_gz = day.getYearGZ()
        y_gan = cls.Gan[year_gz.tg]
        y_zhi = cls.Zhi[year_gz.dz]

        # 计算月柱
        month_gz = day.getMonthGZ()
        m_gan = cls.Gan[month_gz.tg]
        m_zhi = cls.Zhi[month_gz.dz]

        # 计算日柱
        day_gz = day.getDayGZ()
        d_gan = cls.Gan[day_gz.tg]
        d_zhi = cls.Zhi[day_gz.dz]

        # 计算时柱
        # sxtwl 的 getHourGZ 需要传入 (时支索引)，需要先根据小时计算时支
        # 子时: 23-1, 丑时: 1-3 ...
        # (hour + 1) // 2 % 12
        hour_zhi_idx = (birth_time.hour + 1) // 2 % 12
        hour_gz = day.getHourGZ(hour_zhi_idx)
        h_gan = cls.Gan[hour_gz.tg]
        h_zhi = cls.Zhi[hour_gz.dz]

        return {
            "year": f"{y_gan}{y_zhi}",
            "month": f"{m_gan}{m_zhi}",
            "day": f"{d_gan}{d_zhi}",
            "hour": f"{h_gan}{h_zhi}"
        }

    @classmethod
    def get_lunar_date(cls, date: datetime) -> str:
        """获取农历日期描述"""
        day = sxtwl.fromSolar(date.year, date.month, date.day)
        # 简单实现，sxtwl 有更多细节
        months = ["正月", "二月", "三月", "四月", "五月", "六月",
                  "七月", "八月", "九月", "十月", "冬月", "腊月"]
        days = ["初一", "初二", "初三", "初四", "初五", "初六", "初七", "初八", "初九", "初十",
                "十一", "十二", "十三", "十四", "十五", "十六", "十七", "十八", "十九", "二十",
                "廿一", "廿二", "廿三", "廿四", "廿五", "廿六", "廿七", "廿八", "廿九", "三十"]

        # sxtwl 的 month 是从 1 开始的，leap 表示是否闰月
        m = day.getLunarMonth()
        d = day.getLunarDay()

        month_str = months[m-1] if 1 <= m <= 12 else f"{m}月"
        if day.isLunarLeap():
            month_str = "闰" + month_str

        day_str = days[d-1] if 1 <= d <= 30 else f"{d}日"

        return f"{month_str}{day_str}"
