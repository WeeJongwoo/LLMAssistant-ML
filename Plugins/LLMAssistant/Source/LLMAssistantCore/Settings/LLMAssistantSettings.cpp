#include "LLMAssistantSettings.h"

ULLMAssistantSettings::ULLMAssistantSettings()
{
    // Gemini 무료 티어 기본값 (개발용)
    EndpointURL = TEXT("https://generativelanguage.googleapis.com/v1beta/openai/chat/completions");
    ModelName = TEXT("gemini-2.0-flash");
    MaxTokens = 4096;

    // 환경변수에서 API 키 폴백
    FString EnvKey = FPlatformMisc::GetEnvironmentVariable(TEXT("LLM_API_KEY"));
    if (!EnvKey.IsEmpty())
    {
        APIKey = EnvKey;
    }
}