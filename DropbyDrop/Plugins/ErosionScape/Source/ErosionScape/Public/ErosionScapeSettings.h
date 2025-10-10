#pragma once
#include "CoreMinimal.h"
#include "ErosionScapeSettings.generated.h"

class ALandscape;

UENUM(BlueprintType)
enum EWindDirection : uint8
{
	Random,
	Est,
	Nord_Est,
	Nord,
	Nord_Ovest,
	Ovest,
	Sud_Ovest,
	Sud,
	Sud_Est
};

USTRUCT(BlueprintType)
struct FHeightMapGenerationSettings
{
	GENERATED_BODY()

	int32 Seed = -4314;
	bool bRandomizeSeed = false;
	uint32 NumOctaves = 8;
	float Persistence = 0.45f;
	float Lacunarity = 2.f;
	float InitialScale = 1.8f;
	uint32 Size = 505; // Const.
	float MaxHeightDifference = 1.0f;

	TArray<float> HeightMap;
};

USTRUCT(BlueprintType)
struct FExternalHeightMapSettings
{
	GENERATED_BODY()

	float ScalingX = 100.0f;
	float ScalingY = 100.0f;
	float ScalingZ = 100.0f;
	bool bIsExternalHeightMap = false;

	FString LastPNGPath;
};

USTRUCT(BlueprintType)
struct FLandscapeGenerationSettings
{
	GENERATED_BODY()

	uint32 Kilometers = 1;
	bool bKilometers = true;
	uint32 WorldPartitionCellSize = 4;
};

USTRUCT(BlueprintType)
struct FErosionSettings
{
	GENERATED_BODY()

	uint64 ErosionCycles = 100000;
	uint8 WindDirection = 0;
	bool bWindBias = false;
	float Inertia = 0.3f;
	uint32 Capacity = 8;
	float MinimalSlope = 0.01f;
	float DepositionSpeed = 0.2f;
	float ErosionSpeed = 0.7f;
	uint32 Gravity = 10;
	float Evaporation = 0.02f;
	uint32 MaxPath = 64;
	int32 ErosionRadius = 4;
};

class FDropByDropSettings
{
public:
	static FDropByDropSettings& Get();
		
	float GetWindPreviewScale() const;
	void SetWindPreviewScale(const float NewWindPreviewScale);

	UDataTable* GetErosionTemplatesDT() const;
	void SetErosionTemplatesDT(UDataTable* NewErosionTemplatesDT);

private:
	FDropByDropSettings() = default;
	FDropByDropSettings(const FDropByDropSettings& Other) = delete;
	FDropByDropSettings& operator=(const FDropByDropSettings& Other) = delete;
	FDropByDropSettings(FDropByDropSettings&& Other) = delete;
	FDropByDropSettings& operator=(FDropByDropSettings&& Other) = delete;

	float WindPreviewScale = 10.0f;
	UDataTable* ErosionTemplatesDT = nullptr;

};
