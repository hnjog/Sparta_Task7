// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverController.h"
#include "EnhancedInputSubsystems.h"

AHoverController::AHoverController()
	:InputMappingContext(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr),
	UpAction(nullptr),
	DownAction(nullptr),
	RotateZAction(nullptr)
{
}

void AHoverController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* LocalSubSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (nullptr != InputMappingContext)
			{
				// 높은 우선순위 설정
				LocalSubSystem->AddMappingContext(InputMappingContext, 0);
			}
		}
	}
}