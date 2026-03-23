// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MLNPCCharacter.generated.h"

UENUM(BlueprintType)
enum class ENPCAction : uint8
{
	Walk	= 0,
	Jump	= 1,
	ClouchWalk = 2
};


UCLASS()
class LLMASSISTANT_API AMLNPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMLNPCCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FVector StartLocation;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> TraceStartForCrouch;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USceneComponent> TraceStartForJump;


	float StandingTraceHeight = 0.f;
	float JumpTraceHeight = 0.f;           
	float CrouchTraceHeight = 0.f;         

public:	
	// Called every frame
	void ExecuteAction(ENPCAction Action, const FVector& MoveDirection);
	void ResetToStart();

	float GetStandingTraceHeight() const { return StandingTraceHeight; }
	float GetJumpTraceHeight() const { return JumpTraceHeight; }
	float GetCrouchTraceHeight() const { return CrouchTraceHeight; }
};
