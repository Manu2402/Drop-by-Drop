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
		TEXT("/Game/Custom/ErosionTemplates/DT_ErosionTemplate.DT_ErosionTemplate"));
}

void FErosionScapeModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FErosionScapeStyle::Shutdown();
	FErosionScapeCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ErosionScapeTabName);
}

TSharedRef<SDockTab> FErosionScapeModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	auto  HeightmapSettings = MakeShared<FHeightMapGenerationSettings>();
	auto  ExternalSettings = MakeShared<FExternalHeightMapSettings>();
	auto  LandscapeSettings = MakeShared<FLandscapeGenerationSettings>();

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(4)
			[
				SNew(SHeightMapPanel)
				.Heightmap(HeightmapSettings)
				.External(ExternalSettings)
				.Landscape(LandscapeSettings)
			]
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(4)
			[
				SNew(SLandscapePanel)
				.Landscape(LandscapeSettings)
				.Heightmap(HeightmapSettings)
				.External(ExternalSettings)
			]
			+ SHorizontalBox::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill).Padding(4)
			[
				SNew(SErosionPanel)
				.Heightmap(HeightmapSettings)
				.External(ExternalSettings)
				.Landscape(LandscapeSettings)
			]
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
