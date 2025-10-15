// © Manuel Solano
// © Roberto Capparelli

#pragma once

#include "CoreMinimal.h"
#include "DropByDropSettings.generated.h"

#pragma region ForwardDeclarations

class ALandscape;

#pragma endregion

#pragma region Heightmap

/**
 * FHeightMapGenerationSettings
 *
 * Configuration structure for procedural heightmap generation using Perlin noise.
 * Contains parameters to control the noise generation algorithm that creates
 * the base terrain elevation data.
 */
USTRUCT(BlueprintType)
struct FHeightMapGenerationSettings
{
	GENERATED_BODY()

	/** Array storing the generated height values for each point in the terrain. */
	TArray<float> HeightMap;

	/** Seed value for the random number generator. Determines the terrain pattern. */
	int32 Seed = -4314;

	/** Number of noise octaves to combine. */
	uint32 NumOctaves = 8;

	/** Controls how much each octave contributes. */
	float Persistence = 0.45f;

	/** Frequency multiplier between octaves. */
	float Lacunarity = 2.f;

	/** Starting scale of the noise pattern. */
	float InitialScale = 1.8f;

	/** Size of the heightmap grid (constant at 505x505). */
	uint32 Size = 505;

	/** Maximum allowed height difference for terrain normalization. */
	float MaxHeightDifference = 1.0f;

	/** If true, generates a new random seed each time. */
	bool bRandomizeSeed = false;
};

/**
 * FExternalHeightMapSettings
 *
 * Configuration for importing heightmaps from external PNG files.
 * Allows users to load pre-made terrain data instead of generating it procedurally.
 */
USTRUCT(BlueprintType)
struct FExternalHeightMapSettings
{
	GENERATED_BODY()

	/** Path to the last loaded PNG heightmap file. */
	FString LastPNGPath;

	/** Horizontal (X-axis) scaling factor for the imported heightmap. */
	float ScalingX = 100.0f;

	/** Horizontal (Y-axis) scaling factor for the imported heightmap. */
	float ScalingY = 100.0f;

	/** Vertical (Z-axis/height) scaling factor for the imported heightmap. */
	float ScalingZ = 100.0f;

	/** Flag indicating whether an external heightmap is being used. */
	bool bIsExternalHeightMap = false;
};

#pragma endregion

/**
 * FLandscapeGenerationSettings
 *
 * Settings for generating the Unreal Engine landscape.
 * Controls the physical size and world partition configuration.
 */
USTRUCT(BlueprintType)
struct FLandscapeGenerationSettings
{
	GENERATED_BODY()

	/** Size of the landscape in kilometers. */
	uint32 Kilometers = 1;

	/** Size of each world partition cell for streaming (in grid units). */
	uint32 WorldPartitionCellSize = 4;
};

#pragma region Erosion

/**
 * FErosionSettings
 *
 * Parameters for the hydraulic erosion simulation algorithm.
 * This simulates water droplets eroding the terrain over time, creating
 * realistic valleys, rivers, and weathering effects.
 */
USTRUCT(BlueprintType)
struct FErosionSettings
{
	GENERATED_BODY()

	/** Total number of water droplet simulations to run. */
	int64 ErosionCycles = 100000;

	/** How much the droplet retains its direction. */
	float Inertia = 0.3f;

	/** Maximum amount of sediment a droplet can carry. */
	int32 Capacity = 8;

	/** Minimum slope required for erosion to occur. */
	float MinimalSlope = 0.01f;

	/** Rate at which sediment is deposited when droplet slows down. */
	float DepositionSpeed = 0.2f;

	/** Rate at which terrain is eroded by moving water. */
	float ErosionSpeed = 0.7f;

	/** Gravitational force affecting water droplet movement. */
	int32 Gravity = 10;

	/** Rate at which water evaporates, reducing the droplet's volume. */
	float Evaporation = 0.02f;

	/** Maximum number of steps a single droplet can take before terminating. */
	int32 MaxPath = 64;

	/** Radius around droplet position affected by erosion/deposition. */
	int32 ErosionRadius = 4;

	/** If true, droplets are biased to move in a specific wind direction. */
	bool bWindBias = false;

	/** Compass direction for wind bias (see "EWindDirection" enum). */
	uint8 WindDirection = 0;
};

/**
 * EWindDirection
 *
 * Enumeration of wind directions for biasing erosion patterns.
 * Used to create directional weathering effects on the terrain.
 */
UENUM(BlueprintType)
enum EWindDirection : uint8
{
	Random,    
	East,      
	North_East,
	North,     
	North_West,
	West,      
	South_West,
	South,     
	South_East 
};

#pragma endregion

/**
 * FDropByDropSettings
 *
 * Singleton class managing global settings for the "DropByDrop" plugin.
 * Provides centralized access to configuration data and erosion templates.
 * Uses the Meyer's Singleton pattern for thread-safe lazy initialization.
 */
class FDropByDropSettings
{
public:
	/**
	 * Returns the singleton instance of the settings manager.
	 * Thread-safe in C++11 and later due to static local variable initialization.
	 *
	 * @return Reference to the singleton instance.
	 */
	static FDropByDropSettings& Get();

	/**
	 * Retrieves the current scale factor for wind direction preview visualization.
	 *
	 * @return Current wind preview scale value.
	 */
	float GetWindPreviewScale() const;

	/**
	 * Sets the scale factor for wind direction preview visualization in the editor.
	 *
	 * @param NewWindPreviewScale - The new scale value to apply.
	 */
	void SetWindPreviewScale(const float NewWindPreviewScale);

	/**
	 * Retrieves the data table containing predefined erosion presets.
	 *
	 * @return Pointer to the erosion templates data table.
	 */
	UDataTable* GetErosionTemplatesDT() const;

	/**
	 * Sets the data table containing erosion preset templates.
	 *
	 * @param NewErosionTemplatesDT - Pointer to the new data table.
	 */
	void SetErosionTemplatesDT(UDataTable* NewErosionTemplatesDT);

private:
	/** Private constructor to enforce singleton pattern. */
	FDropByDropSettings();

	/** Deleted copy constructor to prevent copying. */
	FDropByDropSettings(const FDropByDropSettings& Other) = delete;

	/** Deleted copy assignment operator to prevent copying. */
	FDropByDropSettings& operator=(const FDropByDropSettings& Other) = delete;

	/** Deleted move constructor to prevent moving. */
	FDropByDropSettings(FDropByDropSettings&& Other) = delete;

	/** Deleted move assignment operator to prevent moving. */
	FDropByDropSettings& operator=(FDropByDropSettings&& Other) = delete;

	/** Data table storing pre-configured erosion setting templates. */
	UDataTable* ErosionTemplatesDT;

	/** Scale factor for visualizing wind direction in the editor preview. */
	float WindPreviewScale;

};