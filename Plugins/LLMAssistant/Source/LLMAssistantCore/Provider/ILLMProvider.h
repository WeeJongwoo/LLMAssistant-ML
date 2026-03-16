#pragma once

#include "CoreMinimal.h"

// 응답 완료 콜백: 성공 여부, 응답 텍스트, 에러 메시지
DECLARE_DELEGATE_ThreeParams(FOnLLMResponseComplete, bool /*bSuccess*/, const FString& /*Response*/, const FString& /*Error*/);

/**
 * LLM 프로바이더 추상 인터페이스.
 * Claude, Gemini, Ollama 등을 동일 인터페이스로 교체 가능.
 */
class ILLMProvider
{
public:
    virtual ~ILLMProvider() = default;

    /**
     * 비-스트리밍 요청 전송.
     * @param MessagesJson  - messages 배열의 JSON 문자열
     * @param OnComplete    - 응답 완료 시 콜백
     */
    virtual void SendRequest(const FString& MessagesJson, FOnLLMResponseComplete OnComplete) = 0;

    /** 프로바이더 이름 (디버그/로그용) */
    virtual FString GetProviderName() const = 0;
};