// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsTrainer.h"
#include "NPCLearningManager.h"
#include "NPCTrainer.generated.h"


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

	// 에이전트별 이전 목표 거리
	TMap<int32, float> PrevDistMap;

	// 에이전트별 스텝 카운터
	TMap<int32, int32> StepCountMap;

	static constexpr int32 MaxSteps = 1000;

	TWeakObjectPtr<ANPCLearningManager> LearningManager;

protected:

	virtual void GatherAgentReward_Implementation(float& OutReward, const int32 AgentId) override;

	virtual void GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion, const int32 AgentId) override;

	virtual void ResetAgentEpisode_Implementation(const int32 AgentId) override;

	virtual void OnAgentsAdded_Implementation(const TArray<int32>& AgentIds) override;

public:
	void SetGoalActor(AActor* InGoalActor) { GoalActor = InGoalActor; }

	void SetLearningManager(ANPCLearningManager* InManager) { LearningManager = InManager; }
};
