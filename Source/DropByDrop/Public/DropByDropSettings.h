// © Manuel Solano
// © Roberto Capparelli

#pragma once

#include "CoreMinimal.h"
#include "DropByDropSettings.generated.h"

#pragma region ForwardDeclarations

class ALandscape;

#pragma endregion

#pragma region Heightmap

USTRUCT(BlueprintType)
struct FHeightMapGenerationSettings
{
	GENERATED_BODY()

	TArray<float> HeightMap;

	int32 Seed = -4314;
	uint32 NumOctaves = 8;
	float Persistence = 0.45f;
	float Lacunarity = 2.f;
	float InitialScale = 1.8f;
	uint32 Size = 505; // Const.
	float MaxHeightDifference = 1.0f;

	bool bRandomizeSeed = false;
};

USTRUCT(BlueprintType)
struct FExternalHeightMapSettings
{
	GENERATED_BODY()

	FString LastPNGPath;

	float ScalingX = 100.0f;
	float ScalingY = 100.0f;
	float ScalingZ = 100.0f;

	bool bIsExternalHeightMap = false;
};

#pragma endregion

USTRUCT(BlueprintType)
struct FLandscapeGenerationSettings
{
	GENERATED_BODY()

	uint32 Kilometers = 1;
	uint32 WorldPartitionCellSize = 4;
};

#pragma region Erosion

USTRUCT(BlueprintType)
struct FErosionSettings
{
	GENERATED_BODY()

	int64 ErosionCycles = 100000;
	float Inertia = 0.3f;
	int32 Capacity = 8;
	float MinimalSlope = 0.01f;
	float DepositionSpeed = 0.2f;
	float ErosionSpeed = 0.7f;
	int32 Gravity = 10;
	float Evaporation = 0.02f;
	int32 MaxPath = 64;
	int32 ErosionRadius = 4;

	bool bWindBias = false;
	uint8 WindDirection = 0;
};

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

class FDropByDropSettings
{
public:
	static FDropByDropSettings& Get();
		
	float GetWindPreviewScale() const;
	void SetWindPreviewScale(const float NewWindPreviewScale);

	UDataTable* GetErosionTemplatesDT() const;
	void SetErosionTemplatesDT(UDataTable* NewErosionTemplatesDT);

private:
	FDropByDropSettings();
	FDropByDropSettings(const FDropByDropSettings& Other) = delete;
	FDropByDropSettings& operator=(const FDropByDropSettings& Other) = delete;
	FDropByDropSettings(FDropByDropSettings&& Other) = delete;
	FDropByDropSettings& operator=(FDropByDropSettings&& Other) = delete;

	UDataTable* ErosionTemplatesDT;
	float WindPreviewScale;

};
