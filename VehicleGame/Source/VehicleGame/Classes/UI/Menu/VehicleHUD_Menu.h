// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "VehicleTypes.h"
#include "VehicleHUD_Menu.generated.h"

namespace EVehicleMenu
{
	enum Type
	{
		PlaySolo,
		HostLocal,
		JoinLocal,
		Map1,
		Map2,
		RemapControls,
		RemapController,
		ChangeQuality,	
		Quit,
	};
};

UCLASS()
class AVehicleHUD_Menu : public AHUD
{
	GENERATED_UCLASS_BODY()

	/** in game menu pointer */
	MenuPtr MainMenu;

	/** controls settings menu */
	MenuPtr ControlsMenu;

	/** called just after game begins */
	virtual void PostInitializeComponents() override;

	/** Clears out the old widgets, rebuilds them */
	void RebuildWidgets(bool bHotReload = false);

private:

	/** main menu widget pointer */
	TSharedPtr<class SVehicleMenuWidget> MenuWidget;

	/** controls setup widget pointer */
	TSharedPtr<class SVehicleControlsSetup> ControlsSetupWidget;

	/** graphics quality menu item */
	FVehicleMenuItem* QualityMenuItem;

	/** quality descriptions list */
	TArray<FText> LowHighList;

	/** menu callback */
	void ExecuteMenuAction(EVehicleMenu::Type Action);

	/** starts the single player game */
	void PlaySolo();

	/** hosts local multiplayer game */
	void HostLocal();

	/** quits the game */
	void Quit();

	/** Display the a loading screen for the given action if it is required. */
	void ShowLoadingScreen();		
};
