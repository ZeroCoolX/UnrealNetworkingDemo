// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "UnrealNetworkingDemoGameMode.h"
#include "UnrealNetworkingDemoPawn.h"
#include "UnrealNetworkingDemoHud.h"

AUnrealNetworkingDemoGameMode::AUnrealNetworkingDemoGameMode()
{
	DefaultPawnClass = AUnrealNetworkingDemoPawn::StaticClass();
	HUDClass = AUnrealNetworkingDemoHud::StaticClass();
}
