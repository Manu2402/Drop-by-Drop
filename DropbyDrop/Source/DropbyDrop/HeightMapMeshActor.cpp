// Fill out your copyright notice in the Description page of Project Settings.


#include "HeightMapMeshActor.h"
#include "ImageUtils.h"


AHeightMapMeshActor::AHeightMapMeshActor()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = Mesh;
}
void AHeightMapMeshActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHeightMapMeshActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHeightMapMeshActor::GenerateMeshFromHeightMap(const TArray<float>& HeightMap, int32 MapSize, float MaxHeight, float ScaleXY)
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	const TArray<FProcMeshTangent> Tangents;

	
	// avoid dynamic resizing
	Vertices.Reserve(MapSize * MapSize);
	Normals.Reserve(MapSize * MapSize);
	UVs.Reserve(MapSize * MapSize);
	Triangles.Reserve((MapSize - 1) * (MapSize - 1) * 6);

	// Generate vertices & UVs
	for (int32 y = 0; y < MapSize; y++)
	{
		for (int32 x = 0; x < MapSize; x++)
		{
			const float Height = HeightMap[y * MapSize + x] * MaxHeight;
			Vertices.Add(FVector(x * ScaleXY, y * ScaleXY, Height));
			UVs.Add(FVector2D(x / static_cast<float>(MapSize), y / static_cast<float>(MapSize)));
			//Normals.Add(FVector(0, 0, 1));  // Placeholder normals, to be recalculated
		}
	}

	// Generate triangles
	for (int32 y = 0; y < MapSize - 1; y++)
	{
		for (int32 x = 0; x < MapSize - 1; x++)
		{
			int32 Index0 = y * MapSize + x;
			int32 Index1 = (y + 1) * MapSize + x;
			int32 Index2 = y * MapSize + (x + 1);
			int32 Index3 = (y + 1) * MapSize + (x + 1);

			// First triangle
			Triangles.Add(Index0);
			Triangles.Add(Index1);
			Triangles.Add(Index2);

			// Second triangle
			Triangles.Add(Index1);
			Triangles.Add(Index3);
			Triangles.Add(Index2);
		}
	}

	Normals.SetNumZeroed(MapSize * MapSize); // Initialize normals to zero
	for (int32 y = 0; y < MapSize - 1; y++)
	{
		for (int32 x = 0; x < MapSize - 1; x++)
		{
			int32 Index0 = y * MapSize + x;
			int32 Index1 = (y + 1) * MapSize + x;
			int32 Index2 = y * MapSize + (x + 1);
			int32 Index3 = (y + 1) * MapSize + (x + 1);

			FVector Vertex0 = Vertices[Index0];
			FVector Vertex1 = Vertices[Index1];
			FVector Vertex2 = Vertices[Index2];
			FVector Vertex3 = Vertices[Index3];

			FVector Normal0 = FVector::CrossProduct(Vertex1 - Vertex0, Vertex2 - Vertex0).GetSafeNormal();
			FVector Normal1 = FVector::CrossProduct(Vertex3 - Vertex1, Vertex2 - Vertex1).GetSafeNormal();

			Normals[Index0] += Normal0;
			Normals[Index1] += Normal0 + Normal1;
			Normals[Index2] += Normal0 + Normal1;
			Normals[Index3] += Normal1;
		}
	}

	// Normalize normals
	for (int32 i = 0; i < Normals.Num(); i++)
	{
		Normals[i].Normalize();
	}

	// Create mesh section
	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, TArray<FLinearColor>(), Tangents, true);
}

void AHeightMapMeshActor::GenerateMeshFromPng(const FString& FilePath,  int32 MapSize, float MaxHeight, float ScaleXY)
{
	TArray<uint8> FileData;
if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
{
    UE_LOG(LogTemp, Error, TEXT("Failed to load file: %s"), *FilePath);
    return;
}

UTexture2D* LoadedTexture = FImageUtils::ImportBufferAsTexture2D(FileData);
if (!LoadedTexture)
{
    UE_LOG(LogTemp, Error, TEXT("Failed to import texture from file: %s"), *FilePath);
    return;
}

FTexture2DMipMap& MipMap = LoadedTexture->GetPlatformData()->Mips[0];
int32 Width = MipMap.SizeX;
int32 Height = MipMap.SizeY;

uint8* MipData = static_cast<uint8*>(MipMap.BulkData.Lock(LOCK_READ_ONLY));
FColor* SrcPtr = reinterpret_cast<FColor*>(MipData);

TArray<float> HeightMap;
HeightMap.SetNum(Width * Height);

float MinValue = FLT_MAX;
float MaxValue = FLT_MIN;

// Trova i valori minimi e massimi
for (int32 y = 0; y < Height; y++)
{
    for (int32 x = 0; x < Width; x++)
    {
        int32 PixelIndex = y * Width + x;
        FColor PixelColor = SrcPtr[PixelIndex];
        float GrayValue = static_cast<float>(PixelColor.R) / 255.0f;
        
        MinValue = FMath::Min(MinValue, GrayValue);
        MaxValue = FMath::Max(MaxValue, GrayValue);

        HeightMap[PixelIndex] = GrayValue;
    }
}

// Normalizza i valori tra 0 e 1 usando il lerp
for (int32 i = 0; i < HeightMap.Num(); i++)
{
    HeightMap[i] = (HeightMap[i] - MinValue) / (MaxValue - MinValue);
}

MipMap.BulkData.Unlock();

// Advanced Gaussian smoothing filter

TArray<float> SmoothedHeightMap;
SmoothedHeightMap.SetNum(Width * Height);

const int32 KernelSize = 5; // Adjust kernel size as needed
const float Kernel[KernelSize][KernelSize] = {
    {1,  4,  6,  4, 1},
    {4, 16, 24, 16, 4},
    {6, 24, 36, 24, 6},
    {4, 16, 24, 16, 4},
    {1,  4,  6,  4, 1}
};
const float KernelSum = 256.0f;

for (int32 y = KernelSize / 2; y < Height - KernelSize / 2; y++)
{
    for (int32 x = KernelSize / 2; x < Width - KernelSize / 2; x++)
    {
        float Sum = 0.0f;

        for (int32 ky = -KernelSize / 2; ky <= KernelSize / 2; ky++)
        {
            for (int32 kx = -KernelSize / 2; kx <= KernelSize / 2; kx++)
            {
                int32 PixelIndex = (y + ky) * Width + (x + kx);
                Sum += HeightMap[PixelIndex] * Kernel[ky + KernelSize / 2][kx + KernelSize / 2];
            }
        }

        int32 CenterIndex = y * Width + x;
        SmoothedHeightMap[CenterIndex] = Sum / KernelSum;
    }
}

// Clamp the height values to a reasonable range
for (float& HeightValue : SmoothedHeightMap)
{
    HeightValue = FMath::Clamp(HeightValue, 0.0f, MaxHeight);
}

GenerateMeshFromHeightMap(SmoothedHeightMap, MapSize, MaxHeight, ScaleXY);

}

