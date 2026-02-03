from typing import List, Optional
from sqlalchemy.ext.asyncio import AsyncSession
from app.models.divination import DivinationRecord
from app.core.ai_service import ai_service
from app.core.prompt_sanitizer import PromptSanitizer
import json

class DivinationService:
    """解字服务"""

    @staticmethod
    async def interpret(
        db: AsyncSession,
        user_id: int,
        words: List[str],
        mode: str,
        intent: str
    ) -> DivinationRecord:
        """
        进行解字测算
        """
        # 1. 安全检查
        PromptSanitizer.validate_word_list(words)

        # 2. 调用 AI 解签
        ai_result = await ai_service.interpret_divination(words, mode, intent)

        # 3. 保存记录
        record = DivinationRecord(
            user_id=user_id,
            mode=mode,
            intent=intent,
            selected_words=words,
            result_idiom=ai_result.get("idiom"),
            result_interpretation=ai_result.get("interpretation"),
            result_advice=ai_result.get("advice")
        )

        db.add(record)
        await db.commit()
        await db.refresh(record)

        return record
