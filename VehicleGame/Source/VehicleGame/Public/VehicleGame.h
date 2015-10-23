// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#ifndef __VEHiCLEGAME_H__
#define __VEHiCLEGAME_H__

#if WITH_EDITOR
#include "UnrealEd.h"
#endif

#include "Engine.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "SoundDefinitions.h"
#include "ParticleDefinitions.h"
#include "BlueprintUtilities.h"
#include "Net/UnrealNetwork.h"

#include "VehicleGameClasses.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVehicle, Log, All);

/** when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_PICKUP		ECC_GameTraceChannel1

#endif
