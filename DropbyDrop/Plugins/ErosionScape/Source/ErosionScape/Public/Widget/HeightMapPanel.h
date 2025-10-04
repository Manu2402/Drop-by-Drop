#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ErosionScapeSettings.h"
#include "Widgets/Input/SComboBox.h"
#include "UObject/StrongObjectPtr.h"

class SHeightMapPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHeightMapPanel) {}
	SLATE_ARGUMENT(TSharedPtr<FHeightMapGenerationSettings>, Heightmap)
	SLATE_ARGUMENT(TSharedPtr<FExternalHeightMapSettings>,   External)
	SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)
SLATE_END_ARGS()

void Construct(const FArguments& Args);

private:
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;
	TSharedPtr<FExternalHeightMapSettings>   External;
	TSharedPtr<FLandscapeGenerationSettings> Landscape;

	// Preview
	TSharedPtr<FSlateBrush>       HeightPreviewBrush;
	TStrongObjectPtr<UTexture2D>  HeightPreviewTexture;
	TSharedPtr< SImage>      PreviewImage;
	void RefreshPreview();

	FHeightMapGenerationSettings PresetLight;
	FHeightMapGenerationSettings PresetMedium;
	FHeightMapGenerationSettings PresetHeavy;
	
	TArray<TSharedPtr<FString>> PresetItems;
	TSharedPtr<FString>         SelectedPreset;
	
	void InitPresets();
	void ApplyPreset(const FHeightMapGenerationSettings& Preset);

	void BuildPresetItems();
	void ApplyPresetByName(const FString& Name);
	void OnPresetChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	TSharedRef<SWidget> MakePresetItemWidget(TSharedPtr<FString> Item) const;


	
	// Callbacks 
	FReply OnPresetLightClicked();
	FReply OnPresetMediumClicked();
	FReply OnPresetHeavyClicked();

	FReply OnCreateHeightmapClicked();
	FReply OnCreateLandscapeFromInternalClicked();
	FReply OnCreateLandscapeFromPNGClicked();
};
