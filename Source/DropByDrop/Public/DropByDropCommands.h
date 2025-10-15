// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "DropByDropStyle.h"

/**
 * FDropByDropCommands
 *
 * Defines the UI commands for the "DropByDrop" plugin.
 * This class inherits from TCommands and is responsible for registering
 * and managing UI command bindings that can be triggered from menus,
 * toolbars, or keyboard shortcuts within the Unreal Engine editor.
 */
class FDropByDropCommands : public TCommands<FDropByDropCommands>
{
public: // Methods.

	/**
	 * Constructor.
	 * Initializes the command context with:
	 * - Context name: "DropByDrop".
	 * - Localized context description.
	 * - Parent context: NAME_None (no parent).
	 * - Style set name from "FDropByDropStyle".
	 */
	FDropByDropCommands() : TCommands<FDropByDropCommands>(TEXT("DropByDrop"), NSLOCTEXT("Contexts", "DropByDrop", "DropByDrop Plugin"), NAME_None, FDropByDropStyle::GetStyleSetName()) { }

	/**
	 * RegisterCommands
	 *
	 * Override from TCommands<> interface.
	 * This method is called to register all UI commands associated with this plugin.
	 * Commands registered here become available for use in the editor UI.
	 */
	virtual void RegisterCommands() override;

public: // Members.

	/**
	 * OpenPluginWindow
	 *
	 * Shared pointer to the UI command info for opening the "DropByDrop" plugin window.
	 * This command can be bound to menu items, toolbar buttons, or keyboard shortcuts.
	 */
	TSharedPtr<FUICommandInfo> OpenPluginWindow;

};