// © Manuel Solano
// © Roberto Capparelli

#pragma once

#include "CoreMinimal.h"

#pragma region ForwardDeclarations

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;
struct FErosionSettings;

class UErosionTemplateManager;

class SHeightMapPanel;
class SLandscapePanel;
class SErosionPanel;

class SWidgetSwitcher;
class UTexture2D;
class SImage;

class ALandscape;

#pragma endregion

class SRootPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRootPanel) {}
		SLATE_ARGUMENT(TObjectPtr<ALandscape>*, ActiveLandscape)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// Settings.
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;
	TSharedPtr<FExternalHeightMapSettings> External;
	TSharedPtr<FLandscapeGenerationSettings> Landscape;
	TSharedPtr<FErosionSettings> Erosion;

	// Active Landscape.
	TObjectPtr<ALandscape>* ActiveLandscape;

	// Template.
	TObjectPtr<UErosionTemplateManager> ErosionTemplateManager;

	// Switcher.
	TSharedPtr<SWidgetSwitcher> Switcher;

	// Preview Heightmap.
	TSharedPtr<FSlateBrush> RightPreviewBrush;
	TSharedPtr<SImage> RightPreviewImage;
	TStrongObjectPtr<UTexture2D> RightPreviewTexture;

	// NavButtonTexts.
	TArray<TSharedPtr<STextBlock>> NavButtonTexts;
	int32 ActiveIndex = 0; // 0 = HeightMap, 1 = Landscape, 2 = Erosion.

private:
	// UI.
	TSharedRef<SWidget> BuildSidebar();
	TSharedRef<SWidget> BuildCenter();
	TSharedRef<SWidget> BuildRightPanel();

	// Actions.
	FReply OnNavClicked(const int32 Index);
	FReply OnActionCreateHeightMap();
	FReply OnActionCreateLandscapeInternal();
	FReply OnActionImportPNG();

	// Preview.
	void RefreshRightPreview();
};
