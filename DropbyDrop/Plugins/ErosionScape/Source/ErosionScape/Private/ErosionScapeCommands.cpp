// Copyright Epic Games, Inc. All Rights Reserved.

#include "ErosionScapeCommands.h"

#define LOCTEXT_NAMESPACE "FErosionScapeModule"

void FErosionScapeCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ErosionScape", "Bring up ErosionScape window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
