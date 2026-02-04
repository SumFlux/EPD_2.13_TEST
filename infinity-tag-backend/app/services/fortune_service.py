"""
运势生成服务
基于十神关系 + 用户职业 + 关注领域生成个性化宜忌
"""
import json
import random
from pathlib import Path
from typing import Dict, List, Optional, Tuple

from app.utils.ten_gods import calculate_ten_god, extract_day_master, get_ten_god_nature


class FortuneService:
    """运势生成服务"""

    _instance = None
    _rules: Optional[Dict] = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._load_rules()
        return cls._instance

    @classmethod
    def _load_rules(cls) -> None:
        """加载运势规则数据"""
        rules_path = Path(__file__).parent.parent / "data" / "fortune_rules.json"
        try:
            with open(rules_path, "r", encoding="utf-8") as f:
                cls._rules = json.load(f)
        except FileNotFoundError:
            cls._rules = cls._get_default_rules()

    @classmethod
    def _get_default_rules(cls) -> Dict:
        """默认规则（备用）"""
        return {
            "ten_god_activities": {},
            "profession_mapping": {"default": {"favorable_bonus": [], "unfavorable_bonus": []}},
            "focus_area_mapping": {},
            "lucky_items_by_ten_god": {},
            "lucky_directions_by_ten_god": {}
        }

    @classmethod
    def get_instance(cls) -> "FortuneService":
        """获取单例实例"""
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    def generate_fortune(
        self,
        user_bazi_day: str,
        today_gan: str,
        profession: Optional[str] = None,
        focus_areas: Optional[List[str]] = None
    ) -> Dict:
        """
        生成个性化运势

        :param user_bazi_day: 用户日柱 (如 "甲子")
        :param today_gan: 当日天干 (如 "丙" 或 "丙寅")
        :param profession: 用户职业
        :param focus_areas: 用户关注领域列表
        :return: 运势结果字典
        """
        # 1. 提取日主天干
        day_master = extract_day_master(user_bazi_day)

        # 2. 计算十神关系
        ten_god = calculate_ten_god(day_master, today_gan)

        # 3. 获取基础宜忌
        base_favorable, base_unfavorable = self._get_base_activities(ten_god)

        # 4. 叠加职业加成 (使用不可变模式，避免修改原列表)
        prof_favorable, prof_unfavorable = self._get_profession_bonus(profession)
        combined_favorable = base_favorable + prof_favorable
        combined_unfavorable = base_unfavorable + prof_unfavorable

        # 5. 根据关注领域排序
        favorable = self._prioritize_by_focus(combined_favorable, focus_areas)
        unfavorable = self._prioritize_by_focus(combined_unfavorable, focus_areas)

        # 6. 去重并取前若干项
        favorable = self._deduplicate_and_limit(favorable, limit=5)
        unfavorable = self._deduplicate_and_limit(unfavorable, limit=5)

        # 7. 获取吉祥物和方位
        lucky_item = self._get_lucky_item(ten_god)
        lucky_direction = self._get_lucky_direction(ten_god)

        # 8. 计算能量等级
        energy_level = self._calculate_energy_level(ten_god)

        return {
            "ten_god": ten_god,
            "ten_god_nature": get_ten_god_nature(ten_god),
            "favorable": favorable[:3],
            "unfavorable": unfavorable[:3],
            "all_favorable": favorable,
            "all_unfavorable": unfavorable,
            "lucky_item": lucky_item,
            "lucky_direction": lucky_direction,
            "energy_level": energy_level
        }

    def _get_base_activities(self, ten_god: str) -> Tuple[List[str], List[str]]:
        """获取十神对应的基础宜忌活动"""
        activities = self._rules.get("ten_god_activities", {}).get(ten_god, {})
        favorable = list(activities.get("favorable", []))
        unfavorable = list(activities.get("unfavorable", []))

        # 如果规则为空，使用通用规则
        if not favorable:
            favorable = ["日常工作", "休息调整", "学习思考"]
        if not unfavorable:
            unfavorable = ["重大决策", "冒险尝试"]

        return favorable, unfavorable

    def _get_profession_bonus(self, profession: Optional[str]) -> Tuple[List[str], List[str]]:
        """获取职业加成宜忌"""
        if not profession:
            profession = "default"

        mapping = self._rules.get("profession_mapping", {})
        prof_rules = mapping.get(profession, mapping.get("default", {}))

        favorable = list(prof_rules.get("favorable_bonus", []))
        unfavorable = list(prof_rules.get("unfavorable_bonus", []))

        return favorable, unfavorable

    def _prioritize_by_focus(
        self,
        activities: List[str],
        focus_areas: Optional[List[str]]
    ) -> List[str]:
        """根据关注领域对活动进行优先级排序"""
        if not focus_areas or not activities:
            return activities

        focus_mapping = self._rules.get("focus_area_mapping", {})

        # 收集所有关注领域的关键词
        priority_keywords = set()
        for area in focus_areas:
            area_info = focus_mapping.get(area, {})
            keywords = area_info.get("keywords", [])
            priority_keywords.update(keywords)

        # 计算每个活动的优先级分数
        scored_activities = []
        for activity in activities:
            score = 0
            for keyword in priority_keywords:
                if keyword in activity:
                    score += 10
            # 部分匹配也给分
            for area in focus_areas:
                if area in activity:
                    score += 5
            scored_activities.append((activity, score))

        # 按分数降序排序
        scored_activities.sort(key=lambda x: x[1], reverse=True)

        return [act for act, _ in scored_activities]

    def _deduplicate_and_limit(self, items: List[str], limit: int = 5) -> List[str]:
        """去重并限制数量"""
        seen = set()
        result = []
        for item in items:
            if item not in seen:
                seen.add(item)
                result.append(item)
            if len(result) >= limit:
                break
        return result

    def _get_lucky_item(self, ten_god: str) -> str:
        """获取吉祥物"""
        items = self._rules.get("lucky_items_by_ten_god", {}).get(ten_god, [])
        if items:
            return random.choice(items)
        # 默认吉祥物
        default_items = ["红绳", "水晶", "桃木", "铜钱", "葫芦"]
        return random.choice(default_items)

    def _get_lucky_direction(self, ten_god: str) -> str:
        """获取吉利方位"""
        directions = self._rules.get("lucky_directions_by_ten_god", {}).get(ten_god, [])
        if directions:
            return random.choice(directions)
        # 默认方位
        default_directions = ["正东", "正西", "正南", "正北", "东南", "东北", "西南", "西北"]
        return random.choice(default_directions)

    def _calculate_energy_level(self, ten_god: str) -> int:
        """根据十神计算能量等级 (60-95)"""
        nature = get_ten_god_nature(ten_god)
        energy = nature.get("favorable_energy", "中性")

        base_ranges = {
            "吉": (75, 95),
            "中性": (65, 85),
            "偏凶": (60, 75)
        }

        min_val, max_val = base_ranges.get(energy, (65, 85))
        return random.randint(min_val, max_val)


# 便捷函数
def generate_personalized_fortune(
    user_bazi_day: str,
    today_gan: str,
    profession: Optional[str] = None,
    focus_areas: Optional[List[str]] = None
) -> Dict:
    """
    生成个性化运势的便捷函数

    :param user_bazi_day: 用户日柱
    :param today_gan: 当日天干
    :param profession: 职业
    :param focus_areas: 关注领域
    :return: 运势结果
    """
    service = FortuneService.get_instance()
    return service.generate_fortune(
        user_bazi_day=user_bazi_day,
        today_gan=today_gan,
        profession=profession,
        focus_areas=focus_areas
    )
