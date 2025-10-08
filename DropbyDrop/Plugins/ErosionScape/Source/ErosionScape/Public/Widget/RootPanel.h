#pragma once
#include "CoreMinimal.h"
#include "ErosionTemplateManager.h"
#include "Widgets/SCompoundWidget.h"

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;
struct FErosionSettings;

class SWidgetSwitcher;
class SImage;

class SHeightMapPanel;
class SLandscapePanel;
class SErosionPanel;

class SRootPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRootPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	FORCEINLINE TSharedPtr<FLandscapeGenerationSettings> GetLandscapeSettings() const { return Landscape; }

private:
	// Settings 
	TSharedPtr<FHeightMapGenerationSettings>  Heightmap;
	TSharedPtr<FExternalHeightMapSettings>    External;
	TSharedPtr<FLandscapeGenerationSettings>  Landscape;
	TSharedPtr<FErosionSettings>              Erosion;

	//Template
	TObjectPtr<UErosionTemplateManager> ErosionTemplateManager;

	// Switcher 
	TSharedPtr<SWidgetSwitcher> Switcher;

	// Preview heightmap
	TSharedPtr<FSlateBrush>               RightPreviewBrush;
	TSharedPtr<SImage>                    RightPreviewImage;
	TStrongObjectPtr<class UTexture2D>    RightPreviewTexture;
	
	// NavButtonTexts
	TArray<TSharedPtr<STextBlock>> NavButtonTexts;
	int32 ActiveIndex = 0; // 0 = HeightMap, 1 = Landscape, 2 = Erosion

private:
	// UI
	TSharedRef<SWidget> BuildSidebar();
	TSharedRef<SWidget> BuildCenter();
	TSharedRef<SWidget> BuildRightPanel();

	// Actions
	FReply OnNavClicked(const int32 Index);
	FReply OnActionCreateHeightMap();
	FReply OnActionCreateLandscapeInternal();
	FReply OnActionImportPNG();

	// Preview
	void RefreshRightPreview();
};
