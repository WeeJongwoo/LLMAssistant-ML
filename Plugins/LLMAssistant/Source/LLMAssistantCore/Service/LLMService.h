#pragma once

#include "CoreMinimal.h"
#include "LLMMessage.h"
#include "ILLMProvider.h"

// UI에 알려주는 콜백: 성공 여부, AI 응답 텍스트, 에러 메시지
DECLARE_DELEGATE_ThreeParams(FOnChatResponseReady, bool, const FString&, const FString&);

/**
 * 대화 서비스 — UI와 Provider 사이의 중간 레이어.
 * 대화 히스토리를 관리하고, 메시지를 JSON으로 변환하여 Provider에 전달.
 */
class LLMASSISTANTCORE_API FLLMService
{
public:
    FLLMService();
    ~FLLMService();

    /** 유저 메시지를 보내고 AI 응답을 받음 */
    void SendMessage(const FString& UserMessage, FOnChatResponseReady OnReady);

    /** 대화 히스토리 초기화 */
    void ClearHistory();

    /** 현재 요청 진행 중인지 */
    bool IsRequestInProgress() const { return bIsRequesting; }

private:
    /** 히스토리를 JSON 문자열로 변환 */
    FString ConvertHistoryToJson() const;

    TArray<FLLMMessage> ConversationHistory;
    TUniquePtr<ILLMProvider> Provider;
    bool bIsRequesting = false;
};