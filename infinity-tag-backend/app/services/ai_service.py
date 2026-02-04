"""
AI 服务模块
负责与 LLM 交互，生成个性化黄历
"""
import json
import httpx
import re
from typing import Dict
from loguru import logger
from datetime import date

from app.config import settings
from app.schemas.user import UserProfileBase
from app.schemas.almanac import AlmanacResponse
from app.core.prompts import ALMANAC_SYSTEM_PROMPT, ALMANAC_USER_PROMPT_TEMPLATE
from app.utils.lunar import LunarUtils


class AIService:
    """AI 服务封装类"""

    def __init__(self):
        self.api_key = settings.AI_API_KEY
        self.base_url = settings.AI_BASE_URL
        self.model = settings.AI_MODEL_NAME
        self.timeout = settings.AI_TIMEOUT
        self.max_retries = settings.AI_MAX_RETRIES

    def _get_headers(self) -> Dict[str, str]:
        """构造请求头"""
        return {
            "Authorization": f"Bearer {self.api_key}",
            "Content-Type": "application/json"
        }

    def _clean_json_string(self, text: str) -> str:
        """
        清洗 AI 返回的 JSON 字符串
        移除 markdown 代码块标记和多余的空白字符
        """
        # 1. 尝试提取 ```json ... ``` 内部的内容
        pattern = r"```json\s*(.*?)\s*```"
        match = re.search(pattern, text, re.DOTALL)
        if match:
            return match.group(1).strip()

        # 2. 如果没有 markdown 标记，尝试提取第一个 { ... }
        # 这是一个简单的启发式方法，应对 AI 在 JSON 前后加废话的情况
        start = text.find("{")
        end = text.rfind("}")
        if start != -1 and end != -1:
            return text[start : end + 1]

        return text.strip()

    async def generate_almanac(
        self,
        profile: UserProfileBase,
        target_date: date
    ) -> AlmanacResponse:
        """
        生成黄历
        :param profile: 用户档案
        :param target_date: 目标日期
        :return: AlmanacResponse 对象
        """
        # 1. 准备上下文数据 (八字计算)
        # 注意：这里我们基于用户生日计算八字，作为 Prompt 的一部分
        # 但生成的是"今日运势"，所以核心是"今日"的信息

        # 计算用户生辰八字
        user_bazi = LunarUtils.get_ba_zi(
            profile.birth_year,
            profile.birth_month,
            profile.birth_day,
            profile.birth_hour
        )

        # 计算今日的农历和节气信息
        # 为了获取今日的节气，我们复用 get_ba_zi 计算今日的干支和节气（sxtwl底层处理了）
        today_lunar = LunarUtils.solar_to_lunar(target_date)
        # 这里暂时用空字符串代替具体的节气，后续 LunarUtils 可扩展获取精准节气
        # 或者直接让 AI 基于日期去推算节气（AI通常知道公历对应的节气）
        solar_term = "未知"

        # 2. 构造 User Prompt
        user_prompt = ALMANAC_USER_PROMPT_TEMPLATE.format(
            gender="男" if profile.gender == 1 else "女",
            year_pillar=user_bazi['year'],
            month_pillar=user_bazi['month'],
            day_pillar=user_bazi['day'],
            hour_pillar=user_bazi['hour'],
            occupation=profile.occupation or "未知",
            date=target_date.isoformat(),
            weekday=target_date.strftime("%A"),
            lunar_date_str=today_lunar['full'],
            solar_term=solar_term
        )

        messages = [
            {"role": "system", "content": ALMANAC_SYSTEM_PROMPT},
            {"role": "user", "content": user_prompt}
        ]

        # 3. 调用 API (带重试)
        last_error = None

        for attempt in range(self.max_retries + 1):
            try:
                # 禁用 HTTP/2 以提高兼容性，显式设置超时
                async with httpx.AsyncClient(timeout=self.timeout, http2=False) as client:
                    logger.info(f"AI Request (Attempt {attempt+1}): {settings.AI_MODEL_NAME}")

                    response = await client.post(
                        f"{self.base_url}/chat/completions",
                        headers=self._get_headers(),
                        json={
                            "model": self.model,
                            "messages": messages,
                            "temperature": 0.7, # 稍微有创意一点，但不要太发散
                            "max_tokens": 2000
                        }
                    )

                    response.raise_for_status()
                    result = response.json()

                    content = result["choices"][0]["message"]["content"]
                    logger.debug(f"AI Raw Response: {content}")

                    # 4. 解析与验证
                    cleaned_json = self._clean_json_string(content)
                    data = json.loads(cleaned_json)

                    # 补充必要的字段以符合 Response 模型
                    # data 中包含了 core fields (lucky_color, etc.)
                    # 我们需要手动构造 AlmanacResponse，因为 Response 包含 id, date 等字段
                    # 但这里只返回 AI 生成的内容部分。
                    # 实际上，调用方需要组合这些数据。
                    # 为了方便，我们这里返回一个 dict 或者 局部的 Schema，
                    # 但为了类型安全，我们假设 AI 返回的就是 AlmanacResponse 的核心部分。

                    # 验证数据结构 (Pydantic 会自动转换类型)
                    # 注意：AlmanacResponse 包含了 id 和 date，这些不是 AI 生成的
                    # 所以我们这里只验证 AI 生成的字段。
                    # 临时创建一个 Pydantic 模型来验证 AI 部分，或者直接返回 dict

                    return data  # 直接返回字典，由上层 Service 组装完整的 Model

            except (httpx.HTTPError, json.JSONDecodeError, KeyError) as e:
                logger.warning(f"AI Generation failed (Attempt {attempt+1}): {str(e)}")
                last_error = e
                continue

        # 如果重试耗尽仍失败
        logger.error("AI Service failed after max retries")
        raise last_error or Exception("AI Service unavailable")

# 导出单例
ai_service = AIService()
