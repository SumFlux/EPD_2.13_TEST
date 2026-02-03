import httpx
import json
import logging
from typing import List, Dict, Any, Optional
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import select
from app.config import settings
from app.models.system import SystemConfig
from app.core.database import AsyncSessionLocal
from fastapi import HTTPException, status
import time

logger = logging.getLogger(__name__)

class AIService:
    """
    AI 服务统一封装
    支持动态配置 (API Key, Model Name)
    """

    def __init__(self):
        self.client = httpx.AsyncClient(timeout=settings.AI_TIMEOUT)
        # 简单内存缓存，避免每次都查库
        self._config_cache = {}
        self._cache_expiry = 0
        self._cache_ttl = 60  # 缓存 60 秒

    async def _get_config(self, db: Optional[AsyncSession], key: str, default: str) -> str:
        """
        从数据库获取动态配置
        优先使用传入的 db session，如果没有则创建新的（不推荐高并发使用）
        """
        # 1. 检查内存缓存
        if time.time() < self._cache_expiry and key in self._config_cache:
            return self._config_cache[key]

        try:
            # 2. 查库
            if db:
                result = await db.execute(
                    select(SystemConfig).where(SystemConfig.config_key == key)
                )
                config = result.scalars().first()
            else:
                # 兼容性 Fallback：不推荐，但在某些无法传入 db 的上下文中可用
                async with AsyncSessionLocal() as session:
                    result = await session.execute(
                        select(SystemConfig).where(SystemConfig.config_key == key)
                    )
                    config = result.scalars().first()

            value = config.config_value if config and config.config_value else default

            # 3. 更新缓存
            self._config_cache[key] = value
            self._cache_expiry = time.time() + self._cache_ttl

            return value
        except Exception as e:
            logger.warning(f"Failed to load system config {key}: {e}")
            return default

    async def get_ai_config(self, db: Optional[AsyncSession] = None) -> Dict[str, str]:
        """获取当前的 AI 配置"""
        return {
            "api_key": await self._get_config(db, "ai.api_key", settings.AI_API_KEY),
            "model_name": await self._get_config(db, "ai.model_name", settings.AI_MODEL_NAME),
            "base_url": await self._get_config(db, "ai.base_url", settings.AI_BASE_URL)
        }

    async def chat_completion(
        self,
        system_prompt: str,
        user_message: str,
        temperature: float = 0.8,
        db: Optional[AsyncSession] = None
    ) -> str:
        """统一的 AI 对话接口"""
        config = await self.get_ai_config(db)

        try:
            response = await self.client.post(
                f"{config['base_url']}/chat/completions",
                headers={
                    "Authorization": f"Bearer {config['api_key']}",
                    "Content-Type": "application/json"
                },
                json={
                    "model": config['model_name'],
                    "messages": [
                        {"role": "system", "content": system_prompt},
                        {"role": "user", "content": user_message}
                    ],
                    "temperature": temperature
                }
            )
            response.raise_for_status()
            data = response.json()
            if not data.get("choices"):
                 raise ValueError("AI response format error: no choices")
            return data["choices"][0]["message"]["content"]
        except httpx.HTTPError as e:
            logger.error(f"AI API request failed: {e}")
            raise HTTPException(
                status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
                detail=f"AI Service Error: {str(e)}"
            )

    async def generate_almanac_commentary(
        self,
        bazi: Dict[str, str],
        ganzhi_day: str,
        profession: Optional[str],
        focus_areas: Optional[List[str]],
        db: Optional[AsyncSession] = None
    ) -> str:
        """生成黄历天机批注"""
        system_prompt = """你是一位精通命理学的天机大师。
请基于用户的八字日主与当日干支的十神关系，
结合职业和关注点，生成20-40字的个性化批注。
风格：半文半白，富有禅意，点到为止。
不要解释原理，直接给出批注。"""

        prof_str = profession if profession else "未填写"
        focus_str = ", ".join(focus_areas) if focus_areas else "综合运势"

        user_message = f"""
用户八字日主: {bazi.get('day', '未知')}
当日干支: {ganzhi_day}
职业: {prof_str}
关注点: {focus_str}
请生成今日天机批注。
"""
        return await self.chat_completion(system_prompt, user_message, 0.9, db)

    async def interpret_divination(
        self,
        words: List[str],
        mode: str,
        intent: str,
        db: Optional[AsyncSession] = None
    ) -> Dict[str, Any]:
        """解字 AI 解签"""
        system_prompt = f"""你是测字大师。
用户选择了「{'、'.join(words)}」二字进行{intent}测算。
模式：{mode}

请严格按照JSON格式回答，不要包含markdown代码块标记：
{{
  "idiom": "相关成语(4字)",
  "interpretation": "详细解签（100-200字）",
  "advice": "宜忌建议（50字内）"
}}"""

        response = await self.chat_completion(
            system_prompt,
            f"请为「{words[0]}」「{words[1]}」二字解签。",
            0.85,
            db
        )

        # 清理 markdown 标记
        clean_response = response.strip().replace("```json", "").replace("```", "")

        # 尝试解析 JSON，增加容错
        try:
            return json.loads(clean_response)
        except json.JSONDecodeError:
            logger.error(f"Failed to parse AI response as JSON: {clean_response}")
            # 降级处理：尝试修复或返回默认错误
            # 这里简单返回一个错误提示结构，避免 500
            return {
                "idiom": "天机不可泄",
                "interpretation": "云深不知处，AI 暂时无法解读天机。请稍后再试。",
                "advice": "宜：静心等待。忌：心浮气躁。"
            }

    async def close(self):
        """关闭客户端"""
        await self.client.aclose()

# 单例实例
ai_service = AIService()
