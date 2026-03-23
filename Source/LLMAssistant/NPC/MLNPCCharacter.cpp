// Fill out your copyright notice in the Description page of Project Settings.


#include "MLNPCCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AMLNPCCharacter::AMLNPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->NavAgentProps.bCanCrouch = true;
	MoveComp->GetNavAgentPropertiesRef().bCanJump = true;
	MoveComp->bCanWalkOffLedgesWhenCrouching = true;
	MoveComp->MaxCustomMovementSpeed = 250.0f;
	MoveComp->FallingLateralFriction = 4.5f;
	MoveComp->MaxWalkSpeed = 500.0f;


	TraceStartForCrouch = CreateDefaultSubobject<USceneComponent>(TEXT("TraceStartForCrouch"));
	TraceStartForJump = CreateDefaultSubobject<USceneComponent>(TEXT("TraceStartForJump"));

	TraceStartForCrouch->SetupAttachment(RootComponent);
	TraceStartForJump->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMLNPCCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	StartLocation = GetActorLocation();

	UCapsuleComponent* Capsule = GetCapsuleComponent();
	StandingTraceHeight = Capsule->GetScaledCapsuleHalfHeight();
	JumpTraceHeight = StandingTraceHeight + TraceStartForJump->GetRelativeLocation().Z;
	CrouchTraceHeight = StandingTraceHeight + TraceStartForCrouch->GetRelativeLocation().Z;
}

void AMLNPCCharacter::ExecuteAction(ENPCAction Action, const FVector& MoveDirection)
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();

	if (!MoveComp)
	{
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("ExecuteAction: %d, Dir: %s, Speed: %f"), (int32)Action, *MoveDirection.ToString(), GetCharacterMovement()->MaxWalkSpeed);

	/*UE_LOG(LogTemp, Warning, TEXT("[JumpDebug] Action=%d IsFalling=%d IsOnGround=%d CanJump=%d bWantsCrouch=%d MovementMode=%d Vel.Z=%.1f"),
		(int32)Action,
		MoveComp->IsFalling(),
		MoveComp->IsMovingOnGround(),
		CanJump(),
		MoveComp->bWantsToCrouch,
		(int32)MoveComp->MovementMode,
		MoveComp->Velocity.Z);*/

	if (MoveComp->IsFalling())
	{
		if (Action == ENPCAction::Jump)
		{
			MoveComp->bWantsToCrouch = false;
			Jump();
		}
		AddMovementInput(MoveDirection);
		return;
	}

	StopJumping();

	switch (Action)
	{
	case ENPCAction::Walk:
		MoveComp->bWantsToCrouch = false;
		AddMovementInput(MoveDirection);
		break;
	case ENPCAction::Jump:
		MoveComp->bWantsToCrouch = false;
		Jump();
		//AddMovementInput(MoveDirection);
		break;
	case ENPCAction::ClouchWalk:
		MoveComp->bWantsToCrouch = true;
		AddMovementInput(MoveDirection);
		break;
	default:
		break;
	}
}

void AMLNPCCharacter::ResetToStart()
{
	SetActorLocation(StartLocation);
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->bWantsToCrouch = false;
}



