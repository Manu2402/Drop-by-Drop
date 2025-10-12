// Copyright Epic Games, Inc. All Rights Reserved.

#include "DropByDropCommands.h"

#define LOCTEXT_NAMESPACE "FDropByDropModule"

void FDropByDropCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "DropByDrop", "Bring up DropByDrop window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE