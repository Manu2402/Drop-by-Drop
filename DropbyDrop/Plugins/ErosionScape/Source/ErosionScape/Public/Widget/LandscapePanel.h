#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FLandscapeGenerationSettings;
struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;

class SLandscapePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLandscapePanel) {}
	SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)
	SLATE_ARGUMENT(TSharedPtr<FHeightMapGenerationSettings>, Heightmap)
	SLATE_ARGUMENT(TSharedPtr<FExternalHeightMapSettings>,   External)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FLandscapeGenerationSettings> Landscape;
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;
	TSharedPtr<FExternalHeightMapSettings>   External;

	FReply OnCreateLandscapeClicked();
	FReply OnDestroyLastClicked();
	FReply OnSplitInProxiesClicked();
};
