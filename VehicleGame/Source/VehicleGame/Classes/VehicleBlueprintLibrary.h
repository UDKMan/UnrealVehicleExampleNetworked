// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleBlueprintLibrary.generated.h"

UCLASS()
class UVehicleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/** Finishes the race */
	UFUNCTION(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
	static void FinishRace(class UObject* WorldContextObject);

	/** Shows blinking message at the bottom of the screen */
	UFUNCTION(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
	static void SetInfoText(class UObject* WorldContextObject, FString InfoText);

	/** Hides info message */
	UFUNCTION(BlueprintCallable, Category=Game, meta=(WorldContext="WorldContextObject"))
	static void HideInfoText(class UObject* WorldContextObject);

	/** shows game menu */
	UFUNCTION(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
	static void ShowGameMenu(class UObject* WorldContextObject);

	/** shows/hides game HUD */
	UFUNCTION(BlueprintCallable, Category=HUD, meta=(WorldContext="WorldContextObject"))
	static void ShowHUD(class UObject* WorldContextObject, bool bEnable);
};
