#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;
class ALandscape;
struct FErosionSettings;

class UErosionTemplateManager;
class STemplateBrowser;

class SErosionPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SErosionPanel)
		{
		}

		SLATE_ARGUMENT(TSharedPtr<FHeightMapGenerationSettings>, Heightmap)
		SLATE_ARGUMENT(TSharedPtr<FExternalHeightMapSettings>, External)
		SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)
		SLATE_ARGUMENT(TObjectPtr<ALandscape>*, SelectedLandscape)
		SLATE_ARGUMENT(TSharedPtr<FErosionSettings>, Erosion)
		SLATE_ARGUMENT(TObjectPtr<UErosionTemplateManager>, TemplateManager)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

private:
	// Shared settings (owned by the outer widget/panel)
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;
	TSharedPtr<FExternalHeightMapSettings> External;
	TSharedPtr<FLandscapeGenerationSettings> Landscape;
	TObjectPtr<ALandscape>* SelectedLandscape;
	TSharedPtr<FErosionSettings> Erosion;
	TObjectPtr<UErosionTemplateManager> TemplateManager = nullptr;

	// Wind UI data
	TArray<TSharedPtr<FString>> WindDirections;
	TSharedPtr<FString> CurrentWindDirection;
	UEnum* WindDirectionEnumPtr = nullptr;

	TSharedPtr<SComboBox<TSharedPtr<FString>>> WindCombo;

private:
	// Helpers
	void BuildWindDirections();

	// Actions
	FReply OnErodeClicked();
};
