// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "EnhancedInputComponent.h"
#include "TaskPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ATaskCharacter::ATaskCharacter()
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

	ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Resource/Robot_scout_R_21/Mesh/SK_Robot_scout_R21.SK_Robot_scout_R21"));
	if (MeshAsset.Succeeded())
	{
		Mesh->SetSkeletalMesh(MeshAsset.Object);
		Mesh->SetRelativeLocation(FVector(0.0, 0.0, -90.0));
		Mesh->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));

		// Anim Class를 찾아 세팅
		// Class를 찾는것이기에 마지막에 _C 를 붙인다!
		ConstructorHelpers::FClassFinder<UAnimInstance> MeshAnimAsset(TEXT("/Game/Resource/Robot_scout_R_21/Demo/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"));
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
	SpringArmComp->TargetOffset.Set(0.0, 0.0, 150.0);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

	MoveSpeed = 600.0f;
	RotateSpeed = 300.0f;
}

// Called when the game starts or when spawned
void ATaskCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	MoveVec = FVector::Zero();
}

void ATaskCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MoveVec.Size() > 0)
	{
		FVector TempMoveVec = MoveVec.GetSafeNormal() * MoveSpeed * DeltaTime;

		AddActorLocalOffset(TempMoveVec);
		MoveVec = FVector::Zero();
	}

	if (FMath::IsNearlyZero(RotateR.Yaw) == false)
	{
		AddActorLocalRotation(FRotator(0.0,RotateR.Yaw * RotateSpeed * DeltaTime,0.0));
	}

	if (FMath::IsNearlyZero(RotateR.Pitch) == false)
	{
		FRotator SpringRot = SpringArmComp->GetRelativeRotation() + FRotator(RotateR.Pitch * RotateSpeed * DeltaTime,0.0 , 0.0);
		SpringRot.Pitch = FMath::Clamp(SpringRot.Pitch, -40.0, 60.0);

		SpringArmComp->SetRelativeRotation(SpringRot);
	}

	RotateR = FRotator::ZeroRotator;
}

// Called to bind functionality to input
void ATaskCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ATaskPlayerController* PlayerController = Cast<ATaskPlayerController>(GetController()))
		{
			if (UInputAction* MoveAction = PlayerController->GetMoveAction())
			{
				EnhancedInput->BindAction(
					MoveAction,
					ETriggerEvent::Triggered,
					this,
					&ATaskCharacter::Move
				);
			}

			if (UInputAction* LookAction = PlayerController->GetLookAction())
			{
				EnhancedInput->BindAction(
					LookAction,
					ETriggerEvent::Triggered,
					this,
					&ATaskCharacter::Look
				);
			}
		}
	}
}

void ATaskCharacter::Move(const FInputActionValue& value)
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

void ATaskCharacter::Look(const FInputActionValue& value)
{
	const FVector2D& LookValue = value.Get<FVector2D>();
	RotateR = FRotator(-LookValue.Y, LookValue.X, 0.0);
}

