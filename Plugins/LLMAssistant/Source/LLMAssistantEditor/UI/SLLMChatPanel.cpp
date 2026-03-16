#include "SLLMChatPanel.h"
#include "LLMService.h"

#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/AppStyle.h"

void SLLMChatPanel::Construct(const FArguments& InArgs)
{
    // LLM 서비스 생성
    LLMService = MakeShared<FLLMService>();

    ChildSlot
    [
        SNew(SVerticalBox)

        // ── 대화 영역 ──
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(4.0f)
        [
            SAssignNew(ChatScrollBox, SScrollBox)
            + SScrollBox::Slot()
            [
                SNew(STextBlock)
                .Text(FText::FromString(TEXT("LLM Assistant에 오신 걸 환영합니다. 메시지를 입력하세요.")))
                .ColorAndOpacity(FSlateColor(FLinearColor::Gray))
                .AutoWrapText(true)
            ]
        ]

        // ── 입력 영역 ──
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f)
        [
            SNew(SHorizontalBox)

            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .Padding(0.0f, 0.0f, 4.0f, 0.0f)
            [
                SNew(SBox)
                .MinDesiredHeight(60.0f)
                .MaxDesiredHeight(120.0f)
                [
                    SAssignNew(InputTextBox, SMultiLineEditableTextBox)
                    .AutoWrapText(true)
                ]
            ]

            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Bottom)
            [
                SAssignNew(SendButton, SButton)
                .Text(FText::FromString(TEXT("Send")))
                .OnClicked(this, &SLLMChatPanel::OnSendClicked)
            ]
        ]
    ];
}

FReply SLLMChatPanel::OnSendClicked()
{
    if (!InputTextBox.IsValid() || !LLMService.IsValid())
    {
        return FReply::Handled();
    }

    // 요청 중이면 무시
    if (LLMService->IsRequestInProgress())
    {
        return FReply::Handled();
    }

    FString UserMessage = InputTextBox->GetText().ToString();
    UserMessage.TrimStartAndEndInline();

    if (UserMessage.IsEmpty())
    {
        return FReply::Handled();
    }

    // 1) 유저 메시지 표시
    AddChatBubble(UserMessage, true);

    // 2) 입력창 초기화 + 비활성화
    InputTextBox->SetText(FText::GetEmpty());
    SetInputEnabled(false);

    // 3) "응답 대기 중..." 표시
    AddChatBubble(TEXT("응답 대기 중..."), false);

    // 4) API 호출
    LLMService->SendMessage(UserMessage,
        FOnChatResponseReady::CreateSP(this, &SLLMChatPanel::OnLLMResponse));

    return FReply::Handled();
}

void SLLMChatPanel::OnLLMResponse(bool bSuccess, const FString& Response, const FString& Error)
{
    // "응답 대기 중..." 버블 제거 (마지막 슬롯)
    if (ChatScrollBox->GetChildren()->Num() > 0)
    {
        ChatScrollBox->RemoveSlot(
            ChatScrollBox->GetChildren()->GetChildAt(ChatScrollBox->GetChildren()->Num() - 1));
    }

    // 응답 또는 에러 표시
    if (bSuccess)
    {
        AddChatBubble(Response, false);
    }
    else
    {
        AddChatBubble(FString::Printf(TEXT("[오류] %s"), *Error), false);
    }

    // 입력 다시 활성화
    SetInputEnabled(true);
}

void SLLMChatPanel::SetInputEnabled(bool bEnabled)
{
    if (InputTextBox.IsValid())
    {
        InputTextBox->SetEnabled(bEnabled);
    }
    if (SendButton.IsValid())
    {
        SendButton->SetEnabled(bEnabled);
    }
}

void SLLMChatPanel::AddChatBubble(const FString& Message, bool bIsUser)
{
    FLinearColor BubbleColor = bIsUser
        ? FLinearColor(0.2f, 0.4f, 0.8f, 1.0f)
        : FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);

    FString Prefix = bIsUser ? TEXT("[You] ") : TEXT("[AI] ");

    ChatScrollBox->AddSlot()
    .Padding(FMargin(bIsUser ? 40.0f : 4.0f, 4.0f, bIsUser ? 4.0f : 40.0f, 4.0f))
    [
        SNew(SBorder)
        .BorderBackgroundColor(BubbleColor)
        .Padding(8.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(Prefix + Message))
            .AutoWrapText(true)
            .ColorAndOpacity(FSlateColor(FLinearColor::White))
        ]
    ];

    ChatScrollBox->ScrollToEnd();
}