// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehiclePlayerController.generated.h"

UCLASS()
class AVehiclePlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	/** checkpoint on track */
	UPROPERTY(BlueprintReadWrite, transient, Category=Game)
	class AVehicleTrackPoint* LastTrackPoint;
	
	// Begin PlayerController overrides
	virtual void UnFreeze() override;
	// End PlayerController overrides

	/** notify about touching new checkpoint */
	void OnTrackPointReached(class AVehicleTrackPoint* TrackPoint);

	/** toggles in game menu */
	void OnToggleInGameMenu();

	/** is handbrake forced? */
	bool IsHandbrakeForced() const;
	
	/** set handbrake forced */
	void SetHandbrakeForced(bool bNewForced);

	void Suicide();

	//////////////////////////////////////////////////////////////////////////
	// Replication

	/** start death on server */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSuicide();

protected:

	/** if set, handbrake will be forced */
	UPROPERTY(transient, replicated)
	bool bHandbrakeOverride;

	virtual void SetupInputComponent() override;
};
