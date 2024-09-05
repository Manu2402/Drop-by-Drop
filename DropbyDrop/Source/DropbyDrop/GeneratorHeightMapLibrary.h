// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "GeneratorHeightMapLibrary.generated.h"

UCLASS()
class DROPBYDROP_API UGeneratorHeightMapLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	public:
	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generation", meta = (Tooltip = "Seed used to initialize the random number generator.", DisplayName = "Random Seed"))
	int32 Seed = -4314;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generation", meta = (Tooltip = "Determines whether a random seed is generated for each heightmap. If false, the specified Seed value will be used."))
	bool bRandomizeSeed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generation", meta = (Tooltip = "The number of octaves used in Perlin noise generation. More octaves result in more detailed heightmaps."))
	int32 NumOctaves = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generation", meta = (Tooltip = "Controls the amplitude of successive octaves. Lower values create smoother heightmaps."))
	float Persistence = 0.45f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generation", meta = (Tooltip = "Controls the frequency of successive octaves. Higher values create more detailed, higher frequency features."))
	float Lacunarity = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generation", meta = (Tooltip = "The initial scale applied to the Perlin noise calculation. Controls the overall size of features in the heightmap."))
	float InitialScale = 1.8f;
*/

	static int32 Seed;
	static bool bRandomizeSeed;
	static int32 NumOctaves;
	static float Persistence;
	static float Lacunarity;
	static float InitialScale;
	static int32 Size;

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetSeed(int32 NewSeed)
	{
		Seed = NewSeed;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetRandomizeSeed(bool bNewRandomizeSeed)
	{
		bRandomizeSeed = bNewRandomizeSeed;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetNumOctaves(int32 NewNumOctaves)
	{
		NumOctaves = NewNumOctaves;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetPersistence(float NewPersistence)
	{
		Persistence = NewPersistence;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetLacunarity(float NewLacunarity)
	{
		Lacunarity = NewLacunarity;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetInitialScale(float NewInitialScale)
	{
		InitialScale = NewInitialScale;
	}
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetSize(int32 NewSize)
	{
		Size = NewSize;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void GenerateLandscapeFromPNG(const FString& HeightmapPath);

	static TArray<uint16> ConvertFloatArrayToUint16(const TArray<float>& FloatData);
	static ALandscape* CallLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap);
	
	// Function to generate heightmap on CPU, generates a heightmap as a float array with the specified map size using Perlin noise.
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static TArray<float> GenerateHeightMapCPU(int32 MapSize);

	//Function to generate texture2D, using an array<float>
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static UTexture2D* CreateHeightMapTexture(const TArray<float>& HeightMapData, int32 Width, int32 Height);

	//Function to generate PNG grayscale from texture2D
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void SaveTextureToFile(UTexture2D* Texture, const FString& FilePath);
	
	//Main Function
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void CreateHeightMap(int32 MapSize);
};
