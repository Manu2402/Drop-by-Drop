#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;
class UErosionTemplateManager;

class SErosionPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SErosionPanel) {}
	SLATE_ARGUMENT(TSharedPtr<FHeightMapGenerationSettings>, Heightmap)
	SLATE_ARGUMENT(TSharedPtr<FExternalHeightMapSettings>,   External)
	SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)
	SLATE_ARGUMENT(TObjectPtr<UErosionTemplateManager>,      TemplateManager)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;
	TSharedPtr<FExternalHeightMapSettings>   External;
	TSharedPtr<FLandscapeGenerationSettings> Landscape;
	TObjectPtr<UErosionTemplateManager>      TemplateManager;

	// Wind direction combo state
	TSharedPtr<IConsoleVariable> PlaceHolder;
	TArray<TSharedPtr<FString>> WindDirections;
	TSharedPtr<FString> CurrentWindDirection;
	UEnum* WindDirectionEnumPtr = nullptr;

	FReply OnErodeClicked();
	void BuildWindDirections();
};
