// Copyright Epic Games, Inc. All Rights Reserved.
// © Manuel Solano
// © Roberto Capparelli

#include "ErosionScape.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Editor/UnrealEd/Public/Selection.h"
#include "ErosionScapeCommands.h"
#include "LandscapeInfoComponent.h"
#include "DropByDropLogger.h"
#include "Widget/RootPanel.h"
#include "LevelEditor.h"
#include "Landscape.h"

#define LOCTEXT_NAMESPACE "FErosionScapeModule"
#define EROSION_TEMPLATES_DT_PATH "/ErosionScape/DT_ErosionTemplates.DT_ErosionTemplates"

#pragma region Module

void FErosionScapeModule::StartupModule()
{
	FErosionScapeStyle::Initialize();
	FErosionScapeStyle::ReloadTextures();

	FErosionScapeCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(FErosionScapeCommands::Get().OpenPluginWindow, FExecuteAction::CreateRaw(this, &FErosionScapeModule::PluginButtonClicked), FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FErosionScapeModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(DropByDropTabName, FOnSpawnTab::CreateRaw(this, &FErosionScapeModule::OnSpawnPluginTab)).SetDisplayName(LOCTEXT("FDropByDropTabTitle", "DropByDrop")).SetMenuType(ETabSpawnerMenuType::Hidden);

	FDropByDropSettings::Get().SetErosionTemplatesDT(LoadObject<UDataTable>(nullptr, TEXT(EROSION_TEMPLATES_DT_PATH)));

#if WITH_EDITOR
	if (!FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		UE_LOG(LogDropByDrop, Error, TEXT("\"LevelEditor\" module isn't loaded!"));
		return;
	}

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnActorSelectionChanged().AddRaw(this, &FErosionScapeModule::OnActorSelectionChangedInvoke);
#endif
}

void FErosionScapeModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FErosionScapeStyle::Shutdown();
	FErosionScapeCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DropByDropTabName);

#if WITH_EDITOR
	if (!FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		UE_LOG(LogDropByDrop, Error, TEXT("\"LevelEditor\" module isn't loaded!"));
		return;
	}

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.OnActorSelectionChanged().RemoveAll(this);
#endif
}

#pragma endregion

void FErosionScapeModule::OnActorSelectionChangedInvoke(const TArray<UObject*>& SelectedActors, bool bForce)
{
	if (!RootPanel.IsValid())
	{
		return;
	}

	if (SelectedActors.Num() <= 0)
	{
		return;
	}

	const int32 LastSelectedActorIndex = SelectedActors.Num() - 1;
	UObject* Object = SelectedActors[LastSelectedActorIndex];

	TObjectPtr<ALandscape> Landscape = Cast<ALandscape>(Object);
	if (!IsValid(Landscape))
	{
		ActiveLandscape = nullptr;
		return;
	}

	ActiveLandscape = Landscape;
}

#pragma region NomadTab + Slate

TSharedRef<SDockTab> FErosionScapeModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SAssignNew(RootPanel, SRootPanel)
				.ActiveLandscape(&ActiveLandscape)
		];
}

void FErosionScapeModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(DropByDropTabName);
}

void FErosionScapeModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Window.
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntryWithCommandList(FErosionScapeCommands::Get().OpenPluginWindow, PluginCommands);
	}

	// Toolbar.
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FErosionScapeCommands::Get().OpenPluginWindow));
		
		Entry.SetCommandList(PluginCommands);
	}
}

#pragma endregion

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FErosionScapeModule, ErosionScape)
