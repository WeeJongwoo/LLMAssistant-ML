#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "LLMAssistantSettings.generated.h"

/**
 * Project Settings > Plugins > LLM Assistant 에 노출되는 설정.
 * API 키는 EditorPerProjectUserSettings에 저장되어 소스 컨트롤에 올라가지 않음.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "LLM Assistant"))
class LLMASSISTANTCORE_API ULLMAssistantSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    ULLMAssistantSettings();

    // ── API 설정 ──

    /** LLM API 키 (Gemini, Claude 등) */
    UPROPERTY(config, EditAnywhere, Category = "API",
        meta = (DisplayName = "API Key", PasswordField = true))
    FString APIKey;

    /** API 엔드포인트 URL */
    UPROPERTY(config, EditAnywhere, Category = "API",
        meta = (DisplayName = "Endpoint URL"))
    FString EndpointURL;

    /** 사용할 모델 이름 */
    UPROPERTY(config, EditAnywhere, Category = "API",
        meta = (DisplayName = "Model Name"))
    FString ModelName;

    /** 응답 최대 토큰 수 */
    UPROPERTY(config, EditAnywhere, Category = "API",
        meta = (DisplayName = "Max Tokens", ClampMin = "1", ClampMax = "32768"))
    int32 MaxTokens;

    // ── 헬퍼 ──

    /** 싱글톤 접근 */
    static const ULLMAssistantSettings* Get()
    {
        return GetDefault<ULLMAssistantSettings>();
    }

    // UDeveloperSettings
    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
    virtual FName GetSectionName() const override { return TEXT("LLMAssistant"); }
};