// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class VehicleGame : ModuleRules
{
	public VehicleGame(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"Landscape",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"Slate",
				"SlateCore",
				"VehicleGameLoadingScreen",
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
				"VehicleGame/Private/UI/Widgets",
				"VehicleGame/Private/UI/Style",
			}
		);
	}
}
