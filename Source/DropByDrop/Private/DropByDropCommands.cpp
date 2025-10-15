// Copyright Epic Games, Inc. All Rights Reserved.

#include "DropByDropCommands.h"

#define LOCTEXT_NAMESPACE "FDropByDropModule"

/**
 * RegisterCommands
 *
 * Registers all UI commands for the DropByDrop plugin.
 * This method is called during plugin initialization to set up
 * the command bindings that will be available in the editor.
 */
void FDropByDropCommands::RegisterCommands()
{
	/**
	 * UI_COMMAND macro registers a new UI command with the following parameters:
	 *
	 * @param OpenPluginWindow - The member variable to store the command info.
	 * @param "DropByDrop" - The friendly name displayed in the UI.
	 * @param "Bring up DropByDrop window" - The tooltip/description text.
	 * @param EUserInterfaceActionType::Button - The type of UI action (button press).
	 * @param FInputChord() - The default keyboard shortcut (empty = no default shortcut).
	 */
	UI_COMMAND(OpenPluginWindow, "DropByDrop", "Bring up DropByDrop window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE