// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGame.h"

AVehicleTrackPoint::AVehicleTrackPoint(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	USceneComponent* SceneComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneComp"));
	RootComponent = SceneComponent;

	UBoxComponent* TriggerComponent = ObjectInitializer.CreateDefaultSubobject<UBoxComponent>(this, TEXT("TriggerComp"));
	TriggerComponent->InitBoxExtent(FVector(50.0f, 1000.0f, 200.0f));
	TriggerComponent->RelativeLocation.Z = 200.0f;
	TriggerComponent->SetCollisionObjectType(COLLISION_PICKUP);
	TriggerComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerComponent->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
	TriggerComponent->AttachParent = RootComponent;

#if WITH_EDITORONLY_DATA
	SpriteComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UBillboardComponent>(this, TEXT("Sprite"));
	ArrowComponent = ObjectInitializer.CreateEditorOnlyDefaultSubobject<UArrowComponent>(this, TEXT("Arrow"));

	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> TrackTextureObject;
			FName ID_Track;
			FText NAME_Track;
			FConstructorStatics()
				: TrackTextureObject(TEXT("/Engine/EditorMaterials/TargetIcon"))
				, ID_Track(TEXT("Track"))
				, NAME_Track(NSLOCTEXT("SpriteCategory", "Track", "Track"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (SpriteComponent)
		{
			SpriteComponent->Sprite = ConstructorStatics.TrackTextureObject.Get();
			SpriteComponent->RelativeScale3D = FVector(3.0f, 3.0f, 3.0f);
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_Track;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Track;
			SpriteComponent->AttachParent = TriggerComponent;
			SpriteComponent->bIsScreenSizeScaled = true;
		}

		if (ArrowComponent)
		{
			ArrowComponent->ArrowColor = FColor(150, 200, 255);
			ArrowComponent->RelativeLocation = FVector(50.0f, 0, 0);
			ArrowComponent->ArrowSize = 5.0f;
			ArrowComponent->bTreatAsASprite = true;
			ArrowComponent->SpriteInfo.Category = ConstructorStatics.ID_Track;
			ArrowComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_Track;
			ArrowComponent->AttachParent = TriggerComponent;
			ArrowComponent->bIsScreenSizeScaled = true;
		}
	}
#endif // WITH_EDITORONLY_DATA
}

void AVehicleTrackPoint::ReceiveActorBeginOverlap(class AActor* Other)
{
	Super::ReceiveActorBeginOverlap(Other);

	ABuggyPawn* OtherVehicle = Cast<ABuggyPawn>(Other);
	if (OtherVehicle)
	{
		OtherVehicle->OnTrackPointReached(this);
	}
}
