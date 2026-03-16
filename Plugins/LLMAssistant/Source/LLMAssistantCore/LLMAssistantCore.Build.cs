// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class LLMAssistantCore : ModuleRules
{
	public LLMAssistantCore(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        IWYUSupport = IWYUSupport.Full;

        string ModuleDir = ModuleDirectory;
        PublicIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDir, "Provider"),
				Path.Combine(ModuleDir, "Service"),
				Path.Combine(ModuleDir, "Settings"),
				// [확장 예약] 폴더가 비어있어도 경로 등록은 무해
				Path.Combine(ModuleDir, "Conversation")
            });
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
                "Engine",
                "DeveloperSettings",
				"Settings"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				//"Slate",
				//"SlateCore",
				"HTTP",
				"Json",
				"JsonUtilities"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
