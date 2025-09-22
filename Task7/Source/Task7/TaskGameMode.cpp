// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskGameMode.h"
#include "TaskCharacter.h"
#include "TaskPlayerController.h"

ATaskGameMode::ATaskGameMode()
{
	DefaultPawnClass = ATaskCharacter::StaticClass();
	PlayerControllerClass = ATaskPlayerController::StaticClass();
}
