// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once


#include "Modules/ModuleManager.h"

// Runtime 모듈 — Provider, Service, Settings를 컴파일 단위에 포함
class FLLMAssistantModule : public IModuleInterface
{
public:
    virtual void StartupModule() override {}
    virtual void ShutdownModule() override {}
};
