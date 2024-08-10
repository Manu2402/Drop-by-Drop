// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "HeightMapGeneratorClass.generated.h"

UCLASS()
class DROPBYDROP_API AHeightMapGeneratorClass : public AActor
{
	GENERATED_BODY()
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Heightmap Generation", meta = (Tooltip = "Seed used to initialize the random number generator.", DisplayName = "Random Seed"))
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

	
	// Function to generate heightmap on CPU, generates a heightmap as a float array with the specified map size using Perlin noise.
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	TArray<float> GenerateHeightMapCPU(int32 MapSize);

	//Function to generate texture2D, using an array<float>
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	UTexture2D* CreateHeightMapTexture(const TArray<float>& HeightMapData, int32 Width, int32 Height);

	//Function to generate PNG grayscale from texture2D
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	void SaveTextureToFile(UTexture2D* Texture, const FString& FilePath);
	
	//Main Function
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	void CreateHeightMap(int32 MapSize);
};
