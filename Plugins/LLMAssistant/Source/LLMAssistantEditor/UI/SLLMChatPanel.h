#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SScrollBox;
class SMultiLineEditableTextBox;
class SButton;
class FLLMService;

class SLLMChatPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLLMChatPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    FReply OnSendClicked();
    void AddChatBubble(const FString& Message, bool bIsUser);

    // API 응답 수신 콜백
    void OnLLMResponse(bool bSuccess, const FString& Response, const FString& Error);

    // 입력 잠금/해제
    void SetInputEnabled(bool bEnabled);

    TSharedPtr<SScrollBox> ChatScrollBox;
    TSharedPtr<SMultiLineEditableTextBox> InputTextBox;
    TSharedPtr<SButton> SendButton;

    // LLM 서비스 (대화 관리 + API 호출)
    TSharedPtr<FLLMService> LLMService;
};