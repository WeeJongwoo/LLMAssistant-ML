// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCTrainer.h"
#include "LLMAssistant/NPC/MLNPCCharacter.h"
#include "LearningAgentsRewards.h"
#include "LearningAgentsCompletions.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"


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
    const float   StraightDist = FVector::Dist(Loc, GoalLoc);

    float PathDist = GetPathDistanceToGoal(Loc, GoalLoc);
    if (PathDist < 0.f) { PathDist = StraightDist; }

    const float PrevPathDist = PrevPathDistMap.Contains(AgentId) ? PrevPathDistMap[AgentId] : PathDist;
    PrevPathDistMap.Add(AgentId, PathDist);

    // 경로거리가 줄어든 만큼 보상 (벽 우회해도 정상적으로 +)
    const float ProgressReward = (PrevPathDist - PathDist) * 0.01f;
    const float TimeReward = -0.001f;
    const float GoalReward = (StraightDist < 100.f) ? 10.f : 0.f;

    // 거의 안 움직였으면(벽에 끼임) 페널티
    const FVector PrevLoc = PrevLocMap.Contains(AgentId) ? PrevLocMap[AgentId] : Loc;
    PrevLocMap.Add(AgentId, Loc);
    const float StuckPenalty = (FVector::Dist(Loc, PrevLoc) < 1.f) ? -0.01f : 0.f;

    OutReward = ProgressReward + TimeReward + GoalReward + StuckPenalty;
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

    //PrevDistMap.Add(AgentId, FVector::Dist(NPC->GetActorLocation(), GoalActor->GetActorLocation()));

    const float InitPath = GetPathDistanceToGoal(NPC->GetActorLocation(), GoalActor->GetActorLocation());
    PrevPathDistMap.Add(AgentId, InitPath < 0.f ? FVector::Dist(NPC->GetActorLocation(), GoalActor->GetActorLocation()) : InitPath);
    PrevLocMap.Add(AgentId, NPC->GetActorLocation());

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

        //PrevDistMap.Add(AgentId, FVector::Dist(NPC->GetActorLocation(), GoalActor->GetActorLocation()));

        const float InitPath = GetPathDistanceToGoal(NPC->GetActorLocation(), GoalActor->GetActorLocation());
        PrevPathDistMap.Add(AgentId, InitPath < 0.f ? FVector::Dist(NPC->GetActorLocation(), GoalActor->GetActorLocation()) : InitPath);
        PrevLocMap.Add(AgentId, NPC->GetActorLocation());

        StepCountMap.Add(AgentId, 0);
    }
}

float UNPCTrainer::GetPathDistanceToGoal(const FVector& Start, const FVector& End) const
{
    UWorld* World = GetWorld();
    if (!World) return -1.f;

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (!NavSys) return -1.f;

    UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(World, Start, End);
    if (Path && Path->IsValid() && !Path->IsPartial())
    {
        return Path->GetPathLength();
    }
    return -1.f;
}
