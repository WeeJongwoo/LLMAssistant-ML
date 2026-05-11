// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LearningAgentsTrainer.h"
#include "NPCLearningManager.generated.h"

class ULearningAgentsManager;
class UNPCInteractor;
class UNPCTrainer;
class ULearningAgentsPolicy;
class ULearningAgentsCritic;
class AMLNPCCharacter;
class ULearningAgentsNeuralNetwork;
class UNPCMLManager;

UENUM(BlueprintType)
enum class EExperimentPreset : uint8
{
	Manual           UMETA(DisplayName = "Manual (use individual fields)"),
	R1_Seed1234      UMETA(DisplayName = "R1 Sparse - Seed 1234"),
	R1_Seed5678      UMETA(DisplayName = "R1 Sparse - Seed 5678"),
	R1_Seed9999      UMETA(DisplayName = "R1 Sparse - Seed 9999"),
	R2_Seed1234      UMETA(DisplayName = "R2 Dense - Seed 1234"),
	R2_Seed5678      UMETA(DisplayName = "R2 Dense - Seed 5678"),
	R2_Seed9999      UMETA(DisplayName = "R2 Dense - Seed 9999"),
	R3_Seed1234      UMETA(DisplayName = "R3 Dense+OriCost - Seed 1234"),
	R3_Seed5678      UMETA(DisplayName = "R3 Dense+OriCost - Seed 5678"),
	R3_Seed9999      UMETA(DisplayName = "R3 Dense+OriCost - Seed 9999")
};

UCLASS()
class LLMASSISTANT_API ANPCLearningManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANPCLearningManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Learning")
	TObjectPtr<AActor> GoalActor;

	UPROPERTY(EditAnywhere, Category = "Learning")
	TArray<TObjectPtr<AMLNPCCharacter>> NPCAgents;

	UPROPERTY(EditAnywhere, Category = "Learning")
	TObjectPtr<ULearningAgentsNeuralNetwork> PolicyNetworkAsset;

	UPROPERTY(EditAnywhere, Category = "Learning")
	TObjectPtr<ULearningAgentsNeuralNetwork> CriticNetworkAsset;

	UPROPERTY(EditAnywhere) 
	TObjectPtr<UNPCMLManager> Manager;

	UPROPERTY() 
	TObjectPtr<UNPCInteractor> Interactor;

	UPROPERTY(EditAnywhere, Category = "Learning")
	TObjectPtr<UNPCTrainer> Trainer;

	UPROPERTY() 
	TObjectPtr<ULearningAgentsPolicy> Policy;

	UPROPERTY() 
	TObjectPtr<ULearningAgentsCritic> Critic;

	UPROPERTY(EditAnywhere, Category = "Learning")
	bool bReinitializeCritic = true;

	UPROPERTY(EditAnywhere, Category = "Learning")
	bool bReinitializePolicy = true;


	UPROPERTY(EditAnywhere, Category = "Learning")
	bool bInferenceMode = false;  // true면 추론 모드로 시작

	UPROPERTY(EditAnywhere, Category = "Learning")
	bool bLoadSnapshotOnBeginPlay = false;  // 훈련 이어서 시작할 때 체크

	UPROPERTY(EditAnywhere, Category = "Learning")
	FString SnapshotDir = TEXT("NPCSnapshots");  // 저장 디렉토리

	UPROPERTY(EditAnywhere, Category = "Learning")
	int32 SaveIntervalEpisodes = 100;  // N 에피소드마다 자동 저장

	int32 CompletedEpisodes = 0;

	// 실험 프리셋 — Manual이 아니면 아래 ExperimentSeed/Tag/SnapshotDir/RewardVariant 자동 적용
	UPROPERTY(EditAnywhere, Category = "Experiment")
	EExperimentPreset ExperimentPreset = EExperimentPreset::Manual;

	// CSV 에피소드 로그 (실험 분석용)
	UPROPERTY(EditAnywhere, Category = "Experiment")
	bool bEnableEpisodeLog = true;

	UPROPERTY(EditAnywhere, Category = "Experiment")
	FString ExperimentTag = TEXT("R2_Seed0");  // 파일명 식별자: <Variant>_<Seed>

	UPROPERTY(EditAnywhere, Category = "Experiment", meta = (ClampMin = "0"))
	int32 ExperimentSeed = 1234;  // 학습 재현용 시드 (변형당 3개로 변경하며 반복)

	FString EpisodeLogPath;

	FLearningAgentsTrainerTrainingSettings TrainingSettings;

public:
	UFUNCTION(BlueprintCallable, Category = "Learning")
	void SaveNetworks();

	UFUNCTION(BlueprintCallable, Category = "Learning")
	void LoadNetworks();

	void OnEpisodeComplete(int32 AgentId, int32 EpisodeSteps, float EpisodeReturn, bool bSuccess);
};
