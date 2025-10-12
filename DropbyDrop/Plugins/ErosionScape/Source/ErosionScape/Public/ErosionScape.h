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

class FErosionScapeModule : public IModuleInterface
{
public: // Methods.
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Open the tab */
	void PluginButtonClicked();

private: // Methods.
	/** Register menus/buttons */
	void RegisterMenus();

	/** Spawn the main tab with our three panels */
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

	void OnActorSelected(UObject* SelectedActor);

private: // Members.
	TSharedPtr<FUICommandList> PluginCommands;

	TSharedPtr<SRootPanel> RootPanel;
	TObjectPtr<ALandscape> ActiveLandscape;

	const TCHAR* ErosionTemplatesDTPath = TEXT("/ErosionScape/DT_ErosionTemplates.DT_ErosionTemplates");
	const FName DropByDropTabName = FName("DropByDrop");

};
