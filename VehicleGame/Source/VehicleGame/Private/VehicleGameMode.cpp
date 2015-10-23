// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGame.h"
#include "Landscape.h"

AVehicleGameMode::AVehicleGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	RaceStartTime = 0;
	RaceFinishTime = 0;	
	bLockingActive = false;

	MinRespawnDelay = 0.01f;
	GameStateClass = AVehicleGameState::StaticClass();
	if ((GEngine != NULL ) && ( GEngine->GameViewport != NULL))
	{
		GEngine->GameViewport->SetSuppressTransitionMessage( true );
	}
}

void AVehicleGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	EnablePlayerLocking();
}

AActor* AVehicleGameMode::ChoosePlayerStart(AController* Player)
{
	APlayerStart* BestStart = NULL;

	// find first non occupied start
	for (int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		APlayerStart* TestSpot = PlayerStarts[i];

		bool bBlocked = false;
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			if (*It != NULL && ((*It)->GetActorLocation() - TestSpot->GetActorLocation()).SizeSquared2D() < 10000)
			{
				bBlocked = true;
				break;
			}
		}
		
		if (!bBlocked)
		{
			BestStart = TestSpot;
			break;
		}
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart(Player);
}

APawn* AVehicleGameMode::SpawnDefaultPawnFor(AController* NewPlayer, class AActor* StartSpot)
{
	check(StartSpot);
	bool bGotStart = false;
	//APawn* NewPawn = Super::SpawnDefaultPawnFor(NewPlayer, StartSpot);
	FVector StartLocation = StartSpot->GetActorLocation();
	FRotator StartRotation(ForceInit);
	StartRotation.Yaw = StartSpot->GetActorRotation().Yaw;
	FRotator EachRot = StartRotation;
	const FVector TraceOffset(0, 0, 250);
	const float CheckSize = 600.0f;

	// Check the respawn for a valid position. Initially check the first position then 4 directions from there
	for (int32 iAngle = 0; iAngle < 5; iAngle++)
	{
		FVector NewLocation = StartLocation;

		FVector RotationOffset = iAngle == 0 ? FVector::ZeroVector : FVector::ForwardVector;
		RotationOffset *= CheckSize;
		FVector NewOff = EachRot.RotateVector(RotationOffset);

		const FVector NewPos = StartLocation + NewOff;
		const FCollisionQueryParams TraceParams(TEXT("SpawnTrace"), true);
		const FVector TraceStart = NewPos + TraceOffset;
		const FVector TraceEnd = NewPos - TraceOffset;
		FHitResult Hit;

		GetWorld()->LineTraceSingle(Hit, TraceStart, TraceEnd, ECC_Vehicle, TraceParams);

		if (Hit.bBlockingHit == true)
		{
			ALandscape* LandObject = Cast<ALandscape>(Hit.Actor.Get());
			if (LandObject != nullptr)
			{
				StartLocation = NewPos;
				bGotStart = true;
				break;
			}
		}
		EachRot.Yaw += 90.0f;
	}

	// Move the spawn Z up a little so we drop onto the track
	StartLocation.Z += 150.0f;
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = Instigator;
	APawn* ResultPawn = GetWorld()->SpawnActor<APawn>(GetDefaultPawnClassForController(NewPlayer), StartLocation, StartRotation, SpawnInfo);
	check(ResultPawn != NULL);
	return ResultPawn;
}

void AVehicleGameMode::PostLogin(APlayerController* NewPlayer)
{
	AVehiclePlayerController* VehiclePC = Cast<AVehiclePlayerController>(NewPlayer);
	if (VehiclePC)
	{
		VehiclePC->SetHandbrakeForced(bLockingActive && !IsRaceActive());
		AVehicleGameState* GameState = GetVehicleGameState();
		if (GameState != NULL)
		{
			GameState->NumRacers++;
		}
	}

	Super::PostLogin(NewPlayer);
}

void AVehicleGameMode::BroadcastRaceState()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AVehiclePlayerController* VehiclePC = Cast<AVehiclePlayerController>(*It);
		if (VehiclePC)
		{
			VehiclePC->SetHandbrakeForced(bLockingActive && !IsRaceActive());
		}
	}
}

void AVehicleGameMode::StartRace()
{
	if (!IsRaceActive())
	{
		AVehicleGameState* GameState = GetGameState<AVehicleGameState>();
		if (GameState != NULL)
		{			
			GameState->bIsRaceActive = true;
		}
		RaceStartTime = GetWorld()->GetTimeSeconds();
		BroadcastRaceState();
	}
}

void AVehicleGameMode::FinishRace()
{
	if (IsRaceActive())
	{
		AVehicleGameState* GameState = GetGameState<AVehicleGameState>();
		if (GameState != NULL)
		{
			GameState->bIsRaceActive = false;
		}
		RaceFinishTime = GetWorld()->GetTimeSeconds();
		BroadcastRaceState();
	}
}

void AVehicleGameMode::Tick(float DeltaSeconds)
{
	AVehicleGameState* GameState = GetGameState<AVehicleGameState>();
	if (GameState != NULL)
	{
		const float CurrentTime = IsRaceActive() ? GetWorld()->GetTimeSeconds() : RaceFinishTime;
		GameState->TotalTime = CurrentTime - RaceStartTime;
	}
}

void AVehicleGameMode::EnablePlayerLocking()
{
	bLockingActive = true;
	BroadcastRaceState();
}

void AVehicleGameMode::SetGamePaused(bool bIsPaused)
{
	AVehiclePlayerController* const MyPlayer = Cast<AVehiclePlayerController>(GEngine->GetFirstLocalPlayerController(GetWorld()));
	if (MyPlayer != NULL)
	{
		// Cache the current pause state
		bool bWasPaused = MyPlayer->IsPaused();	
		// Apply new state
		MyPlayer->SetPause(bIsPaused);
	}
	
}

bool AVehicleGameMode::IsRaceActive() const
{
	AVehicleGameState* GameState = GetGameState<AVehicleGameState>();
	bool bIsActive = false;
	if (GameState != NULL)
	{
		bIsActive = GameState->bIsRaceActive;
	}
	return bIsActive;
}

bool AVehicleGameMode::HasRaceStarted() const
{
	return (RaceStartTime > 0.0f);
}

bool AVehicleGameMode::HasRaceFinished() const
{
	return HasRaceStarted() && !IsRaceActive();
}

float AVehicleGameMode::GetRaceTimer() const
{
	// Return the time from the game state
	AVehicleGameState* GameState = GetGameState<AVehicleGameState>();
	if (GameState != NULL)
	{
		return GameState->TotalTime;
	}
	// We shouldn't really not have a game state but just in case
	const float CurrentTime = IsRaceActive() ? GetWorld()->GetTimeSeconds() : RaceFinishTime;
	return CurrentTime - RaceStartTime;
}

AVehicleGameState* AVehicleGameMode::GetVehicleGameState() const
{
	return GetGameState<AVehicleGameState>();
}


void AVehicleGameMode::SetGameInfoText(const FString& InString)
{
	GameInfoText = InString;
}

const FString& AVehicleGameMode::GetGameInfoText() const
{
	return GameInfoText;
}


AActor* AVehicleGameMode::FindPlayerStart(AController* Player, const FString& IncomingName)
{
	AVehiclePlayerController* VehicleController = Cast<AVehiclePlayerController>(Player);
	AActor* RestartSpot = nullptr;
	if ((VehicleController != nullptr) && (VehicleController->LastTrackPoint != nullptr))
	{
		RestartSpot = VehicleController->LastTrackPoint;
	}
	if (RestartSpot == nullptr)
	{
		RestartSpot = Super::FindPlayerStart(Player, IncomingName);		
	}
	return RestartSpot;
}


