#pragma once

#include "CoreMinimal.h"
#include "ErosionScapeSettings.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "GeneratorHeightMapLibrary.generated.h"

USTRUCT()
struct FErosionTemplateRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ErosionCyclesField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 WindDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InertiaField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CapacityField;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinSlopeField;

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
};

UCLASS()
class UGeneratorHeightMapLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static UDataTable* ErosionTemplatesDataTable;

#pragma region Erosion
	
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void GenerateErosion(const FExternalHeightMapSettings& ExternalSettings,
	                             FLandscapeGenerationSettings& LandscapeSettings,
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

	static void LoadRowIntoErosionFields(const FErosionTemplateRow* TemplateDatas);

	static UDataTable* GetErosionTemplates();
	static void SetErosionTemplates(const TCHAR* DataTablePath);
#pragma endregion

#pragma region Landscape
	
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void GenerateLandscapeFromPNG(const FString& HeightmapPath,
	                                      FHeightMapGenerationSettings& HeightmapSettings,
	                                      FExternalHeightMapSettings& ExternalSettings,
	                                      FLandscapeGenerationSettings& LandscapeSettings);

	static void CreateLandscapeFromOtherHeightMap(const FString& FilePath,
	FExternalHeightMapSettings& ExternalSettings,
	 FLandscapeGenerationSettings& LandscapeSettings, FHeightMapGenerationSettings& HeightmapSettings);

	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void SplitLandscapeIntoProxies(FLandscapeGenerationSettings& LandscapeSettings);

	static bool SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX, int32& MaxY,  const int32 Size = 505);

	static ALandscape* GenerateLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap);

	// --- One-click landscape creation (prefer preview asset, fallback to internal) ---
	static void GenerateLandscapeAuto(FHeightMapGenerationSettings& HeightmapSettings,
									  FExternalHeightMapSettings&   ExternalSettings,
									  FLandscapeGenerationSettings& LandscapeSettings);

#pragma endregion

#pragma region Heightmap
	
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static bool CreateAndSaveHeightMap(FHeightMapGenerationSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static TArray<float> CreateHeightMapArray(const FHeightMapGenerationSettings& Settings);

	//Function to generate texture2D, using an array<float>
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
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

	//Wind Preview
	static float WindPreviewScale;

	UFUNCTION(BlueprintCallable, Category="Erosion|Debug")
	static void SetWindPreviewScale(float NewScale) { WindPreviewScale = NewScale; }

	UFUNCTION(BlueprintCallable, Category="Erosion|Debug")
	static float GetWindPreviewScale() { return WindPreviewScale; }
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Erosion|Debug")
	static void DrawWindDirectionPreview(
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
