// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGame.h"

AVehicleGame_Menu::AVehicleGame_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	HUDClass = AVehicleHUD_Menu::StaticClass();
}

void AVehicleGame_Menu::RestartPlayer(class AController* NewPlayer)
{
	// don't restart
}
