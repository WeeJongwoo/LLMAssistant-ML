#include "LLMAssistantEditorModule.h"
#include "SLLMChatPanel.h"

#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

// 탭 고유 ID
static const FName LLMChatTabName(TEXT("LLMAssistantChat"));

void FLLMAssistantEditorModule::StartupModule()
{
    // 1) Nomad Tab 등록 — 에디터 어디서든 열 수 있는 독립 탭
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        LLMChatTabName,
        FOnSpawnTab::CreateRaw(this, &FLLMAssistantEditorModule::SpawnChatTab)
    )
        .SetDisplayName(FText::FromString(TEXT("LLM Assistant")))
        .SetMenuType(ETabSpawnerMenuType::Hidden); // 툴바 버튼으로만 열기

    // 2) ToolMenus가 준비되면 툴바 버튼 등록
    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(
            this, &FLLMAssistantEditorModule::RegisterToolbarButton
        )
    );
}

void FLLMAssistantEditorModule::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LLMChatTabName);

    // ToolMenus 정리
    if (UToolMenus* ToolMenus = UToolMenus::TryGet())
    {
        ToolMenus->UnregisterOwner(this);
    }
}

TSharedRef<SDockTab> FLLMAssistantEditorModule::SpawnChatTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SLLMChatPanel)
        ];
}

void FLLMAssistantEditorModule::RegisterToolbarButton()
{
    // 에디터 메인 툴바에 버튼 추가
    UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(
        "LevelEditor.LevelEditorToolBar.PlayToolBar"
    );

    FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("LLMAssistant");

    Section.AddEntry(FToolMenuEntry::InitToolBarButton(
        "OpenLLMChat",
        FUIAction(FExecuteAction::CreateLambda([]()
            {
                // 탭 열기 (이미 열려 있으면 포커스)
                FGlobalTabmanager::Get()->TryInvokeTab(LLMChatTabName);
            })),
        FText::FromString(TEXT("LLM Chat")),   // 버튼 라벨
        FText::FromString(TEXT("LLM 채팅 탭 열기")),  // 툴팁
        FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Comment")
    ));
}

IMPLEMENT_MODULE(FLLMAssistantEditorModule, LLMAssistantEditor)