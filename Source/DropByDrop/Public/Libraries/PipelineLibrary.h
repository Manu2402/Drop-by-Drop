// © Manuel Solano
// © Roberto Capparelli

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PipelineLibrary.generated.h"

#pragma region ForwardDeclarations

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;
struct FErosionSettings;
class FDropByDropSettings;

class ALandscape;

#pragma endregion

#pragma region DataStructures

/**
 * Data structure representing a row in an erosion template data table.
 * This struct is used to store preset configurations for terrain erosion simulation,
 * allowing users to save and load different erosion profiles.
 */
USTRUCT(BlueprintType)
struct FErosionTemplateRow : public FTableRowBase
{
	GENERATED_BODY()

	// Number of erosion simulation cycles to perform.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	int64 ErosionCyclesField;

	// How much the water droplet maintains its direction of flow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	float InertiaField;

	// Maximum amount of sediment a water droplet can carry.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	int32 CapacityField;

	// Minimum terrain slope required for erosion to occur.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	float MinimalSlopeField;

	// Rate at which sediment is deposited when the droplet slows down.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	float DepositionSpeedField;

	// Rate at which terrain is eroded by flowing water.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	float ErosionSpeedField;

	// Gravity strength affecting water flow.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	int32 GravityField;

	// Rate at which water evaporates, reducing erosion capacity.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	float EvaporationField;

	// Maximum path length a water droplet can travel.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	int32 MaxPathField;

	// Radius of influence for erosion effects (in "cells" units).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	int32 ErosionRadiusField;

	// Direction of wind for wind-based erosion effects.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ErosionTemplateRow")
	uint8 WindDirection;
};

/**
 * Enumeration of supported raw image formats for heightmap data.
 * These formats represent different pixel/channel configurations.
 */
enum class ERawFormat
{
	Unknown,    // Unrecognized format.

	BGRA8,      // 8-bit per channel, Blue-Green-Red-Alpha order.
	RGBA8,      // 8-bit per channel, Red-Green-Blue-Alpha order.
	G8,         // 8-bit grayscale (single channel).
	G16,        // 16-bit grayscale (single channel, high precision).
	R32F,       // 32-bit floating point red channel.
	RGBA16F     // 16-bit floating point per channel (HDR).
};

#pragma endregion

/**
 * Blueprint Function Library providing static utility functions for procedural landscape generation.
 * This library handles heightmap generation, erosion simulation, landscape creation, and template management.
 */
UCLASS()
class UPipelineLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#pragma region Erosion + Templates

	/**
	 * Applies erosion simulation to an existing landscape.
	 * @param ActiveLandscape - The landscape to apply erosion to.
	 * @param ErosionSettings - Configuration settings for the erosion algorithm.
	 */
	static void GenerateErosion(TObjectPtr<ALandscape> ActiveLandscape, FErosionSettings& ErosionSettings);

	/**
	 * Saves a new erosion template with specified parameters to persistent storage.
	 * @param TemplateName - Name identifier for the template.
	 * @param ErosionCyclesValue - Number of simulation cycles.
	 * @param InertiaValue - Flow direction persistence.
	 * @param CapacityValue - Sediment carrying capacity.
	 * @param MinSlopeValue - Minimum slope for erosion.
	 * @param DepositionSpeedValue - Sediment deposition rate.
	 * @param ErosionSpeedValue - Terrain erosion rate.
	 * @param GravityValue - Gravity strength.
	 * @param EvaporationValue - Water evaporation rate.
	 * @param MaxPathValue - Maximum droplet path length.
	 * @param ErosionRadiusValue - Erosion effect radius.
	 * @return True if save was successful, false otherwise.
	 */
	static bool SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue, const float InertiaValue, const int32 CapacityValue, const float MinSlopeValue, const float DepositionSpeedValue, const float ErosionSpeedValue, const int32 GravityValue, const float EvaporationValue, const int32 MaxPathValue, const int32 ErosionRadiusValue);

	/**
	 * Saves an entire data table of erosion templates.
	 * @param ErosionTemplatesDT - Data table containing multiple erosion templates.
	 * @return True if save was successful, false otherwise.
	 */
	static bool SaveErosionTemplates(UDataTable* ErosionTemplatesDT);

	/**
	 * Loads a previously saved erosion template by name.
	 * @param TemplateName - Name of the template to load.
	 * @return Pointer to the loaded template data, or nullptr if not found.
	 */
	static FErosionTemplateRow* LoadErosionTemplate(const FString& TemplateName);

	/**
	 * Populates erosion settings from a template row.
	 * @param OutErosionSettings - Settings structure to populate.
	 * @param TemplateDatas - Source template data to copy from.
	 */
	static void LoadRowIntoErosionFields(TSharedPtr<FErosionSettings>& OutErosionSettings, const FErosionTemplateRow* TemplateDatas);

	/**
	 * Deletes a saved erosion template.
	 * @param TemplateName - Name of the template to delete.
	 * @return True if deletion was successful, false otherwise.
	 */
	static bool DeleteErosionTemplate(const FString& TemplateName);
#pragma endregion

#pragma region Heightmap

	/**
	 * Generates a heightmap and saves it as an asset.
	 * @param HeightMapSettings - Configuration for heightmap generation.
	 * @return True if creation and save were successful, false otherwise.
	 */
	static bool CreateAndSaveHeightMap(FHeightMapGenerationSettings& HeightMapSettings);

	/**
	 * Generates a heightmap as an array of normalized float values.
	 * @param HeightMapSettings - Generation parameters.
	 * @return Array of height values.
	 */
	static TArray<float> CreateHeightMapArray(const FHeightMapGenerationSettings& HeightMapSettings);

	/**
	 * Creates a "Texture2D" asset from heightmap data for visualization and export.
	 * @param HeightMapData - Array of normalized height values.
	 * @param Width - Texture width in pixels.
	 * @param Height - Texture height in pixels.
	 * @return Generated texture object.
	 */
	static UTexture2D* CreateHeightMapTexture(const TArray<float>& HeightMapData, const int32 Width, const int32 Height);

	/**
	 * Opens a file dialog for the user to select an external heightmap file.
	 * @param ExternalSettings - Settings object to populate with file information.
	 * @return True if a file was successfully selected, false if the dialog was canceled.
	 */
	static bool OpenHeightmapFileDialog(TSharedPtr<FExternalHeightMapSettings> ExternalSettings);

	/**
	 * Loads heightmap data from an external file (RAW, PNG, etc.).
	 * @param FilePath - Full path to the heightmap file.
	 * @param OutHeightmap - Output array of 16-bit height values.
	 * @param OutNormalizedHeightmap - Output array of normalized height values.
	 * @param Settings - Settings to populate with file metadata.
	 */
	static void LoadHeightmapFromFileSystem(const FString& FilePath, TArray<uint16>& OutHeightmap, TArray<float>& OutNormalizedHeightmap, FExternalHeightMapSettings& Settings);

	/**
	 * Debugging utility to compare a generated heightmap with a RAW file on disk.
	 * @param RawFilePath - Path to the reference RAW file.
	 * @param GeneratedHeightmap - Generated heightmap to compare.
	 * @param Width - Heightmap width.
	 * @param Height - Heightmap height.
	 */
	static void CompareHeightmaps(const FString& RawFilePath, const TArray<uint16>& GeneratedHeightmap, int32 Width, int32 Height);
#pragma endregion

#pragma region Landscape

	/**
	 * Creates a landscape actor using internally generated heightmap data.
	 * @param HeightmapSettings - Settings for heightmap generation.
	 * @param ExternalSettings - Imported from file system heightmap settings.
	 * @param LandscapeSettings - Landscape creation parameters (size, scale, etc.).
	 */
	static void CreateLandscapeFromInternalHeightMap(FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings);

	/**
	 * Creates a landscape actor from an external heightmap file.
	 * @param FilePath - Path to the external heightmap file.
	 * @param ExternalSettings - Imported from file system heightmap settings.
	 * @param LandscapeSettings - Landscape creation parameters.
	 * @param HeightmapSettings - Heightmap settings for processing.
	 */
	static void CreateLandscapeFromExternalHeightMap(const FString& FilePath, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings, FHeightMapGenerationSettings& HeightmapSettings);

	/**
	 * Automated landscape generation pipeline combining generation and creation.
	 * @param HeightmapSettings - Heightmap generation settings.
	 * @param ExternalSettings - Imported from file system heightmap settings.
	 * @param LandscapeSettings - Landscape creation settings.
	 */
	static void GenerateLandscapeAuto(FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings);

	/**
	 * Core function to spawn a landscape actor with given heightmap data.
	 * @param LandscapeTransform - World transform (location, rotation, scale) for the landscape.
	 * @param Heightmap - 16-bit heightmap data.
	 * @return Pointer to the created landscape.
	 */
	static TObjectPtr<ALandscape> GenerateLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap);

	/**
	 * Divides a large landscape into multiple streaming proxies for performance optimization.
	 * @param LandscapeSettings - The landscape to split.
	 */
	static void SplitLandscapeIntoProxies(ALandscape& LandscapeSettings);

	/**
	 * Calculates and validates landscape size parameters based on desired dimensions.
	 * Unreal landscapes have specific size requirements (power-of-2 based).
	 * @param SubSectionSizeQuads - Output: quads per subsection.
	 * @param NumSubsections - Output: number of subsections.
	 * @param MaxX - Output: maximum X dimension.
	 * @param MaxY - Output: maximum Y dimension.
	 * @param Size - Desired size (default 505).
	 * @return True if parameters are valid.
	 */
	static bool SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX, int32& MaxY, const int32 Size = 505);
#pragma endregion

#pragma region Utilities

	/**
	 * Converts normalized float heightmap data to 16-bit unsigned integer format.
	 * This is necessary for Unreal's landscape system which uses uint16 for heightmaps.
	 * @param FloatData - Input array of normalized float values.
	 * @return Array of 16-bit unsigned integers (0-65535 range).
	 */
	static TArray<uint16> ConvertArrayFromFloatToUInt16(const TArray<float>& FloatData);

	/**
	 * Saves a texture as a persistent asset in the Unreal content browser.
	 * @param Texture - Texture to save.
	 * @param AssetName - Name for the asset.
	 * @return True if save was successful, false otherwise.
	 */
	static bool SaveToAsset(UTexture2D* Texture, const FString& AssetName);

	/**
	 * Gets the current scale factor for wind direction preview visualization.
	 * @return Current scale value.
	 */
	static float GetWindPreviewScale();

	/**
	 * Sets the scale factor for wind direction preview visualization.
	 * @param NewScale - New scale value to apply.
	 */
	static void SetWindPreviewScale(const float NewScale);

	/**
	 * Draws a debug visualization of wind direction on the landscape in the editor viewport.
	 * @param ErosionSettings - Erosion settings containing wind direction data.
	 * @param SelectedLandscape - Landscape to draw the preview on.
	 * @param ArrowLength - Length of the direction arrow.
	 * @param ArrowThickness - Thickness of the arrow line.
	 * @param ArrowHeadSize - Size of the arrow head.
	 * @param Duration - How long to display the preview in seconds.
	 * @param bAlsoDrawCone - Whether to also draw a cone showing wind spread.
	 * @param ConeHalfAngleDeg - Half-angle of the wind cone in degrees.
	 */
	static void DrawWindDirectionPreview(const FErosionSettings& ErosionSettings, const ALandscape* SelectedLandscape, float ArrowLength = 8000.f, float ArrowThickness = 12.f, float ArrowHeadSize = 300.f, float Duration = 5.f, bool bAlsoDrawCone = true, float ConeHalfAngleDeg = 15.f);
#pragma endregion

private:
#pragma region Utilities (Private)

	/**
	 * Calculates the world transform for a new landscape based on settings and heightmap size.
	 * @param ExternalSettings - Imported from file system heightmap settings.
	 * @param LandscapeSettings - Landscape generation settings.
	 * @param HeightmapSize - Size of the heightmap.
	 * @return Calculated transform for landscape placement.
	 */
	static FTransform GetNewTransform(const FExternalHeightMapSettings& ExternalSettings, const FLandscapeGenerationSettings& LandscapeSettings, int32 HeightmapSize);
#pragma endregion

#pragma region Landscape (Private)

	/**
	 * Initializes landscape data structures and validates settings before creation.
	 * @param HeightData - Heightmap data array.
	 * @param HeightmapSettings - Heightmap generation settings.
	 * @param ExternalSettings - Imported from file system file settings.
	 * @param LandscapeSettings - Landscape creation settings.
	 * @return True if initialization was successful, false otherwise.
	 */
	static bool InitLandscape(TArray<uint16>& HeightData, FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings);

	/**
	 * Calculates the starting position for wind preview visualization on the landscape.
	 * @param ActiveLandscape - The landscape to calculate position for.
	 * @param World - Current world context.
	 * @return Starting position vector for wind preview.
	 */
	static FVector GetWindPreviewStart(const ALandscape* ActiveLandscape, UWorld* World);
#pragma endregion

};