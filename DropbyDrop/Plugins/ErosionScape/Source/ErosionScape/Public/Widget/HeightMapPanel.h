#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;

class SHeightMapPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHeightMapPanel) {}
	SLATE_ARGUMENT(TSharedPtr<FHeightMapGenerationSettings>, Heightmap)
	SLATE_ARGUMENT(TSharedPtr<FExternalHeightMapSettings>,   External)
	SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;
	TSharedPtr<FExternalHeightMapSettings>   External;
	TSharedPtr<FLandscapeGenerationSettings> Landscape;

	// Preview
	TSharedPtr<FSlateBrush> HeightPreviewBrush;
	void RefreshPreview();

	// Callbacks
	FReply OnCreateHeightmapClicked();
	FReply OnCreateLandscapeFromInternalClicked();
	FReply OnCreateLandscapeFromPNGClicked();
};
