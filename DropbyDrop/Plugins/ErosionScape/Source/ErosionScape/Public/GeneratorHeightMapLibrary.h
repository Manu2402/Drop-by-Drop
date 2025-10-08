#pragma once

#include "CoreMinimal.h"
#include "ErosionScapeSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "ErosionScapeSettings.h"
#include "GeneratorHeightMapLibrary.generated.h"

USTRUCT(BlueprintType)
struct FErosionTemplateRow : public FTableRowBase
{
	GENERATED_BODY()

	uint64 ErosionCyclesField;
	uint8 WindDirection;
	float InertiaField;
	uint32 CapacityField;
	float MinSlopeField;
	float DepositionSpeedField;
	float ErosionSpeedField;
	uint32 GravityField;
	float EvaporationField;
	uint32 MaxPathField;
	int32 ErosionRadiusField;
};

UCLASS()
class UGeneratorHeightMapLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region Erosion
	
	static void GenerateErosion(const FExternalHeightMapSettings& ExternalSettings,
	                             FLandscapeGenerationSettings& LandscapeSettings,
		                          FErosionSettings& ErosionSettings,
	                             const FHeightMapGenerationSettings& HeightMapSetting,
	                            int32 HeightMapSize);
	
	static bool SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue,
	                                const float InertiaValue, const int32 CapacityValue,
	                                const float MinSlopeValue, const float DepositionSpeedValue,
	                                const float ErosionSpeedValue, const int32 GravityValue,
	                                const float EvaporationValue, const int32 MaxPathValue,
	                                const int32 ErosionRadiusValue);

	static FErosionTemplateRow* LoadErosionTemplate(const FString& TemplateName);
	static bool DeleteErosionTemplate(const FString& TemplateName);
	static bool SaveErosionTemplates();

	static void LoadRowIntoErosionFields(TSharedPtr<FErosionSettings>& OutErosionSettings, const FErosionTemplateRow* TemplateDatas);

	static UDataTable* GetErosionTemplates();
	static void SetErosionTemplates(const TCHAR* DataTablePath);
#pragma endregion

#pragma region Landscape
	
	static void GenerateLandscapeFromPNG(const FString& HeightmapPath,
	                                      FHeightMapGenerationSettings& HeightmapSettings,
	                                      FExternalHeightMapSettings& ExternalSettings,
	                                      FLandscapeGenerationSettings& LandscapeSettings);

	static void CreateLandscapeFromOtherHeightMap(const FString& FilePath,
	FExternalHeightMapSettings& ExternalSettings,
	 FLandscapeGenerationSettings& LandscapeSettings, FHeightMapGenerationSettings& HeightmapSettings);

	static void SplitLandscapeIntoProxies(FLandscapeGenerationSettings& LandscapeSettings);

	static bool SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX, int32& MaxY,  const int32 Size = 505);

	static ALandscape* GenerateLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap);

	// --- One-click landscape creation (prefer preview asset, fallback to internal) ---
	static void GenerateLandscapeAuto(FHeightMapGenerationSettings& HeightmapSettings,
									  FExternalHeightMapSettings&   ExternalSettings,
									  FLandscapeGenerationSettings& LandscapeSettings);

#pragma endregion

#pragma region Heightmap
	
	static bool CreateAndSaveHeightMap(FHeightMapGenerationSettings& Settings);

	static TArray<float> CreateHeightMapArray(const FHeightMapGenerationSettings& Settings);

	//Function to generate texture2D, using an array<float>
	static UTexture2D* CreateHeightMapTexture(const TArray<float>& HeightMapData, const int32 Width,
	                                          const int32 Height);

	static void LoadHeightmapFromPNG(const FString& FilePath, TArray<uint16>& OutHeightmap,
	                                 TArray<float>& OutNormalizedHeightmap, FExternalHeightMapSettings& Settings);

#pragma endregion
#pragma region Utilities

	static TArray<uint16> ConvertFloatArrayToUint16(const TArray<float>& FloatData);
	static bool SaveToAsset(UTexture2D* Texture, const FString& AssetName);
	static void OpenHeightmapFileDialog(
		TSharedPtr<struct FExternalHeightMapSettings> ExternalSettings = nullptr,
		TSharedPtr<struct FLandscapeGenerationSettings> LandscapeSettings = nullptr,
		TSharedPtr<struct FHeightMapGenerationSettings> HeightMapSettings = nullptr
	);

	static void SetWindPreviewScale(float NewScale) { FDropByDropSettings::Get().WindPreviewScale = NewScale; }

	static float GetWindPreviewScale() { return FDropByDropSettings::Get().WindPreviewScale; }
	
	static void DrawWindDirectionPreview(
		const FErosionSettings& ErosionSettings,
		const FLandscapeGenerationSettings& LandscapeSettings,
		float ArrowLength      = 8000.f,
		float ArrowThickness   = 12.f,
		float ArrowHeadSize    = 300.f,
		float Duration         = 5.f,
		bool  bAlsoDrawCone    = true,
		float ConeHalfAngleDeg = 15.f);
	// ---
	
private:
	static FTransform GetNewTransform(const FExternalHeightMapSettings& ExternalSettings,
	                                  const FLandscapeGenerationSettings& LandscapeSettings,
	                                  int32 HeightmapSize);
	static void DestroyLastLandscape(const FLandscapeGenerationSettings& LandscapeSettings);
#pragma endregion

};
