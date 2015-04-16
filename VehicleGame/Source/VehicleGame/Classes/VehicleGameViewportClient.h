// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleGameViewportClient.generated.h"

UCLASS(Within=Engine, transient, config=Engine)
class UVehicleGameViewportClient : public UGameViewportClient
{
	GENERATED_UCLASS_BODY()

public:


#if WITH_EDITOR
	virtual void DrawTransition(class UCanvas* Canvas) override;
#endif //WITH_EDITOR	
};