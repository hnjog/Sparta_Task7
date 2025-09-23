// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskHoverPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "EnhancedInputComponent.h"
#include "HoverController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ATaskHoverPawn::ATaskHoverPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetCapsuleHalfHeight(88.0f);
	CapsuleComponent->SetCapsuleRadius(34.0f);
	CapsuleComponent->SetSimulatePhysics(false);
	SetRootComponent(CapsuleComponent);

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(CapsuleComponent);
	Mesh->SetSimulatePhysics(false);

	ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Resource/Drone_Scavenger/SK/SK_Drone_Scavenger.SK_Drone_Scavenger"));
	if (MeshAsset.Succeeded())
	{
		Mesh->SetSkeletalMesh(MeshAsset.Object);
		Mesh->SetRelativeLocation(FVector(0.0, 0.0, 0.0));
		Mesh->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));

		// Anim Class를 찾아 세팅
		// Class를 찾는것이기에 마지막에 _C 를 붙인다!
		ConstructorHelpers::FClassFinder<UAnimInstance> MeshAnimAsset(TEXT("/Game/Resource/Drone_Scavenger/Animations/ABP_Drone.ABP_Drone_C"));
		if (MeshAnimAsset.Succeeded())
		{
			Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			Mesh->SetAnimInstanceClass(MeshAnimAsset.Class);
		}
	}

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(CapsuleComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = false;
	SpringArmComp->TargetOffset.Set(0.0, 0.0, 0.0);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));

	MoveSpeed = 600.0f;
	RotateSpeed = 300.0f;
}

// Called when the game starts or when spawned
void ATaskHoverPawn::BeginPlay()
{
	Super::BeginPlay();
	
	MoveVec = FVector::Zero();
	RotateR = FRotator::ZeroRotator;
}

// Called every frame
void ATaskHoverPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MoveVec.Size() > 0)
	{
		FVector TempMoveVec = MoveVec.GetSafeNormal() * MoveSpeed * DeltaTime;

		AddActorLocalOffset(TempMoveVec);
		MoveVec = FVector::Zero();
	}

	// Quat 로 바꿔야할지 고민해볼 것
	if (RotateR.IsNearlyZero() == false)
	{
		AddActorLocalRotation(RotateR);
	}

	RotateR = FRotator::ZeroRotator;
}

// Called to bind functionality to input
void ATaskHoverPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AHoverController* PlayerController = Cast<AHoverController>(GetController()))
		{
			if (UInputAction* MoveAction = PlayerController->GetMoveAction())
			{
				EnhancedInput->BindAction(
					MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ATaskHoverPawn::Move
				);
			}

			if (UInputAction* LookAction = PlayerController->GetLookAction())
			{
				EnhancedInput->BindAction(
					LookAction,
					ETriggerEvent::Triggered,
					this,
					&ATaskHoverPawn::Look
				);
			}

			if (UInputAction* UpAction = PlayerController->GetUpAction())
			{
				EnhancedInput->BindAction(
					UpAction,
					ETriggerEvent::Triggered,
					this,
					&ATaskHoverPawn::UpMove
				);
			}

			if (UInputAction* DownAction = PlayerController->GetDownAction())
			{
				EnhancedInput->BindAction(
					DownAction,
					ETriggerEvent::Triggered,
					this,
					&ATaskHoverPawn::DownMove
				);
			}

			if (UInputAction* RotZAction = PlayerController->GetRotateZAction())
			{
				EnhancedInput->BindAction(
					RotZAction,
					ETriggerEvent::Triggered,
					this,
					&ATaskHoverPawn::RotateZ
				);
			}
		}
	}
}

void ATaskHoverPawn::Move(const FInputActionValue& value)
{
	if (Controller == nullptr)
		return;

	const FVector2D& MoveValue = value.Get<FVector2D>();
	const float YValue = SpringArmComp->GetRelativeRotation().Yaw;
	const FRotator YawRot(0.f, YValue, 0.f);

	if (FMath::IsNearlyZero(MoveValue.X) == false)
	{
		const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		MoveVec += (Forward * MoveValue.X);
	}

	if (FMath::IsNearlyZero(MoveValue.Y) == false)
	{
		const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		MoveVec += (Right * MoveValue.Y);
	}
}

void ATaskHoverPawn::Look(const FInputActionValue& value)
{
	const FVector2D& LookValue = value.Get<FVector2D>();
	RotateR = FRotator(-LookValue.Y, LookValue.X, 0.0);
}

void ATaskHoverPawn::UpMove(const FInputActionValue& value)
{
	const float& UpValue = value.Get<float>();
	const FRotator RollRot(0.f, 0.f, UpValue);
	if (FMath::IsNearlyZero(UpValue) == false)
	{
		const FVector Up = FRotationMatrix(RollRot).GetUnitAxis(EAxis::Z);
		MoveVec += (Up * UpValue);
	}
}

void ATaskHoverPawn::DownMove(const FInputActionValue& value)
{
	const float& DownValue = value.Get<float>();
	const FRotator RollRot(0.f, 0.f, DownValue);
	if (FMath::IsNearlyZero(DownValue) == false)
	{
		const FVector Down = FRotationMatrix(RollRot).GetUnitAxis(EAxis::Z);
		MoveVec -= (Down * DownValue);
	}
}

void ATaskHoverPawn::RotateZ(const FInputActionValue& value)
{
	const float& RotateValue = value.Get<float>();
	RotateR.Roll = -RotateValue;
}
