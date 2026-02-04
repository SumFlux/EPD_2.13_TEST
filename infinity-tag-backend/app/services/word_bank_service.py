"""
字库服务
从 JSON 文件加载字库数据，提供随机字获取功能
"""
import json
import random
from pathlib import Path
from typing import Dict, List, Optional


class WordBankService:
    """字库服务 - 单例模式"""

    _instance = None
    _words: Optional[Dict] = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._load_words()
        return cls._instance

    @classmethod
    def _load_words(cls) -> None:
        """加载字库数据"""
        words_path = Path(__file__).parent.parent / "data" / "word_bank.json"
        try:
            with open(words_path, "r", encoding="utf-8") as f:
                cls._words = json.load(f)
        except FileNotFoundError:
            cls._words = cls._get_default_words()

    @classmethod
    def _get_default_words(cls) -> Dict:
        """默认字库（备用）"""
        return {
            "categories": {
                "默认": ["天", "地", "人", "和", "道", "法", "术", "器"]
            },
            "all_words": ["天", "地", "人", "和", "道", "法", "术", "器",
                         "名", "利", "情", "缘", "心", "性", "命", "运"],
            "special_combinations": {}
        }

    @classmethod
    def get_instance(cls) -> "WordBankService":
        """获取单例实例"""
        if cls._instance is None:
            cls._instance = cls()
        return cls._instance

    def get_random_words(
        self,
        count: int = 8,
        category: Optional[str] = None
    ) -> List[str]:
        """
        获取随机字

        :param count: 需要的字数
        :param category: 可选的类别筛选
        :return: 随机字列表
        """
        if category and category in self._words.get("categories", {}):
            pool = self._words["categories"][category]
        else:
            pool = self._words.get("all_words", [])

        # 确保不超过池中字数
        actual_count = min(count, len(pool))
        return random.sample(pool, k=actual_count)

    def get_words_by_intent(
        self,
        intent: str,
        count: int = 8
    ) -> List[str]:
        """
        根据意图获取相关字

        :param intent: 意图类型 (如 "事业", "姻缘", "学业", "健康")
        :param count: 需要的字数
        :return: 相关字列表
        """
        special = self._words.get("special_combinations", {})

        if intent in special:
            base_words = list(special[intent])
        else:
            base_words = []

        # 如果特殊组合不够，从全部字库补充
        if len(base_words) < count:
            all_words = self._words.get("all_words", [])
            remaining = [w for w in all_words if w not in base_words]
            needed = count - len(base_words)
            base_words.extend(random.sample(remaining, k=min(needed, len(remaining))))

        return random.sample(base_words, k=min(count, len(base_words)))

    def get_all_categories(self) -> List[str]:
        """获取所有可用类别"""
        return list(self._words.get("categories", {}).keys())

    def get_category_words(self, category: str) -> List[str]:
        """获取指定类别的所有字"""
        return self._words.get("categories", {}).get(category, [])

    def get_metadata(self) -> Dict:
        """获取字库元数据"""
        return self._words.get("metadata", {})


# 便捷函数
def get_random_words(count: int = 8, category: Optional[str] = None) -> List[str]:
    """获取随机字的便捷函数"""
    service = WordBankService.get_instance()
    return service.get_random_words(count, category)


def get_words_by_intent(intent: str, count: int = 8) -> List[str]:
    """根据意图获取字的便捷函数"""
    service = WordBankService.get_instance()
    return service.get_words_by_intent(intent, count)
