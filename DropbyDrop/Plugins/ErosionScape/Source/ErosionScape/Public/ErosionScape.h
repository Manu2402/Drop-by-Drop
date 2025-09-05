#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FErosionScapeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Open the tab */
	void PluginButtonClicked();

private:
	/** Register menus/buttons */
	void RegisterMenus();

	/** Spawn the main tab with our three panels */
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	/** Small helper to show notifications */
	void ShowEditorNotification(const FString& Message, bool bSuccess) const;

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
