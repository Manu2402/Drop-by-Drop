// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ErosionScapeStyle.h"

class FErosionScapeCommands : public TCommands<FErosionScapeCommands>
{
public:
	FErosionScapeCommands() : TCommands<FErosionScapeCommands>(TEXT("ErosionScape"), NSLOCTEXT("Contexts", "ErosionScape", "ErosionScape Plugin"), NAME_None, FErosionScapeStyle::GetStyleSetName()) { }

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};