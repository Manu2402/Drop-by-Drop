// Copyright Epic Games, Inc. All Rights Reserved.
// © Manuel Solano
// © Roberto Capparelli

#pragma once
#include "CoreMinimal.h"

#pragma region ForwardDeclarations

class SDockTab;
class FSpawnTabArgs;
class FUICommandList;
class FMenuBuilder;
class SRootPanel;

class ALandscape;

#pragma endregion

/**
 * Main module class for the "DropByDrop" plugin.
 * Handles plugin initialization, UI integration with Unreal Editor,
 * and landscape selection tracking.
 */
class FDropByDropModule : public IModuleInterface
{
public: // Methods.
	/**
	 * IModuleInterface implementation.
	 * Called when the module is loaded during engine startup.
	 */
	virtual void StartupModule() override;

	/**
	 * IModuleInterface implementation.
	 * Called when the module is unloaded during engine shutdown.
	 */
	virtual void ShutdownModule() override;

	/**
	 * Handles the button click event to open the plugin's main tab.
	 * Invoked when user clicks the toolbar button or menu item.
	 */
	void PluginButtonClicked();

private: // Methods.
	/**
	 * Registers the plugin's menu entries and toolbar buttons.
	 * Adds entries to the Level Editor's main menu and toolbar.
	 */
	void RegisterMenus();

	/**
	 * Spawns the main plugin tab containing the root panel.
	 * @param SpawnTabArgs - Arguments passed when spawning the tab.
	 * @return A shared reference to the newly created dock tab.
	 */
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

	/**
	 * Callback invoked when actor selection changes in the editor.
	 * Updates the active landscape reference if a landscape is selected.
	 * @param NewSelection - Array of newly selected objects.
	 * @param bForce - Whether the selection change was forced.
	 */
	void OnActorSelectionChangedInvoke(const TArray<UObject*>& NewSelection, bool bForce);

private: // Members.
	/** Command list for handling plugin UI actions. */
	TSharedPtr<FUICommandList> PluginCommands;

	/** Reference to the main UI panel of the plugin. */
	TSharedPtr<SRootPanel> RootPanel;

	/** Currently active landscape being edited. */
	TObjectPtr<ALandscape> ActiveLandscape;

	/** Unique name identifier for the plugin's tab. */
	const FName DropByDropTabName = FName("DropByDrop");

};
