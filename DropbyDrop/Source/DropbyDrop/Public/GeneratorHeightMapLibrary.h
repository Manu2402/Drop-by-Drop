#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "Misc/FileHelper.h"
#include "Landscape.h"
#include "GeneratorHeightMapLibrary.generated.h"

UCLASS()
class DROPBYDROP_API UGeneratorHeightMapLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#pragma  region Param
	//Param HeightMap
		
	static TArray<float> HeightMap;
	static int32 Seed;					//"Seed used to initialize the random number generator."
	static bool bRandomizeSeed;			//Determines whether a random seed is generated for each heightmap. If false, the specified Seed value will be used.
	static int32 NumOctaves;			//The number of octaves used in Perlin noise generation. More octaves result in more detailed heightmaps.
	static float Persistence;			//Controls the amplitude of successive octaves. Lower values create smoother heightmaps.
	static float Lacunarity;			//Controls the frequency of successive octaves. Higher values create more detailed, higher frequency features.
	static float InitialScale;			//The initial scale applied to the Perlin noise calculation. Controls the overall size of features in the heightmap.
	static int32 Size;					//Size of the PNG
	static float MaxHeightDifference;	//The max difference of the range [0,1]
	
	//Param LandScape
	static ALandscape* StaticLandscape;
	static int32 WorldPartitionGridSize;
	static float Kilometers;
	static bool bDestroyLastLandscape;
#pragma endregion
	
public:
#pragma region Get/Set Param
	UFUNCTION(BlueprintCallable, Category ="HeightMap")
	static void SetNewStaticLandscape(ALandscape* NewStaticLandscape)
	{
		StaticLandscape = NewStaticLandscape;
	}
	
	UFUNCTION(BlueprintCallable,Category ="HeightMap")
	static void SetWorldPartitionGridSize(const int32 NewWorldPartitionGridSize)
	{
		WorldPartitionGridSize = NewWorldPartitionGridSize;
	}
	
	UFUNCTION(BlueprintCallable,Category ="HeightMap")
	static void SetKilometers(const float NewKilometers)
	{
		Kilometers = NewKilometers;
	}
	
	UFUNCTION(BlueprintCallable, Category="Heightmap")
	static void SetbDestroyLastLandscape(const bool NewbDestroyLastLandscape)
	{
		bDestroyLastLandscape = NewbDestroyLastLandscape;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetSeed(const int32 NewSeed)
	{
		Seed = NewSeed;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetRandomizeSeed(const bool bNewRandomizeSeed)
	{
		bRandomizeSeed = bNewRandomizeSeed;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetNumOctaves(const int32 NewNumOctaves)
	{
		NumOctaves = NewNumOctaves;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetPersistence(const float NewPersistence)
	{
		Persistence = NewPersistence;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetLacunarity(const float NewLacunarity)
	{
		Lacunarity = NewLacunarity;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetInitialScale(const float NewInitialScale)
	{
		InitialScale = NewInitialScale;
	}
	
	
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static int32 GetMapSize()
	{
		return Size;
	}
	
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetMaxHeightDifference(const float NewMaxHeightDifference) //[0,1]
	{
		MaxHeightDifference = NewMaxHeightDifference;
	}
#pragma endregion

#pragma region Erosion
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void GenerateErosion();
	
	UFUNCTION(BlueprintCallable, Category ="Heightmap")
	static void ErodeLandscapeProxy(ALandscapeProxy* LandscapeProxy);
#pragma endregion

#pragma region Landscape
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void GenerateLandscapeFromPNG(const FString& HeightmapPath);
	
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void SplitLandscapeIntoProxies();
	
	static bool SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX, int32& MaxY);
	
	static ALandscape* GenerateLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap);
	

#pragma endregion

#pragma region Heightmap
	// Function to generate heightmap on CPU, generates a heightmap as a float array with the specified map size using Perlin noise.
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static TArray<float> GenerateHeightMapCPU(const int32 MapSize);
	
	//Main Function
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void CreateHeightMap(const int32 MapSize);
	
	//Function to generate texture2D, using an array<float>
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static UTexture2D* CreateHeightMapTexture(const TArray<float>& HeightMapData,const int32 Width,const int32 Height);
#pragma endregion

#pragma region Utilities
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void SaveTextureToFile(UTexture2D* Texture, const FString& FilePath);
	
	static TArray<uint16> ConvertFloatArrayToUint16(const TArray<float>& FloatData);
private:
	static FTransform GetNewTransform();
	static void DestroyLastLandscape();
#pragma endregion
};
