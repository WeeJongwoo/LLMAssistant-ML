#include "LLMService.h"
#include "GeminiProvider.h"

FLLMService::FLLMService()
{
    // 기본 프로바이더로 Gemini 사용
    Provider = MakeUnique<FGeminiProvider>();
}

FLLMService::~FLLMService() = default;

void FLLMService::SendMessage(const FString& UserMessage, FOnChatResponseReady OnReady)
{
    if (bIsRequesting)
    {
        OnReady.ExecuteIfBound(false, TEXT(""), TEXT("이전 요청이 아직 진행 중입니다."));
        return;
    }

    // 1) 유저 메시지를 히스토리에 추가
    ConversationHistory.Add(FLLMMessage(TEXT("user"), UserMessage));

    // 2) 히스토리 전체를 JSON으로 변환
    FString MessagesJson = ConvertHistoryToJson();

    // 3) Provider에 전달
    bIsRequesting = true;

    Provider->SendRequest(MessagesJson,
        FOnLLMResponseComplete::CreateLambda(
            [this, OnReady](bool bSuccess, const FString& Response, const FString& Error)
            {
                bIsRequesting = false;

                if (bSuccess)
                {
                    // AI 응답을 히스토리에 추가
                    ConversationHistory.Add(FLLMMessage(TEXT("assistant"), Response));
                    OnReady.ExecuteIfBound(true, Response, TEXT(""));
                }
                else
                {
                    OnReady.ExecuteIfBound(false, TEXT(""), Error);
                }
            }
        )
    );
}

void FLLMService::ClearHistory()
{
    ConversationHistory.Empty();
}

FString FLLMService::ConvertHistoryToJson() const
{
    // [{"role":"user","content":"안녕"},{"role":"assistant","content":"안녕하세요!"}]
    FString Json = TEXT("[");

    for (int32 i = 0; i < ConversationHistory.Num(); ++i)
    {
        if (i > 0) Json += TEXT(",");

        // content 안의 특수문자 이스케이프
        FString EscapedContent = ConversationHistory[i].Content;
        EscapedContent.ReplaceInline(TEXT("\\"), TEXT("\\\\"));
        EscapedContent.ReplaceInline(TEXT("\""), TEXT("\\\""));
        EscapedContent.ReplaceInline(TEXT("\n"), TEXT("\\n"));
        EscapedContent.ReplaceInline(TEXT("\r"), TEXT("\\r"));
        EscapedContent.ReplaceInline(TEXT("\t"), TEXT("\\t"));

        Json += FString::Printf(
            TEXT("{\"role\":\"%s\",\"content\":\"%s\"}"),
            *ConversationHistory[i].Role,
            *EscapedContent
        );
    }

    Json += TEXT("]");
    return Json;
}