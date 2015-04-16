// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGame.h"

UVehicleBlueprintLibrary::UVehicleBlueprintLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

AVehicleGameMode* GetGameFromContextObject(class UObject* WorldContextObject)
{
	UWorld* const MyWorld = GEngine->GetWorldFromContextObject(WorldContextObject);
	check(MyWorld);

	AVehicleGameMode* const MyGame = MyWorld->GetAuthGameMode<AVehicleGameMode>();
	return MyGame;
}

void UVehicleBlueprintLibrary::FinishRace(class UObject* WorldContextObject)
{
	AVehicleGameMode* const MyGame = GetGameFromContextObject(WorldContextObject);
	if (MyGame)
	{
		MyGame->FinishRace();
	}
}

void UVehicleBlueprintLibrary::SetInfoText(class UObject* WorldContextObject, FString InfoText)
{
	AVehicleGameMode* const MyGame = GetGameFromContextObject(WorldContextObject);
	if (MyGame)
	{
		MyGame->SetGameInfoText(InfoText);
	}
}

 void UVehicleBlueprintLibrary::HideInfoText(class UObject* WorldContextObject)
 {
 	AVehicleGameMode* const MyGame = GetGameFromContextObject(WorldContextObject);
	if (MyGame)
	{
		MyGame->SetGameInfoText(FString());
	}
 }

 void UVehicleBlueprintLibrary::ShowGameMenu(class UObject* WorldContextObject)
 {
	 UWorld* const MyWorld = GEngine->GetWorldFromContextObject(WorldContextObject);
	 APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(MyWorld);
	 AVehicleHUD* MyHUD = LocalPC ? Cast<AVehicleHUD>(LocalPC->GetHUD()) : NULL;
	 if (MyHUD)
	 {
		 if (!MyHUD->IsGameMenuUp())
		 {
			 MyHUD->ToggleGameMenu();
		 }
	 }
 }

 void UVehicleBlueprintLibrary::ShowHUD(class UObject* WorldContextObject, bool bEnable)
 {
	 UWorld* const MyWorld = GEngine->GetWorldFromContextObject(WorldContextObject);
	 APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(MyWorld);
	 AVehicleHUD* MyHUD = LocalPC ? Cast<AVehicleHUD>(LocalPC->GetHUD()) : NULL;
	 if (MyHUD)
	 {
		 MyHUD->EnableHUD(bEnable);
	 }
 }


