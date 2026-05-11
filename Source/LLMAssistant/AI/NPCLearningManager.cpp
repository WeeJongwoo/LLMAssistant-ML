// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCLearningManager.h"
#include "LLMAssistant/NPC/MLNPCCharacter.h"
#include "NPCInteractor.h"
#include "NPCTrainer.h"
#include "LearningAgentsManager.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "NPCMLManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

namespace
{
    struct FExperimentPresetConfig
    {
        ERewardVariant Variant;
        int32 Seed;
        const TCHAR* Tag;
    };

    bool ResolvePreset(EExperimentPreset Preset, FExperimentPresetConfig& OutCfg)
    {
        switch (Preset)
        {
        case EExperimentPreset::R1_Seed1234: OutCfg = { ERewardVariant::R1_Sparse, 1234, TEXT("R1_Seed1234") }; return true;
        case EExperimentPreset::R1_Seed5678: OutCfg = { ERewardVariant::R1_Sparse, 5678, TEXT("R1_Seed5678") }; return true;
        case EExperimentPreset::R1_Seed9999: OutCfg = { ERewardVariant::R1_Sparse, 9999, TEXT("R1_Seed9999") }; return true;
        case EExperimentPreset::R2_Seed1234: OutCfg = { ERewardVariant::R2_Dense, 1234, TEXT("R2_Seed1234") }; return true;
        case EExperimentPreset::R2_Seed5678: OutCfg = { ERewardVariant::R2_Dense, 5678, TEXT("R2_Seed5678") }; return true;
        case EExperimentPreset::R2_Seed9999: OutCfg = { ERewardVariant::R2_Dense, 9999, TEXT("R2_Seed9999") }; return true;
        case EExperimentPreset::R3_Seed1234: OutCfg = { ERewardVariant::R3_DenseOrientationCost, 1234, TEXT("R3_Seed1234") }; return true;
        case EExperimentPreset::R3_Seed5678: OutCfg = { ERewardVariant::R3_DenseOrientationCost, 5678, TEXT("R3_Seed5678") }; return true;
        case EExperimentPreset::R3_Seed9999: OutCfg = { ERewardVariant::R3_DenseOrientationCost, 9999, TEXT("R3_Seed9999") }; return true;
        default: return false;
        }
    }
}


// Sets default values
ANPCLearningManager::ANPCLearningManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//PrimaryActorTick.TickInterval = 0.1f;

	Manager = CreateDefaultSubobject<UNPCMLManager>(TEXT("Manager"));
}

// Called when the game starts or when spawned
void ANPCLearningManager::BeginPlay()
{
	Super::BeginPlay();

	// 실험 프리셋 적용 — Manual이 아니면 개별 필드를 덮어씀
	FExperimentPresetConfig PresetCfg;
	const bool bPresetActive = ResolvePreset(ExperimentPreset, PresetCfg);
	if (bPresetActive)
	{
		ExperimentSeed = PresetCfg.Seed;
		ExperimentTag = PresetCfg.Tag;
		SnapshotDir = FString::Printf(TEXT("NPCSnapshots/%s"), PresetCfg.Tag);

		// 새로운 학습이 항상 깨끗한 가중치에서 시작하도록 강제
		bReinitializePolicy = true;
		bReinitializeCritic = true;
		bLoadSnapshotOnBeginPlay = false;

		UE_LOG(LogTemp, Log, TEXT("[Experiment] Preset active: Tag=%s Seed=%d SnapshotDir=%s"),
			*ExperimentTag, ExperimentSeed, *SnapshotDir);
	}

	Interactor = Cast<UNPCInteractor>(ULearningAgentsInteractor::MakeInteractor(Manager, UNPCInteractor::StaticClass(), TEXT("NPCInteractor")));
	Interactor->SetGoalActor(GoalActor);

	Policy = ULearningAgentsPolicy::MakePolicy(Manager, Interactor, ULearningAgentsPolicy::StaticClass(), TEXT("Policy"), nullptr, PolicyNetworkAsset, nullptr, bReinitializePolicy, bReinitializePolicy, bReinitializePolicy);
	Critic = ULearningAgentsCritic::MakeCritic(Manager, Interactor, Policy, ULearningAgentsCritic::StaticClass(), TEXT("Critic"), CriticNetworkAsset, bReinitializeCritic);

	if (!bInferenceMode)
	{
		FLearningAgentsTrainerSettings TrainerSettings;
		TrainerSettings.MaxEpisodeStepNum = 2000;

		Trainer = Cast<UNPCTrainer>(ULearningAgentsTrainer::MakeTrainer(Manager, Interactor, Policy, Critic, UNPCTrainer::StaticClass(), TEXT("Trainer"), TrainerSettings));
		Trainer->SetGoalActor(GoalActor);
		Trainer->SetLearningManager(this);

		if (bPresetActive)
		{
			Trainer->SetRewardVariant(PresetCfg.Variant);
		}

		TrainingSettings.RandomSeed = ExperimentSeed;
		FMath::RandInit(ExperimentSeed);
	}
	
	for (AMLNPCCharacter* NPC : NPCAgents)
	{
		if (NPC)
		{
			int32 Id = Manager->AddAgent(NPC);
			//UE_LOG(LogTemp, Warning, TEXT("Registered Agent %d: %s"), Id, *NPC->GetName());
		}
	}

	if (bInferenceMode || bLoadSnapshotOnBeginPlay)
	{
		LoadNetworks();
	}

	if (bEnableEpisodeLog && !bInferenceMode)
	{
		const FString LogDir = FPaths::ProjectSavedDir() / TEXT("Logs") / TEXT("RL");
		IFileManager& FileManager = IFileManager::Get();
		if (!FileManager.DirectoryExists(*LogDir))
		{
			FileManager.MakeDirectory(*LogDir, /*Tree=*/true);
		}
		const FString TimeStamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
		EpisodeLogPath = LogDir / FString::Printf(TEXT("RL_%s_%s.csv"), *ExperimentTag, *TimeStamp);

		const FString Header = TEXT("EpisodeIdx,AgentId,Steps,Return,Success\n");
		FFileHelper::SaveStringToFile(Header, *EpisodeLogPath);
		UE_LOG(LogTemp, Log, TEXT("Episode log: %s"), *EpisodeLogPath);
	}
}

void ANPCLearningManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Manager를 강제로 재생성해서 MaxAgentNum 보장
	/*if (Manager)
	{
		Manager->DestroyComponent();
	}
	Manager = NewObject<UNPCMLManager>(this, TEXT("Manager"));

	FProperty* Prop = UNPCMLManager::StaticClass()->FindPropertyByName(TEXT("MaxAgentNum"));
	if (Prop)
	{
		int32 Value = 20;
		Prop->SetValue_InContainer(Manager, &Value);
	}
	Manager->RegisterComponent();*/
}

// Called every frame
void ANPCLearningManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Log, TEXT("Manager Tick - Running Training"));

	if (!Manager || !Policy)
	{
		return;
	}

	if(bInferenceMode)
	{
		Policy->RunInference();
	}
	else
	{
		if (!Trainer)
		{
			return;
		}
		Trainer->RunTraining(TrainingSettings);
	}
}

void ANPCLearningManager::SaveNetworks()
{
	const FString BasePath = FPaths::ProjectSavedDir() / SnapshotDir;

	IFileManager& FileManager = IFileManager::Get();
	if (!FileManager.DirectoryExists(*BasePath))
	{
		FileManager.MakeDirectory(*BasePath, /*Tree=*/true);
	}

	if (Policy)
	{
		if (ULearningAgentsNeuralNetwork* PolicyNet = Policy->GetPolicyNetworkAsset())
		{
			FFilePath PolicyPath;
			PolicyPath.FilePath = BasePath / TEXT("Policy");
			PolicyNet->SaveNetworkToSnapshot(PolicyPath);
		}
	}
	if (Critic)
	{
		if (ULearningAgentsNeuralNetwork* CriticNet = Critic->GetCriticNetworkAsset())
		{
			FFilePath CriticPath;
			CriticPath.FilePath = BasePath / TEXT("Critic");
			CriticNet->SaveNetworkToSnapshot(CriticPath);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Networks saved to %s"), *BasePath);
}

void ANPCLearningManager::LoadNetworks()
{
	const FString BasePath = FPaths::ProjectSavedDir() / SnapshotDir;

	if (Policy)
	{
		if (ULearningAgentsNeuralNetwork* PolicyNet = Policy->GetPolicyNetworkAsset())
		{
			FFilePath PolicyPath;
			PolicyPath.FilePath = BasePath / TEXT("Policy");
			PolicyNet->LoadNetworkFromSnapshot(PolicyPath);
		}
	}
	if (Critic)
	{
		if (ULearningAgentsNeuralNetwork* CriticNet = Critic->GetCriticNetworkAsset())
		{
			FFilePath CriticPath;
			CriticPath.FilePath = BasePath / TEXT("Critic");
			CriticNet->LoadNetworkFromSnapshot(CriticPath);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Networks loaded from %s"), *BasePath);
}

void ANPCLearningManager::OnEpisodeComplete(int32 AgentId, int32 EpisodeSteps, float EpisodeReturn, bool bSuccess)
{
	CompletedEpisodes++;

	if (bEnableEpisodeLog && !EpisodeLogPath.IsEmpty())
	{
		const FString Row = FString::Printf(TEXT("%d,%d,%d,%.4f,%d\n"),
			CompletedEpisodes, AgentId, EpisodeSteps, EpisodeReturn, bSuccess ? 1 : 0);
		FFileHelper::SaveStringToFile(Row, *EpisodeLogPath,
			FFileHelper::EEncodingOptions::AutoDetect,
			&IFileManager::Get(),
			FILEWRITE_Append);
	}

	if (SaveIntervalEpisodes > 0 && CompletedEpisodes % SaveIntervalEpisodes == 0)
	{
		SaveNetworks();
		UE_LOG(LogTemp, Log, TEXT("Auto-saved at episode %d"), CompletedEpisodes);
	}
}

