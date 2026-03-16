#pragma once

#include "CoreMinimal.h"
#include "ILLMProvider.h"

/**
 * Google Gemini API 프로바이더.
 * OpenAI 호환 엔드포인트를 사용하므로 나중에 Claude/OpenAI로 교체 쉬움.
 */
class FGeminiProvider : public ILLMProvider
{
public:
    virtual void SendRequest(const FString& MessagesJson, FOnLLMResponseComplete OnComplete) override;
    virtual FString GetProviderName() const override { return TEXT("Gemini"); }
};