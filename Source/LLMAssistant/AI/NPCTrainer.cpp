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
    const FVector GoalLoc = GoalActor->GetActorLocation();
    const float DistToGoal = FVector::Dist(Loc, GoalLoc);

    const float PrevDist = PrevDistMap.FindRef(AgentId);
    PrevDistMap.Add(AgentId, DistToGoal);

    const float GoalReward = (DistToGoal < 100.f) ? 10.f : 0.f;

    switch (RewardVariant)
    {
    case ERewardVariant::R1_Sparse:
    {
        const float TimeReward = -0.001f;
        OutReward = TimeReward + GoalReward;
        break;
    }
    case ERewardVariant::R2_Dense:
    {
        const float ApproachReward = (PrevDist - DistToGoal) * 0.01f;
        const float TimeReward = -0.001f;
        OutReward = ApproachReward + TimeReward + GoalReward;
        break;
    }
    case ERewardVariant::R3_DenseOrientationCost:
    {
        const float ApproachReward = (PrevDist - DistToGoal) * 0.01f;
        const float TimeReward = -0.001f;

        FVector ToGoal = GoalLoc - Loc;
        ToGoal.Z = 0.f;
        ToGoal.Normalize();
        FVector Forward = NPC->GetActorForwardVector();
        Forward.Z = 0.f;
        Forward.Normalize();
        const float OrientationBonus = FVector::DotProduct(Forward, ToGoal) * 0.005f;

        const float ActionCost = (NPC->GetLastAction() == ENPCAction::Jump) ? -0.002f : 0.f;

        OutReward = ApproachReward + TimeReward + GoalReward + OrientationBonus + ActionCost;
        break;
    }
    default:
        OutReward = GoalReward;
        break;
    }

    EpisodeReturnMap.FindOrAdd(AgentId, 0.f) += OutReward;
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

    const bool bGoalReached = DistToGoal < 100.f;
    const bool bTimeout = Steps >= MaxSteps;

    const ELearningAgentsCompletion GoalCompletion =
        ULearningAgentsCompletions::MakeCompletionOnCondition(bGoalReached, ELearningAgentsCompletion::Termination, TEXT("GoalReached"));

    const ELearningAgentsCompletion TimeoutCompletion =
        ULearningAgentsCompletions::MakeCompletionOnCondition(bTimeout, ELearningAgentsCompletion::Truncation, TEXT("Timeout"));

    OutCompletion = ULearningAgentsCompletions::CompletionOr(GoalCompletion, TimeoutCompletion);

    if (bGoalReached || bTimeout)
    {
        LastSuccessMap.Add(AgentId, bGoalReached);
        NaturalCompletionMap.Add(AgentId, true);
    }
}

void UNPCTrainer::ResetAgentEpisode_Implementation(const int32 AgentId)
{
    AMLNPCCharacter* NPC = Cast<AMLNPCCharacter>(GetAgent(AgentId));
    if (!NPC || !GoalActor) return;

    const int32 EpisodeSteps = StepCountMap.FindRef(AgentId);
    const float EpisodeReturn = EpisodeReturnMap.FindRef(AgentId);
    const bool bSuccess = LastSuccessMap.FindRef(AgentId);
    const bool bNatural = NaturalCompletionMap.FindRef(AgentId);

    NPC->ResetToStart();

    PrevDistMap.Add(AgentId, FVector::Dist(NPC->GetActorLocation(), GoalActor->GetActorLocation()));
    StepCountMap.Add(AgentId, 0);
    EpisodeReturnMap.Add(AgentId, 0.f);
    LastSuccessMap.Add(AgentId, false);
    NaturalCompletionMap.Add(AgentId, false);

    // 정책 업데이트로 인한 강제 리셋(중도 끊긴 에피소드)은 분석 노이즈이므로 로그 제외
    if (LearningManager.IsValid() && EpisodeSteps > 0 && bNatural)
    {
        LearningManager->OnEpisodeComplete(AgentId, EpisodeSteps, EpisodeReturn, bSuccess);
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
