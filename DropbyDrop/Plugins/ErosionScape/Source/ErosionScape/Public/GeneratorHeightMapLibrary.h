#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/Texture2D.h"
#include "ErosionScape.h"
#include "Misc/FileHelper.h"
#include "Landscape.h"
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

private:
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

	//Param External HeightMap
	static float ScalingX;
	static float ScalingY;
	static float ScalingZ;
	static bool bIsExternalHeightMap;
	//Param LandScape
	static ALandscape* StaticLandscape;
	static int32 WorldPartitionGridSize;
	static int32 Kilometers;
	static bool bKilometers;
	static bool bDestroyLastLandscape;
#pragma endregion

	static UDataTable* ErosionTemplatesDataTable;

public:
#pragma region Get/Set Param
	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void SetNewStaticLandscape(ALandscape* NewStaticLandscape)
	{
		StaticLandscape = NewStaticLandscape;
	}

	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void SetWorldPartitionGridSize(const int32 NewWorldPartitionGridSize)
	{
		WorldPartitionGridSize = NewWorldPartitionGridSize;
	}
	static int32 GetWorldPartitionGridSize()
	{
		return WorldPartitionGridSize;
	}

	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void SetKilometers(const int32 NewKilometers)
	{
		Kilometers = NewKilometers;
	}
	static int32 GetKilometers()
	{
		return Kilometers;
	}

	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	static void SetbKilometers(const bool NewbKilometers)
	{
		bKilometers = NewbKilometers;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetbDestroyLastLandscape(const bool NewbDestroyLastLandscape)
	{
		bDestroyLastLandscape = NewbDestroyLastLandscape;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetSeed(int32 NewSeed)
	{
		Seed = NewSeed;
		UE_LOG(LogTemp, Log, TEXT("Seed: %d"), Seed);
	}
	static int32 GetSeed()
	{
		return Seed;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetRandomizeSeed(const bool bNewRandomizeSeed)
	{
		bRandomizeSeed = bNewRandomizeSeed;
	}
	static bool GetRandomizeSeed()
	{
		return bRandomizeSeed;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetNumOctaves(const int32 NewNumOctaves)
	{
		NumOctaves = NewNumOctaves;
	}
	static int32 GetOctaves()
	{
		return NumOctaves;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetPersistence(const float NewPersistence)
	{
		Persistence = NewPersistence;
	}
	static float GetPersistence()
	{
		return Persistence;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetLacunarity(const float NewLacunarity)
	{
		Lacunarity = NewLacunarity;
	}
	static float GetLacunarity()
	{
		return Lacunarity;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetInitialScale(const float NewInitialScale)
	{
		InitialScale = NewInitialScale;
	}
	static float GetInitialScale()
	{
		return InitialScale;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetSize(const int32 NewSize)
	{
		Size = NewSize;
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
	static float GetMaxHeightDifference()
	{
		return MaxHeightDifference;
	}

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetScalingX(const float NewScalingX)
	{
		ScalingX = NewScalingX;
	}
	static float GetScalingX()
	{
		return ScalingX;
	}
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetScalingY(const float NewScalingY)
	{
		ScalingY = NewScalingY;
	}
	static float GetScalingY()
	{
		return ScalingY;
	}
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void SetScalingZ(const float NewScalingZ)
	{
		ScalingZ = NewScalingZ;
	}
	static float GetScalingZ()
	{
		return ScalingZ;
	}
#pragma endregion

#pragma region Erosion
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void GenerateErosion();

	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void ErodeLandscapeProxy(ALandscapeProxy* LandscapeProxy);

	// --------------------------------------------------------------------------------------------------

	static bool SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue,
		const uint8 WindDirectionValue,
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

	// --------------------------------------------------------------------------------------------------

#pragma endregion

#pragma region Landscape
	UFUNCTION(BlueprintCallable, Category = "Heightmap")
	static void GenerateLandscapeFromPNG(const FString& HeightmapPath);

	static void CreateLandscapeFromOtherHeightMap(const FString& FilePath);

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
	static UTexture2D* CreateHeightMapTexture(const TArray<float>& HeightMapData, const int32 Width, const int32 Height);

	static void LoadHeightmapFromPNG(const FString& FilePath, TArray<uint16>& OutHeightmap, TArray<float>& OutNormalizedHeightmap);
	static TArray<uint16> ResizeHeightmapBilinear( const TArray<uint16>& InputHeightmap, int32 SrcWidth, int32 SrcHeight, int32 TargetWidth, int32 TargetHeight);
#pragma endregion

#pragma region Utilitie

	static TArray<uint16> ConvertFloatArrayToUint16(const TArray<float>& FloatData);
	static bool SaveToAsset(UTexture2D* Texture, const FString& AssetName);

private:
	static FTransform GetNewTransform(bool bExternalHeightmap);
	static void DestroyLastLandscape();
#pragma endregion
};