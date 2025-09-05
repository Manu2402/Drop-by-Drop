#pragma once
#include "CoreMinimal.h"
#include "ErosionScapeSettings.generated.h"

class ALandscape;

USTRUCT(BlueprintType)
struct FHeightMapGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	int32 Seed = -4314;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	bool bRandomizeSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	int32 NumOctaves = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	float Persistence = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	float Lacunarity = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	float InitialScale = 1.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	int32 Size = 505;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	float MaxHeightDifference = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap")
	TArray<float> HeightMap;
};
USTRUCT(BlueprintType)
struct FExternalHeightMapSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExternalHeightmap")
	float ScalingX = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExternalHeightmap")
	float ScalingY = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExternalHeightmap")
	float ScalingZ = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExternalHeightmap")
	bool bIsExternalHeightMap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ExternalHeightmap")
	FString LastPNGPath;
};

USTRUCT(BlueprintType)
struct FLandscapeGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
	int32 Kilometers = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
	bool bKilometers = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
	bool bDestroyLastLandscape = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
	int32 WorldPartitionGridSize = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape")
	TObjectPtr<ALandscape> TargetLandscape = nullptr;
};