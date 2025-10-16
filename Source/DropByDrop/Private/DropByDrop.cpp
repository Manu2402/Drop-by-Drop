// Copyright Epic Games, Inc. All Rights Reserved.
// © Manuel Solano
// © Roberto Capparelli

#include "DropByDrop.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Components/LandscapeInfoComponent.h"
#include "Editor/UnrealEd/Public/Selection.h"
#include "DropByDropCommands.h"
#include "DropByDropLogger.h"
#include "Widget/RootPanel.h"
#include "LevelEditor.h"
#include "Landscape.h"

#define LOCTEXT_NAMESPACE "FDropByDropModule"
#define EROSION_TEMPLATES_DT_PATH "/DropByDrop/DT_ErosionTemplates.DT_ErosionTemplates"

#pragma region Module

/**
 * Called when the module is loaded during engine startup.
 * Initializes plugin style, commands, UI elements, and hooks into editor events.
 */
void FDropByDropModule::StartupModule()
{
	// Initialize and load plugin visual style and textures.
	FDropByDropStyle::Initialize();
	FDropByDropStyle::ReloadTextures();

	// Register plugin commands for UI actions.
	FDropByDropCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);

	// Map the "Open Plugin Window" command to the button click handler.
	PluginCommands->MapAction(
		FDropByDropCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FDropByDropModule::PluginButtonClicked),
		FCanExecuteAction()
	);

	// Register the menu initialization callback
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FDropByDropModule::RegisterMenus));

	// Register the nomad tab spawner for the plugin's main window.
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(DropByDropTabName, FOnSpawnTab::CreateRaw(this, &FDropByDropModule::OnSpawnPluginTab) )
		.SetDisplayName(LOCTEXT("FDropByDropTabTitle", "DropByDrop"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	// Load the erosion templates data table and store it in plugin settings.
	FDropByDropSettings::Get().SetErosionTemplatesDT(LoadObject<UDataTable>(nullptr, TEXT(EROSION_TEMPLATES_DT_PATH)));

#if WITH_EDITOR
	// Verify that the LevelEditor module is loaded.
	if (!FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		UE_LOG(LogDropByDrop, Error, TEXT("\"LevelEditor\" module isn't loaded!"));
		return;
	}

	// Hook into the Level Editor's actor selection changed event.
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnActorSelectionChanged().AddRaw(this, &FDropByDropModule::OnActorSelectionChangedInvoke);
#endif
}

/**
 * Called when the module is unloaded during engine shutdown.
 * Cleans up registered callbacks, commands, and UI elements.
 */
void FDropByDropModule::ShutdownModule()
{
	// Unregister tool menu callbacks.
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	// Shutdown style and unregister commands.
	FDropByDropStyle::Shutdown();
	FDropByDropCommands::Unregister();

	// Unregister the plugin's tab spawner.
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DropByDropTabName);

#if WITH_EDITOR
	// Verify that the LevelEditor module is still loaded.
	if (!FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		UE_LOG(LogDropByDrop, Error, TEXT("\"LevelEditor\" module isn't loaded!"));
		return;
	}

	// Remove the actor selection changed callback.
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnActorSelectionChanged().RemoveAll(this);
#endif
}

#pragma endregion

/**
 * Callback invoked when actor selection changes in the editor.
 * Updates the active landscape reference if a landscape actor is selected.
 */
void FDropByDropModule::OnActorSelectionChangedInvoke(const TArray<UObject*>& SelectedActors, bool bForce)
{
	// Exit early if the root panel hasn't been created yet.
	if (!RootPanel.IsValid())
	{
		return;
	}

	// Exit early if no actors are selected.
	if (SelectedActors.Num() <= 0)
	{
		return;
	}

	// Get the last selected actor (most recent selection).
	const int32 LastSelectedActorIndex = SelectedActors.Num() - 1;
	UObject* Object = SelectedActors[LastSelectedActorIndex];

	// Try to cast the selected object to a "Landscape" actor.
	TObjectPtr<ALandscape> Landscape = Cast<ALandscape>(Object);
	if (!IsValid(Landscape))
	{
		// Clear the active landscape if selection is not a landscape.
		ActiveLandscape = nullptr;
		return;
	}

	// Store the selected landscape as the active landscape.
	ActiveLandscape = Landscape;
}

#pragma region NomadTab + Slate

/**
 * Spawns the main plugin tab containing the root panel.
 * Creates the Slate UI hierarchy for the plugin interface.
 */
TSharedRef<SDockTab> FDropByDropModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Create and assign the root panel widget.
			SAssignNew(RootPanel, SRootPanel)
				.ActiveLandscape(&ActiveLandscape)
		];
}

/**
 * Handles the button click event to open the plugin's main tab.
 * Invoked when user clicks the toolbar button or menu item.
 */
void FDropByDropModule::PluginButtonClicked()
{
	// Attempt to invoke (show/focus) the plugin's tab.
	FGlobalTabmanager::Get()->TryInvokeTab(DropByDropTabName);
}

/**
 * Registers the plugin's menu entries and toolbar buttons.
 * Adds entries to the Level Editor's main menu and toolbar.
 */
void FDropByDropModule::RegisterMenus()
{
	// Ensure proper cleanup by scoping ownership.
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add entry to the Window menu in the main menu bar.
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntryWithCommandList(FDropByDropCommands::Get().OpenPluginWindow, PluginCommands);
	}

	// Add button to the Level Editor toolbar.
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
		FToolMenuEntry& Entry = Section.AddEntry(
			FToolMenuEntry::InitToolBarButton(FDropByDropCommands::Get().OpenPluginWindow)
		);

		// Associate the command list with the toolbar entry.
		Entry.SetCommandList(PluginCommands);
	}
}

#pragma endregion

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FDropByDropModule, DropByDrop)