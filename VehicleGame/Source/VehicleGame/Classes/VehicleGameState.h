// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGameState.generated.h"

UCLASS()
class AVehicleGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

	/** number of teams in current game */
	UPROPERTY(Transient, Replicated)
	int32 NumRacers;

	/** total race time */
	UPROPERTY(Transient, Replicated)
		float TotalTime;

	/** is timer paused? */
	UPROPERTY(Transient, Replicated)
		bool bTimerPaused;

	/** is timer paused? */
	UPROPERTY(Transient, Replicated)
		bool bIsRaceActive;

	UFUNCTION(BlueprintCallable, Category = Game)
		float GetTotalTime();

	UFUNCTION(BlueprintCallable, Category = Game)
		bool IsRaceActive() const;
};
