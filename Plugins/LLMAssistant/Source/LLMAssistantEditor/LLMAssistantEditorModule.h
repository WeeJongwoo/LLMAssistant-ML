#pragma once


#include "Modules/ModuleManager.h"

class FLLMAssistantEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    // 탭 생성 콜백 — FGlobalTabmanager가 탭을 열 때 호출
    TSharedRef<class SDockTab> SpawnChatTab(const class FSpawnTabArgs& Args);

    // 툴바 버튼 등록
    void RegisterToolbarButton();
};