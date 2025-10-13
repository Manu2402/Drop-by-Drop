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

class ALandscape;

#pragma endregion 

#define WIND_DIRECTIONS 9

class SErosionPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SErosionPanel) {}
		SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)
		SLATE_ARGUMENT(TSharedPtr<FErosionSettings>, Erosion)
		SLATE_ARGUMENT(TObjectPtr<ALandscape>*, ActiveLandscape)
		SLATE_ARGUMENT(TObjectPtr<UErosionTemplateManager>, TemplateManager)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

private: // Members.
	TSharedPtr<FLandscapeGenerationSettings> Landscape;
	TSharedPtr<FErosionSettings> Erosion;

	TObjectPtr<ALandscape>* ActiveLandscape;
	TObjectPtr<UErosionTemplateManager> TemplateManager;

	// Wind UI data.
	TArray<TSharedPtr<FString>> WindDirections;
	TSharedPtr<FString> CurrentWindDirection;

	TSharedPtr<SComboBox<TSharedPtr<FString>>> WindCombo;

	const TCHAR* WindDirectionsNames[WIND_DIRECTIONS] =
	{
		TEXT("Random"), 

		TEXT("North"), 
		TEXT("South"), 
		TEXT("East"), 
		TEXT("West"),
		TEXT("North_West"), 
		TEXT("North_East"), 
		TEXT("South_West"), 
		TEXT("South_East")
	};

private: // Methods.
	void BuildWindDirections();

	FReply OnErodeClicked();
};
