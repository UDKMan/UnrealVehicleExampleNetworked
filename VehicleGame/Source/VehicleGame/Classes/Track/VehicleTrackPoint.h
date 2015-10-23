// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleTrackPoint.generated.h"

UCLASS()
class AVehicleTrackPoint : public AActor
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	UBillboardComponent* SpriteComponent;

	UPROPERTY()
	UArrowComponent* ArrowComponent;
public:
#endif

	/** set checkpoint for touching vehicle */
	virtual void ReceiveActorBeginOverlap(class AActor* Other) override;

#if WITH_EDITORONLY_DATA
	/** Returns SpriteComponent subobject **/
	FORCEINLINE UBillboardComponent* GetSpriteComponent() const { return SpriteComponent; }
	/** Returns ArrowComponent subobject **/
	FORCEINLINE UArrowComponent* GetArrowComponent() const { return ArrowComponent; }
#endif
};
