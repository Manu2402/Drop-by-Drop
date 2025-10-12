// © Manuel Solano
// © Roberto Capparelli

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GeneratorHeightMapLibrary.generated.h"

#pragma region ForwardDeclarations

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;
struct FErosionSettings;
class FDropByDropSettings;

class ALandscape;

#pragma endregion

#pragma region DataStructures

USTRUCT(BlueprintType)
struct FErosionTemplateRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ErosionCyclesField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InertiaField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CapacityField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinimalSlopeField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DepositionSpeedField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ErosionSpeedField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GravityField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EvaporationField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPathField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ErosionRadiusField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 WindDirection;
};

enum class ERawFormat
{
	Unknown,

	BGRA8,
	RGBA8, 
	G8,
	G16,
	R32F,
	RGBA16F
};

#pragma endregion

UCLASS()
class UGeneratorHeightMapLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region Erosion + Templates
	static void GenerateErosion(TObjectPtr<ALandscape> ActiveLandscape, FErosionSettings& ErosionSettings);

	static bool SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue, const float InertiaValue, const int32 CapacityValue, const float MinSlopeValue, const float DepositionSpeedValue, const float ErosionSpeedValue, const int32 GravityValue, const float EvaporationValue, const int32 MaxPathValue, const int32 ErosionRadiusValue);
	static bool SaveErosionTemplates(UDataTable* ErosionTemplatesDT);

	static FErosionTemplateRow* LoadErosionTemplate(const FString& TemplateName);
	static void LoadRowIntoErosionFields(TSharedPtr<FErosionSettings>& OutErosionSettings, const FErosionTemplateRow* TemplateDatas);

	static bool DeleteErosionTemplate(const FString& TemplateName);
#pragma endregion

#pragma region Heightmap
	static bool CreateAndSaveHeightMap(FHeightMapGenerationSettings& Settings);

	static TArray<float> CreateHeightMapArray(const FHeightMapGenerationSettings& Settings);
	static UTexture2D* CreateHeightMapTexture(const TArray<float>& HeightMapData, const int32 Width, const int32 Height);

	static void OpenHeightmapFileDialog(TSharedPtr<FExternalHeightMapSettings> ExternalSettings);
	static void LoadHeightmapFromFileSystem(const FString& FilePath, TArray<uint16>& OutHeightmap, TArray<float>& OutNormalizedHeightmap, FExternalHeightMapSettings& Settings);

	static void CompareHeightmaps(const FString& RawFilePath, const TArray<uint16>& GeneratedHeightmap, int32 Width, int32 Height);
#pragma endregion

#pragma region Landscape
	static void CreateLandscapeFromInternalHeightMap(FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings);
	static void CreateLandscapeFromExternalHeightMap(const FString& FilePath, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings, FHeightMapGenerationSettings& HeightmapSettings);
	static void GenerateLandscapeAuto(FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings);
	static TObjectPtr<ALandscape> GenerateLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap);

	static void SplitLandscapeIntoProxies(ALandscape& LandscapeSettings);

	static bool SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX, int32& MaxY, const int32 Size = 505);
#pragma endregion

#pragma region Utilities
	static TArray<uint16> ConvertArrayFromFloatToUInt16(const TArray<float>& FloatData);

	static bool SaveToAsset(UTexture2D* Texture, const FString& AssetName);

	static float GetWindPreviewScale();
	static void SetWindPreviewScale(const float NewScale);

	static void DrawWindDirectionPreview(const FErosionSettings& ErosionSettings, const ALandscape* SelectedLandscape, float ArrowLength = 8000.f, float ArrowThickness = 12.f, float ArrowHeadSize = 300.f, float Duration = 5.f, bool bAlsoDrawCone = true, float ConeHalfAngleDeg = 15.f);
#pragma endregion

private:
#pragma region Utilities (Private)
	static FTransform GetNewTransform(const FExternalHeightMapSettings& ExternalSettings, const FLandscapeGenerationSettings& LandscapeSettings, int32 HeightmapSize);
#pragma endregion

#pragma region Landscape (Private)
	static bool InitLandscape(TArray<uint16>& HeightData, FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings);
	static FVector GetWindPreviewStart(const ALandscape* ActiveLandscape, UWorld* World);
#pragma endregion

};
