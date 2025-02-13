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

	static UTexture2D* LoadSavedTexture(const FString& AssetName);
	static TSharedPtr<FSlateBrush> CreateSlateBrushFromTexture(UTexture2D* Texture);
	TSharedPtr<FSlateBrush> StoredBrush;


private:
	
	void RegisterMenus();
	TSharedRef<SWidget> CreateHeightMapColumn();
	TSharedRef<SWidget> CreateLandScapeColumn();
	TSharedRef<SWidget> CreateErosionColumn();
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

	//---------------------------------------------------------------------------------------------

	TArray<TSharedPtr<FString>> ErosionTemplatesOptions;
	TArray<TSharedPtr<FString>> FilteredErosionTemplatesOptions;
	TSharedPtr<FString> CurrentErosionTemplateOption;

	TSharedPtr<SListView<TSharedPtr<FString>>> ListView;

	void SetUpTemplates(const UDataTable* ErosionTemplatesDataTable);
	void AddTemplate(const FString& Param);
	void DeleteTemplate();

	UDataTable* GetErosionTemplates() const;
	void SetErosionTemplates(const TCHAR* DataTablePath);

	//---------------------------------------------------------------------------------------------

	TArray<TSharedPtr<FString>> WindDirections;
	TSharedPtr<FString> CurrentWindDirection;
	UEnum* WindDirectionEnumPtr = nullptr;

	void SetWindDirectionsFromEnum();

	//---------------------------------------------------------------------------------------------
	


	//---------------------------------------------------------------------------------------------

	TSharedPtr<class FUICommandList> PluginCommands;
};
