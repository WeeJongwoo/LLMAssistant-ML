// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCLearningManager.h"
#include "LLMAssistant/NPC/MLNPCCharacter.h"
#include "NPCInteractor.h"
#include "NPCTrainer.h"
#include "LearningAgentsManager.h"
#include "LearningAgentsPolicy.h"
#include "LearningAgentsCritic.h"
#include "NPCMLManager.h"


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
	}
	
	for (AMLNPCCharacter* NPC : NPCAgents)
	{
		if (NPC)
		{
			int32 Id = Manager->AddAgent(NPC);
			//UE_LOG(LogTemp, Warning, TEXT("Registered Agent %d: %s"), Id, *NPC->GetName());
		}
	}

	if (bInferenceMode)
	{
		LoadNetworks();
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
		Trainer->RunTraining();
	}
}

void ANPCLearningManager::SaveNetworks()
{
	const FString BasePath = FPaths::ProjectSavedDir() / SnapshotDir;

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

void ANPCLearningManager::OnEpisodeComplete()
{
	CompletedEpisodes++;
	if (SaveIntervalEpisodes > 0 && CompletedEpisodes % SaveIntervalEpisodes == 0)
	{
		SaveNetworks();
		UE_LOG(LogTemp, Log, TEXT("Auto-saved at episode %d"), CompletedEpisodes);
	}
}

