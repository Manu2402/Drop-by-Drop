// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FErosionScapeModule : public IModuleInterface, public TSharedFromThis<FErosionScapeModule>
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();

private:
	
	void RegisterMenus();
	TSharedRef<SWidget> CreateHeightMapColumn();
	TSharedRef<SWidget> CreateLandScapeColumn();
	TSharedRef<SWidget> CreateErosionColumn();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	//---------------------------------------------------------------------------------------------

	TArray<TSharedPtr<FString>> ErosionTemplatesOptions;
	TSharedPtr<FString> CurrentErosionTemplateOptions;

	void SetUpTemplates(const UDataTable* ErosionTemplatesDataTable);
	void AddTemplate(const FString& Param);
	void RemoveTemplate();

	UDataTable* GetErosionTemplates() const;
	void SetErosionTemplates(const TCHAR* DataTablePath);

	//---------------------------------------------------------------------------------------------

	TSharedPtr<class FUICommandList> PluginCommands;
};
