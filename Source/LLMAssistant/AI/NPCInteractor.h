// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LearningAgentsInteractor.h"
#include "NPCInteractor.generated.h"

/**
 * 
 */
UCLASS()
class LLMASSISTANT_API UNPCInteractor : public ULearningAgentsInteractor
{
	GENERATED_BODY()
	
	
protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> GoalActor;

	float TraceRange = 200.0f;

protected:
	
	virtual void SpecifyAgentObservation_Implementation(FLearningAgentsObservationSchemaElement& OutObservationSchemaElement, ULearningAgentsObservationSchema* InObservationSchema) override;
	
	virtual void GatherAgentObservation_Implementation(FLearningAgentsObservationObjectElement& OutObservationObjectElement, ULearningAgentsObservationObject* InObservationObject, const int32 AgentId) override;

	virtual void SpecifyAgentAction_Implementation(FLearningAgentsActionSchemaElement& OutActionSchemaElement, ULearningAgentsActionSchema* InActionSchema) override;

	virtual void PerformAgentAction_Implementation(const ULearningAgentsActionObject* InActionObject, const FLearningAgentsActionObjectElement& InActionObjectElement, const int32 AgentId) override;

public:
	void SetGoalActor(AActor* InGoalActor) { GoalActor = InGoalActor; }
};
