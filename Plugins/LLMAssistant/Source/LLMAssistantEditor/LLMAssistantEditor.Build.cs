using UnrealBuildTool;
using System.IO;

public class LLMAssistantEditor : ModuleRules
{
    public LLMAssistantEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        IWYUSupport = IWYUSupport.Full;

        string ModuleDir = ModuleDirectory;
        PublicIncludePaths.AddRange(new string[]
        {
            Path.Combine(ModuleDir, "UI"),
            // [확장 예약]
            Path.Combine(ModuleDir, "Context"),
            Path.Combine(ModuleDir, "Action"),
            Path.Combine(ModuleDir, "Template")
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "LLMAssistantCore"     // Runtime 모듈 참조
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "InputCore",
            "UnrealEd",
            "ToolMenus"
            // [확장 시 추가]
            // "LevelEditor"      — 뷰포트 접근
            // "ContentBrowser"    — 에셋 검색
            // "AssetRegistry"     — 에셋 레지스트리
        });
    }
}