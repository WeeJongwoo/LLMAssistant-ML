// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCTrainer.h"
#include "LLMAssistant/NPC/MLNPCCharacter.h"
#include "LearningAgentsRewards.h"
#include "LearningAgentsCompletions.h"


void UNPCTrainer::GatherAgentReward_Implementation(float& OutReward, const int32 AgentId)
{
    AMLNPCCharacter* NPC = Cast<AMLNPCCharacter>(GetAgent(AgentId));
    if (!NPC || !GoalActor)
    {
        OutReward = 0.f;
        return;
    }

    const FVector Loc = NPC->GetActorLocation();
    const float DistToGoal = FVector::Dist(Loc, GoalActor->GetActorLocation());

    const float PrevDist = PrevDistMap.FindRef(AgentId);
    const float ApproachReward = (PrevDist - DistToGoal) * 0.01f;
    PrevDistMap.Add(AgentId, DistToGoal);

    const float TimeReward = -0.001f;

    const float GoalReward = (DistToGoal < 100.f) ? 10.f : 0.f;

    OutReward = ApproachReward + TimeReward + GoalReward;
}

void UNPCTrainer::GatherAgentCompletion_Implementation(ELearningAgentsCompletion& OutCompletion, const int32 AgentId)
{
    int32& Steps = StepCountMap.FindOrAdd(AgentId, 0);
    Steps++;

    AMLNPCCharacter* NPC = Cast<AMLNPCCharacter>(GetAgent(AgentId));
    if (!NPC || !GoalActor)
    {
        OutCompletion = ELearningAgentsCompletion::Truncation;
        return;
    }

    const float DistToGoal = FVector::Dist(NPC->GetActorLocation(), GoalActor->GetActorLocation());

    const ELearningAgentsCompletion GoalCompletion =
        ULearningAgentsCompletions::MakeCompletionOnCondition(DistToGoal < 100.f, ELearningAgentsCompletion::Termination, TEXT("GoalReached"));

    const ELearningAgentsCompletion TimeoutCompletion =
        ULearningAgentsCompletions::MakeCompletionOnCondition(Steps >= MaxSteps, ELearningAgentsCompletion::Truncation, TEXT("Timeout"));

    OutCompletion = ULearningAgentsCompletions::CompletionOr(GoalCompletion, TimeoutCompletion);
}

void UNPCTrainer::ResetAgentEpisode_Implementation(const int32 AgentId)
{
    AMLNPCCharacter* NPC = Cast<AMLNPCCharacter>(GetAgent(AgentId));
    if (!NPC || !GoalActor) return;

    NPC->ResetToStart();

    PrevDistMap.Add(AgentId, FVector::Dist(NPC->GetActorLocation(), GoalActor->GetActorLocation()));

    StepCountMap.Add(AgentId, 0);

    if (LearningManager.IsValid())
    {
        LearningManager->OnEpisodeComplete();
    }
}

void UNPCTrainer::OnAgentsAdded_Implementation(const TArray<int32>& AgentIds)
{
    Super::OnAgentsAdded_Implementation(AgentIds);
    for(const int32 AgentId : AgentIds)
    {
        AMLNPCCharacter* NPC = Cast<AMLNPCCharacter>(GetAgent(AgentId));
        if (!NPC || !GoalActor) continue;

        PrevDistMap.Add(AgentId, FVector::Dist(NPC->GetActorLocation(), GoalActor->GetActorLocation()));
        StepCountMap.Add(AgentId, 0);
    }
}
