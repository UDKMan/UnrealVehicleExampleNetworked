// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRaceStartingDelegate);

// Forward declarations
class AVehicleGameState;

UCLASS()
class AVehicleGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

	/** Starts race */
	UFUNCTION(BlueprintCallable, Category=Game)
	void StartRace();

	/** Finishes race */
	void FinishRace();

	/** Lock movement of newly logged in players if race is not active */
	void EnablePlayerLocking();

	// Begin AGameMode interface
	virtual AActor* ChoosePlayerStart(AController* Player) override;
	virtual class AActor* FindPlayerStart(AController* Player, const FString& IncomingName = TEXT("")) override;
	virtual APawn* SpawnDefaultPawnFor(AController* NewPlayer, class AActor* StartSpot) override;
	virtual void Tick(float DeltaSeconds) override;
	// End AGameMode interface

	/** Check if race is active */
	bool IsRaceActive() const;

	/** Check if race has started */
	bool HasRaceStarted() const;

	/** Check if race has finished */
	bool HasRaceFinished() const;

	/** Get time elapsed from race start */
	UFUNCTION(BlueprintCallable, Category=Game)
	float GetRaceTimer() const;

	/* 
	 * Set the information text at the bottom of the screen 
	 *
	 * @param	InString	String to set
	 */
	void SetGameInfoText(const FString& InString);

	/*
	* Set the information text at the bottom of the screen
	*
	* @param	InString	String to set
	*/
	const FString& GetGameInfoText() const;

	/** 
	 * Set the pause state of the game.
	 * 
	 * @param	bIsPaused	The required pause state
	 */
	UFUNCTION(BlueprintCallable, Category=Game)
	void SetGamePaused(bool bIsPaused);

	/* 
	 * Returns a cast version of the game state. Will return NULL if the game state is NOT of type AVehicleGameState.
	 */
	UFUNCTION(BlueprintCallable, Category = Game)
	AVehicleGameState* GetVehicleGameState() const;

protected:

	/** Information text at the bottom of the screen */
	FString GameInfoText;

	/** Timestamp of race start */
	float RaceStartTime;

	/** Timestamp of race finish */
	float RaceFinishTime;

	/** Is player locking active? */
	bool bLockingActive;

	/* Lock all players until race starts */
	virtual void HandleMatchHasStarted() override;

	/** Lock player movement if needed */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Notify all clients about race state */
	void BroadcastRaceState();

	/** Delegate to broadcast about race starting */
	UPROPERTY(BlueprintAssignable)
	FRaceStartingDelegate OnRaceStarting;
};
