#include "ErosionScape.h"
#include "ErosionScapeCommands.h"
#include "ErosionScapeStyle.h"
#include "ErosionScapeSettings.h"
#include "GeneratorHeightMapLibrary.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widget/HeightMapPanel.h"
#include "Widget/LandscapePanel.h"
#include "Widget/ErosionPanel.h"
#include "Widget/RootPanel.h"
#include "Editor.h"
#include "Editor/UnrealEd/Public/Selection.h"
#include "Landscape.h"
#include "DropByDropLogger.h"

static const FName ErosionScapeTabName("ErosionScape");
#define LOCTEXT_NAMESPACE "FErosionScapeModule"

void FErosionScapeModule::StartupModule()
{
	FErosionScapeStyle::Initialize();
	FErosionScapeStyle::ReloadTextures();

	FErosionScapeCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FErosionScapeCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FErosionScapeModule::PluginButtonClicked),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FErosionScapeModule::RegisterMenus)
	);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		                        ErosionScapeTabName,
		                        FOnSpawnTab::CreateRaw(this, &FErosionScapeModule::OnSpawnPluginTab)
	                        )
	                        .SetDisplayName(LOCTEXT("FErosionScapeTabTitle", "ErosionScape"))
	                        .SetMenuType(ETabSpawnerMenuType::Hidden);

	UGeneratorHeightMapLibrary::SetErosionTemplates(
		TEXT("/ErosionScape/DT_ErosionTemplates.DT_ErosionTemplates"));

	//Commands
	IConsoleManager::Get().RegisterConsoleCommand(
		TEXT("WindPreview.Scale"),
		TEXT("Set global scale for wind direction preview arrows"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() == 0)
			{
				UE_LOG(LogDropByDrop, Warning, TEXT("Usage: WindPreview.Scale <float>"));
				return;
			}

			const float NewScale = FCString::Atof(*Args[0]);
			UGeneratorHeightMapLibrary::SetWindPreviewScale(NewScale);
			UE_LOG(LogDropByDrop, Log, TEXT("WindPreview scale set to %f"), NewScale);
		}),
		ECVF_Default
	);

#if WITH_EDITOR
	OnActorSelectedHandle = GEditor->GetSelectedActors()->SelectObjectEvent.AddRaw(this, &FErosionScapeModule::OnActorSelected);
#endif

}

void FErosionScapeModule::OnActorSelected(UObject* Object)
{
	if (!Landscape.IsValid())
	{
		Landscape = RootPanel->GetLandscapeSettings();
	}

#if WITH_EDITOR
	if (ALandscape* L = Cast<ALandscape>(Object))
	{
		Landscape->TargetLandscape = L;
	}
	else
	{
		Landscape->TargetLandscape = nullptr;
	}
#endif
}

void FErosionScapeModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FErosionScapeStyle::Shutdown();
	FErosionScapeCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ErosionScapeTabName);

	IConsoleManager::Get().UnregisterConsoleObject(TEXT("WindPreview.Scale"), false);


}

TSharedRef<SDockTab> FErosionScapeModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
	.TabRole(ETabRole::NomadTab)
	[
		SAssignNew(RootPanel, SRootPanel)
	];
}

void FErosionScapeModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ErosionScapeTabName);
}

void FErosionScapeModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Window
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntryWithCommandList(FErosionScapeCommands::Get().OpenPluginWindow, PluginCommands);
	}

	// Toolbar
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
		FToolMenuEntry& Entry = Section.AddEntry(
			FToolMenuEntry::InitToolBarButton(FErosionScapeCommands::Get().OpenPluginWindow)
		);
		Entry.SetCommandList(PluginCommands);
	}
}

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
