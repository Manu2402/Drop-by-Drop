#pragma once
#include "CoreMinimal.h"
#include "ErosionScapeSettings.generated.h"

class ALandscape;

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
	bool bDestroyLastLandscape = false;
	uint32 WorldPartitionCellSize = 4;
	bool bIsSplittedIntoProxies = false;

	TObjectPtr<ALandscape> TargetLandscape = nullptr;
};