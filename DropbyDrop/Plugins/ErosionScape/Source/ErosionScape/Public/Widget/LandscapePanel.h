// © Manuel Solano
// © Roberto Capparelli

#pragma once

#include "CoreMinimal.h"

#pragma region ForwardDeclarations

struct FLandscapeGenerationSettings;
struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;

class ALandscape;

#pragma endregion

class SLandscapePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLandscapePanel) {}
		SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)
		SLATE_ARGUMENT(TSharedPtr<FHeightMapGenerationSettings>, Heightmap)
		SLATE_ARGUMENT(TSharedPtr<FExternalHeightMapSettings>, External)
		SLATE_ARGUMENT(TObjectPtr<ALandscape>*, ActiveLandscape)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<FLandscapeGenerationSettings> Landscape;
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;
	TSharedPtr<FExternalHeightMapSettings> External;
	TObjectPtr<ALandscape>* ActiveLandscape;

	FReply OnCreateLandscapeClicked();
	FReply OnSplitInProxiesClicked();
};
