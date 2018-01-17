// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "CompGameMode.h"
#include "CompCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACompGameMode::ACompGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
