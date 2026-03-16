#pragma once

#include "CoreMinimal.h"

/** 대화 메시지 하나를 나타내는 구조체 */
struct FLLMMessage
{
    /** "user" 또는 "assistant" */
    FString Role;

    /** 메시지 내용 */
    FString Content;

    FLLMMessage() = default;

    FLLMMessage(const FString& InRole, const FString& InContent)
        : Role(InRole), Content(InContent)
    {
    }
};