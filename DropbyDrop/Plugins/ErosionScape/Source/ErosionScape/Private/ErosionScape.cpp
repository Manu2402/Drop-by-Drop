// Copyright Epic Games, Inc. All Rights Reserved.
// © Manuel Solano
// © Roberto Capparelli

#include "ErosionScape.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Editor/UnrealEd/Public/Selection.h"
#include "ErosionScapeCommands.h"
#include "DropByDropLandscape.h"
#include "Widget/RootPanel.h"
#include "Landscape.h"

#define LOCTEXT_NAMESPACE "FErosionScapeModule"

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

	FDropByDropSettings::Get().SetErosionTemplatesDT(LoadObject<UDataTable>(nullptr, ErosionTemplatesDTPath));

#if WITH_EDITOR
	GEditor->GetSelectedActors()->SelectObjectEvent.AddRaw(this, &FErosionScapeModule::OnActorSelected);
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
	GEditor->GetSelectedActors()->SelectObjectEvent.RemoveAll(this);
#endif
}

#pragma endregion

void FErosionScapeModule::OnActorSelected(UObject* Object)
{
	if (!RootPanel.IsValid())
	{
		return;
	}

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

void FErosionScapeModule::ShowEditorNotification(const FString& Message, const bool bSuccess) const
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.bFireAndForget = true;
	Info.bUseLargeFont = true;

	if (bSuccess)
	{
		Info.ExpireDuration = 2.0f;
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.Success"));
	}
	else
	{
		Info.ExpireDuration = 5.0f;
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.Fail"));
		Info.bUseThrobber = false;
		Info.FadeOutDuration = 1.5f;
	}

	FSlateNotificationManager::Get().AddNotification(Info);
}

#undef LOCTEXT_NAMESPACE
IMPLEMENT_MODULE(FErosionScapeModule, ErosionScape)
