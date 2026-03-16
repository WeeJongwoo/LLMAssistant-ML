// Copyright Epic Games, Inc. All Rights Reserved.

#include "LLMAssistantModule.h"
#include "LLMAssistantSettings.h"
#include "ISettingsModule.h"

//#define LOCTEXT_NAMESPACE "FLLMAsistantModule"

void FLLMAssistantModule::StartupModule()
{
    // Settings 모듈을 통해 수동 등록
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->RegisterSettings(
            "Project",                                          // 컨테이너 (Project Settings)
            "Plugins",                                          // 카테고리
            "LLMAssistant",                                     // 섹션 이름
            FText::FromString(TEXT("LLM Assistant")),            // 표시 이름
            FText::FromString(TEXT("LLM 채팅 어시스턴트 설정")),    // 설명
            GetMutableDefault<ULLMAssistantSettings>()           // 설정 객체
        );
    } 
}

void FLLMAssistantModule::ShutdownModule()
{
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "LLMAssistant");
    }
}

IMPLEMENT_MODULE(FLLMAssistantModule, LLMAssistantCore)
