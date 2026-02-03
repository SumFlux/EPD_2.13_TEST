import re
from typing import List

class PromptSanitizer:
    """Prompt 注入攻击防护"""

    DANGEROUS_PATTERNS = [
        r"ignore\s+previous\s+instructions",
        r"system\s*:",
        r"forget\s+everything",
        r"new\s+instructions",
        r"your\s+instructions",
        r"openai",
        r"anthropic"
    ]

    @classmethod
    def sanitize(cls, user_input: str, max_length: int = 200) -> str:
        """清洗用户输入"""
        if not user_input:
            return ""

        # 长度限制
        if len(user_input) > max_length:
            raise ValueError(f"输入超过最大长度 {max_length}")

        # 检测危险模式
        for pattern in cls.DANGEROUS_PATTERNS:
            if re.search(pattern, user_input, re.IGNORECASE):
                raise ValueError("检测到不安全的输入内容")

        # 移除特殊字符 (保留汉字、数字、字母、常用标点)
        # 移除 < > { } [ ] \
        sanitized = re.sub(r'[<>{}[\]\\]', '', user_input)
        return sanitized.strip()

    @classmethod
    def validate_word_list(cls, words: List[str]) -> bool:
        """验证字列表是否安全"""
        if len(words) != 2:
            raise ValueError("必须选择2个字")

        for word in words:
            if not cls.is_chinese_character(word):
                raise ValueError(f"'{word}' 不是有效的汉字")
        return True

    @staticmethod
    def is_chinese_character(char: str) -> bool:
        """判断是否为汉字"""
        if len(char) != 1:
            return False
        return '\u4e00' <= char <= '\u9fff'
