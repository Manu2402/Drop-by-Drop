#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;

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

private:
	// Settings 
	TSharedPtr<FHeightMapGenerationSettings>  Heightmap;
	TSharedPtr<FExternalHeightMapSettings>    External;
	TSharedPtr<FLandscapeGenerationSettings>  Landscape;

	// Switcher 
	TSharedPtr<SWidgetSwitcher> Switcher;

	// Preview heightmap
	TSharedPtr<FSlateBrush>               RightPreviewBrush;
	TSharedPtr<SImage>                    RightPreviewImage;
	TStrongObjectPtr<class UTexture2D>    RightPreviewTexture;

	// 
	int32 ActiveIndex = 0; // 0 = HeightMap, 1 = Landscape, 2 = Erosion

private:
	// UI
	TSharedRef<SWidget> BuildSidebar();
	TSharedRef<SWidget> BuildCenter();
	TSharedRef<SWidget> BuildRightPane();

	// Actions
	FReply OnNavClicked(const int32 Index);
	FReply OnActionCreateHeightMap();
	FReply OnActionCreateLandscapeInternal();
	FReply OnActionImportPNG();

	// Preview
	void RefreshRightPreview();
};
