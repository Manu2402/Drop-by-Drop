// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "DropByDropStyle.h"

class FDropByDropCommands : public TCommands<FDropByDropCommands>
{
public: // Methods.
	FDropByDropCommands() : TCommands<FDropByDropCommands>(TEXT("DropByDrop"), NSLOCTEXT("Contexts", "DropByDrop", "DropByDrop Plugin"), NAME_None, FDropByDropStyle::GetStyleSetName()) { }

	// TCommands<> interface
	virtual void RegisterCommands() override;

public: // Members.
	TSharedPtr<FUICommandInfo> OpenPluginWindow;

};