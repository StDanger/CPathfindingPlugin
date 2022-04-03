// Copyright Epic Games, Inc. All Rights Reserved.

#include "CPathStartGameMode.h"
#include "CPathStartCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACPathStartGameMode::ACPathStartGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
