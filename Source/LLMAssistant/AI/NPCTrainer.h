// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsTrainer.h"
#include "NPCLearningManager.h"
#include "NPCTrainer.generated.h"


UENUM(BlueprintType)
enum class ERewardVariant : uint8
{
	R1_Sparse                  UMETA(DisplayName = "R1 - Sparse (Goal only)"),
	R2_Dense                   UMETA(DisplayName = "R2 - Dense (Approach + Time + Goal)"),
	R3_DenseOrientationCost    UMETA(DisplayName = "R3 - Dense + Orientation + ActionCost")
};


/**
 *
 */
UCLASS()
class LLMASSISTANT_API UNPCTrainer : public ULearningAgentsTrainer
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> GoalActor;

	UPROPERTY(EditAnywhere, Category = "Reward")
	ERewardVariant RewardVariant = ERewardVariant::R2_Dense;

	// 에이전트별 이전 목표 거리
	TMap<int32, float> PrevDistMap;

	// 에이전트별 스텝 카운터
	TMap<int32, int32> StepCountMap;

	// 에이전트별 에피소드 누적 보상
	TMap<int32, float> EpisodeReturnMap;

	// 에이전트별 마지막 에피소드 성공 여부
	TMap<int32, bool> LastSuccessMap;

	// 에이전트별 마지막 에피소드가 자연 종료(성공/Timeout)되었는지 여부
	TMap<int32, bool> NaturalCompletionMap;

	static constexpr int32 MaxSteps = 800;

	TWeakObjectPtr<ANPCLearningManager> LearningManager;

protected:

	virtual void GatherAgentReward_Implementation(float& OutReward, const int32 AgentId) override;

	virtual void GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion, const int32 AgentId) override;

	virtual void ResetAgentEpisode_Implementation(const int32 AgentId) override;

	virtual void OnAgentsAdded_Implementation(const TArray<int32>& AgentIds) override;

public:
	void SetGoalActor(AActor* InGoalActor) { GoalActor = InGoalActor; }

	void SetLearningManager(ANPCLearningManager* InManager) { LearningManager = InManager; }

	void SetRewardVariant(ERewardVariant InVariant) { RewardVariant = InVariant; }

	ERewardVariant GetRewardVariant() const { return RewardVariant; }
};
