// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.


#include "VehicleGame.h"
#include "SoundDefinitions.h"

USoundNodeVehicleEngine::USoundNodeVehicleEngine(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USoundNodeVehicleEngine::ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances)
{
	FSoundParseParameters UpdatedParams = ParseParams;
	StoreCurrentRPM(AudioDevice, ActiveSound, ParseParams, CurrentMaxRPM);

	for (int32 ChildNodeIndex = 0; ChildNodeIndex < ChildNodes.Num(); ChildNodeIndex++)
	{
		if (ChildNodes[ChildNodeIndex])
		{
			const float FadeInRPMMin = EngineSamples[ChildNodeIndex].FadeInRPMStart;
			const float FadeInRPMMax = EngineSamples[ChildNodeIndex].FadeInRPMEnd;
			const float FadeOutRPMMin = EngineSamples[ChildNodeIndex].FadeOutRPMStart;
			const float FadeOutRPMMax = EngineSamples[ChildNodeIndex].FadeOutRPMEnd;

			CurrentMaxRPM = FMath::Max(CurrentMaxRPM, FadeOutRPMMax);

			const float RPMAlpha = (CurrentRPM - FadeInRPMMin) / (FadeOutRPMMax - FadeInRPMMin);

			float PitchToSet = FMath::Lerp(1.0f, EngineSamples[ChildNodeIndex].MaxPitchMultiplier, RPMAlpha);
			float VolumeToSet = 1.0f;

			if (CurrentRPM >= FadeInRPMMin && CurrentRPM <= FadeInRPMMax && FadeInRPMMin != FadeInRPMMax)
			{
				VolumeToSet = (CurrentRPM - FadeInRPMMin) / (FadeInRPMMax - FadeInRPMMin);
			}
			else if (CurrentRPM >= FadeOutRPMMin && CurrentRPM <= FadeOutRPMMax && FadeOutRPMMin != FadeOutRPMMax)
			{
				VolumeToSet = 1.0f - (CurrentRPM - FadeOutRPMMin) / (FadeOutRPMMax - FadeOutRPMMin);
			}
			else if (CurrentRPM < FadeInRPMMax || CurrentRPM > FadeOutRPMMax)
			{
				VolumeToSet = 0.0f;
				PitchToSet = 1.0f;
			}

			UpdatedParams.Volume = ParseParams.Volume * VolumeToSet;
			UpdatedParams.Pitch = ParseParams.Pitch * PitchToSet;

			// "play" the rest of the tree
			ChildNodes[ChildNodeIndex]->ParseNodes(AudioDevice, GetNodeWaveInstanceHash(NodeWaveInstanceHash, ChildNodes[ChildNodeIndex], ChildNodeIndex), ActiveSound, UpdatedParams, WaveInstances);
		}
	}
}

void USoundNodeVehicleEngine::CreateStartingConnectors()
{
	InsertChildNode(ChildNodes.Num());
	InsertChildNode(ChildNodes.Num());
}

void USoundNodeVehicleEngine::InsertChildNode( int32 Index )
{
	Super::InsertChildNode(Index);

	EngineSamples.InsertZeroed(Index);
	EngineSamples[Index].MaxPitchMultiplier = 1.0f;
}


void USoundNodeVehicleEngine::RemoveChildNode( int32 Index )
{
	Super::RemoveChildNode(Index);
	EngineSamples.RemoveAt(Index);
}

#if WITH_EDITOR
void USoundNodeVehicleEngine::SetChildNodes(TArray<USoundNode*>& InChildNodes)
{
	Super::SetChildNodes(InChildNodes);

	if (EngineSamples.Num() < ChildNodes.Num())
	{
		const int32 OldSize = EngineSamples.Num();
		const int32 NumToAdd = ChildNodes.Num() - OldSize;
		EngineSamples.AddZeroed(NumToAdd);
		for (int32 NewIndex = OldSize; NewIndex < EngineSamples.Num(); ++NewIndex)
		{
			EngineSamples[NewIndex].MaxPitchMultiplier = 1.0f;
		}
	}
	else if (EngineSamples.Num() > ChildNodes.Num())
	{
		const int32 NumToRemove = EngineSamples.Num() - ChildNodes.Num();
		EngineSamples.RemoveAt(EngineSamples.Num() - NumToRemove, NumToRemove);
	}
}
#endif //WITH_EDITOR

void USoundNodeVehicleEngine::StoreCurrentRPM(FAudioDevice* AudioDevice, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, float MaxRPM)
{
	AActor* SoundOwner = ActiveSound.AudioComponent.IsValid() ? ActiveSound.AudioComponent->GetOwner() : NULL;
	AVehiclePlayerController* PCOwner = Cast<AVehiclePlayerController>(SoundOwner);
	if (PCOwner)
	{
		SoundOwner = PCOwner->GetPawn();
	}

	UWorld* MyWorld = NULL;
	float DesiredRPM = 0.0f;
	ABuggyPawn* PawnOwner = Cast<ABuggyPawn>(SoundOwner);
	if (PawnOwner)
	{
		DesiredRPM = PawnOwner->GetEngineRotationSpeed();
		MyWorld = PawnOwner->GetWorld();
	}

	const float CurrTime = MyWorld ? MyWorld->GetTimeSeconds() : 0.0f;
	const float DeltaTime = (CurrTime - CurrentRPMStoreTime);

	CurrentRPMStoreTime = CurrTime;
	CurrentRPM = FMath::FInterpTo(CurrentRPM, FMath::Min(DesiredRPM, MaxRPM), DeltaTime, 10.0f);
}