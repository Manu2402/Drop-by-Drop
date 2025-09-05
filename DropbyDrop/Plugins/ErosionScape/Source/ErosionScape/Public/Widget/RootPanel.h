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
	// Settings condivisi tra i 3 pannelli
	TSharedPtr<FHeightMapGenerationSettings>  Heightmap;
	TSharedPtr<FExternalHeightMapSettings>    External;
	TSharedPtr<FLandscapeGenerationSettings>  Landscape;

	// Switcher centrale
	TSharedPtr<SWidgetSwitcher> Switcher;

	// Anteprima texture sulla colonna di destra (sempre visibile)
	TSharedPtr<FSlateBrush>               RightPreviewBrush;
	TSharedPtr<SImage>                    RightPreviewImage;
	TStrongObjectPtr<class UTexture2D>    RightPreviewTexture;

	// Stato corrente
	int32 ActiveIndex = 0; // 0 = HeightMap, 1 = Landscape, 2 = Erosion

private:
	// UI
	TSharedRef<SWidget> BuildSidebar();
	TSharedRef<SWidget> BuildCenter();
	TSharedRef<SWidget> BuildRightPane();

	// Azioni
	FReply OnNavClicked(const int32 Index);
	FReply OnActionCreateHeightMap();
	FReply OnActionCreateLandscapeInternal();
	FReply OnActionImportPNG();

	// Preview
	void RefreshRightPreview();
};
