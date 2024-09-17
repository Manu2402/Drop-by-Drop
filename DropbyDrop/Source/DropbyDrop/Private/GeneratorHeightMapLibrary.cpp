#include "GeneratorHeightMapLibrary.h"

#include "Editor.h"
#include "EngineUtils.h"
#include "Editor/EditorEngine.h"
#include "ImageUtils.h"
#include "LandscapeProxy.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ErosionLibrary.h"
#include "LandscapeImportHelper.h"
#include "LandscapeStreamingProxy.h"
#include "LandscapeSubsystem.h"
#include "Misc/ScopedSlowTask.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

int32 UGeneratorHeightMapLibrary::Seed = -4314;
bool UGeneratorHeightMapLibrary::bRandomizeSeed = false;
int32 UGeneratorHeightMapLibrary::NumOctaves = 8;
float UGeneratorHeightMapLibrary::Persistence = 0.45f;
float UGeneratorHeightMapLibrary::Lacunarity = 2.f;
float UGeneratorHeightMapLibrary::InitialScale = 1.8f;
int32 UGeneratorHeightMapLibrary::Size = 505;
float UGeneratorHeightMapLibrary::MaxHeightDifference = 1;
ALandscape* UGeneratorHeightMapLibrary::StaticLandscape = nullptr;
//LandScapeParam
//int32 UGeneratorHeightMapLibrary::SectionSize = 63;
int32 UGeneratorHeightMapLibrary::NumSubsections = 1;
int32 UGeneratorHeightMapLibrary::Kilometers = 1;
bool UGeneratorHeightMapLibrary::bKilometers = false;
int32 UGeneratorHeightMapLibrary::WorldPartitionGridSize = 4;

TArray<float> UGeneratorHeightMapLibrary::HeightMap;

void UGeneratorHeightMapLibrary::GenerateErosion()
{
	UErosionLibrary::SetHeights(HeightMap);

	UErosionLibrary::ErosionHandler(Size);

	TArray<uint16> ErodedHeightmapU16 = ConvertFloatArrayToUint16(UErosionLibrary::GetHeights());

	// Generate new landscape.
	const FTransform LandscapeTransform = GetNewTransform();

	CallLandscape(LandscapeTransform, ErodedHeightmapU16);
}

void UGeneratorHeightMapLibrary::ErodeLandscapeProxy(ALandscapeProxy* LandscapeProxy)
{
	TArray<uint16> HeightmapData;

	if (!LandscapeProxy)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid LandscapeProxy."));
		return;
	}

	// Get all landscape components
	TArray<ULandscapeComponent*> LandscapeComponents;
	LandscapeProxy->GetComponents<ULandscapeComponent>(LandscapeComponents);

	//DA FINIREEE!!!
	//TODO: Create voice in menu for call this method!

}

void UGeneratorHeightMapLibrary::GenerateLandscapeFromPNG(const FString& HeightmapPath)
{
	//Get World
	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get editor world"));
		return;
	}

	//TArray<uint16> HeightData = ConvertFloatArrayToUint16(UErosionLibrary::GetHeights());
	TArray<uint16> HeightData = ConvertFloatArrayToUint16(HeightMap);

	FTransform LandscapeTransform = GetNewTransform();

	//Create HeightMap

	StaticLandscape = CallLandscape(LandscapeTransform, HeightData);

	if (StaticLandscape)
	{
		UE_LOG(LogTemp, Log, TEXT("Landscape created successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create landscape."));
	}
}

void UGeneratorHeightMapLibrary::SplitLandscapeIntoProxies()
{
	if(!StaticLandscape) return;
	
	// Register all landscape components
	ULandscapeInfo* LandscapeInfo = StaticLandscape->GetLandscapeInfo();
	LandscapeInfo->UpdateLayerInfoMap(StaticLandscape);
	StaticLandscape->RegisterAllComponents();

		FScopedSlowTask SlowTask(100, FText::FromString("Split Landscape"));
		SlowTask.MakeDialog(true); 
		if (ULandscapeSubsystem* LandscapeSubsystem = GEditor->GetEditorWorldContext().World()->GetSubsystem<
			ULandscapeSubsystem>())
		{
			LandscapeSubsystem->ChangeGridSize(LandscapeInfo, WorldPartitionGridSize);
		}
	
		SlowTask.EnterProgressFrame(100);
	
	// Update the landscape after modifications
	StaticLandscape->PostEditChange();
	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (UWorldPartitionSubsystem* WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>())
		{
			// Force update of World Partition
			WorldPartitionSubsystem->UpdateStreamingState();

			// Optionally, refresh all editor views
			GEditor->RedrawAllViewports();
		}
	}
}


FTransform UGeneratorHeightMapLibrary::GetNewTransform()
{
	FVector NewScale;
	if (bKilometers)
	{
		FVector CurrentScale = FVector(100, 100, 100);
		float CurrentSizeInUnits = Size * CurrentScale.X;
		float DesiredSizeInUnits = Kilometers * 1000.0f * 100.0f;
		float ScaleFactor = DesiredSizeInUnits / CurrentSizeInUnits;
		NewScale = CurrentScale * ScaleFactor;
	}
	else
	{
		NewScale = FVector(100, 100, 100);
	}

	FTransform LandscapeTransform =
		FTransform(FQuat(FRotator::ZeroRotator), FVector(-100800, -100800, 17200), NewScale);
	return LandscapeTransform;
}

TArray<uint16> UGeneratorHeightMapLibrary::ConvertFloatArrayToUint16(const TArray<float>& FloatData)
{
	TArray<uint16> Uint16Data;
	Uint16Data.AddUninitialized(FloatData.Num());

	for (int32 i = 0; i < FloatData.Num(); i++)
	{
		//Scale value from 0-1 to 0 - 65535
		Uint16Data[i] = FMath::Clamp<uint16>(FMath::RoundToInt(FloatData[i] * 65535.0f), 0, 65535);
	}

	return Uint16Data;
}

ALandscape* UGeneratorHeightMapLibrary::CallLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap)
{
	int32 HeightmapSize = Size;
	int32 SubSectionSizeQuads;
	FIntPoint NewLandscape_ComponentCount;
	FLandscapeImportHelper::ChooseBestComponentSizeForImport(Size, Size, SubSectionSizeQuads, NumSubsections,
	                                                         NewLandscape_ComponentCount);

	int32 ComponentCountX = NewLandscape_ComponentCount.X;
	int32 ComponentCountY = NewLandscape_ComponentCount.Y;
	const int32 QuadsPerComponent = NumSubsections * SubSectionSizeQuads;
	int32 MaxX = ComponentCountX * QuadsPerComponent + 1;
	int32 MaxY = ComponentCountY * QuadsPerComponent + 1;

	if (MaxX != HeightmapSize || MaxY != HeightmapSize)
	{
		UE_LOG(LogTemp, Error,
		       TEXT("Dimensioni calcolate (%d x %d) non corrispondono alla dimensione dell'heightmap (%d x %d)."),
		       MaxX, MaxY, HeightmapSize, HeightmapSize);
		return nullptr;
	}

	// Prepare data for landscape import
	TMap<FGuid, TArray<uint16>> HeightDataPerLayer;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;
	TArray<FLandscapeImportLayerInfo> MaterialImportLayers; //Empty for the moment, TODO: Create MaterialLayers
	MaterialImportLayers.Reserve(0);
	MaterialLayerDataPerLayer.Add(FGuid(), MoveTemp(MaterialImportLayers));

	// Copy the provided heightmap data into the import data
	if (Heightmap.Num() == MaxX * MaxY)
	{
		HeightDataPerLayer.Add(FGuid(), MoveTemp(Heightmap));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Dimensioni dell'heightmap non corrispondono al landscape generato: %d != %d x %d"),
		       Heightmap.Num(), MaxX, MaxY);
		return nullptr;
	}

	// Get the world 
	UWorld* World = nullptr;
	{
		FWorldContext& EditorWorldContext = GEditor->GetEditorWorldContext();
		World = EditorWorldContext.World();
	}
	ALandscape* Landscape = World->SpawnActor<ALandscape>();
	Landscape->bCanHaveLayersContent = true;
	Landscape->LandscapeMaterial = nullptr;
	// Assign an appropriate material if needed

	Landscape->SetActorTransform(LandscapeTransform);
	// Import landscape data
	UE_LOG(LogTemp, Error, TEXT("NumSubSections: %d; SubSectionSizeQuads:%d; MaxX,Y: %d;"), NumSubsections,
	       SubSectionSizeQuads, MaxY);

	Landscape->Import(
		FGuid::NewGuid(),
		0, 0,
		MaxX - 1, MaxY - 1, // -1, MaxY-1,
		NumSubsections,
		SubSectionSizeQuads,
		HeightDataPerLayer,
		nullptr,
		MaterialLayerDataPerLayer,
		ELandscapeImportAlphamapType::Additive);

/*
	// Register all landscape components
	ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
	LandscapeInfo->UpdateLayerInfoMap(Landscape);
	Landscape->RegisterAllComponents();
	if (bSplitInProxies)
	{
		if (ULandscapeSubsystem* LandscapeSubsystem = GEditor->GetEditorWorldContext().World()->GetSubsystem<
			ULandscapeSubsystem>())
		{
			LandscapeSubsystem->ChangeGridSize(LandscapeInfo, WorldPartitionGridSize);
		}
	}*/
	// Update the landscape after modifications
	Landscape->PostEditChange();
	return Landscape;
}

TArray<float> UGeneratorHeightMapLibrary::GenerateHeightMapCPU(int32 MapSize)
{
	TArray<float> HeightMapValues;
	HeightMapValues.SetNum(MapSize * MapSize);

	// Seed handling
	Seed = bRandomizeSeed ? FMath::RandRange(-10000, 10000) : Seed;
	const FRandomStream RandomStream(Seed);

	// Allocate array for offsets
	TArray<FVector2D> Offsets;
	Offsets.SetNum(NumOctaves);

	// Generate random offsets for each octave, Perlin Noise
	for (int32 i = 0; i < NumOctaves; i++)
	{
		Offsets[i] = FVector2D(RandomStream.FRandRange(-1000.0f, 1000.0f), RandomStream.FRandRange(-1000.0f, 1000.0f));
	}

	// Track minimum and maximum noise values
	float MinValue = TNumericLimits<float>::Max();
	float MaxValue = TNumericLimits<float>::Min();

	// Loop through each pixel in the heightmap
	for (int32 y = 0; y < MapSize; y++)
	{
		for (int32 x = 0; x < MapSize; x++)
		{
			float NoiseValue = 0.0f;
			float Scale = InitialScale;
			float Weight = 1.0f;

			// Accumulate noise from each octave
			for (int32 i = 0; i < NumOctaves; i++)
			{
				FVector2D P = Offsets[i] + FVector2D(x, y) / static_cast<float>(MapSize) * Scale;
				//Create a vector(x,y) contains the current position of the pixel in the map end normalized the vector {0,1}
				NoiseValue += FMath::PerlinNoise2D(P) * Weight; //Value of Perlin Noise in position P
				Weight *= Persistence; //Muliply the weight with the current with of octave
				Scale *= Lacunarity; //Scale the position P with the frequence of the octave
			}

			HeightMapValues[y * MapSize + x] = NoiseValue;
			MinValue = FMath::Min(MinValue, NoiseValue);
			MaxValue = FMath::Max(MaxValue, NoiseValue);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Min noise value: %f, Max noise value: %f"), MinValue, MaxValue);

	// Normalize heightmap values (if necessary)
	if (MinValue != MaxValue)
	{
		for (int32 i = 0; i < HeightMapValues.Num(); i++)
		{
			HeightMapValues[i] = (HeightMapValues[i] - MinValue) / (MaxValue - MinValue);

			// Scale the heightmap based on MaxHeightDifference
			HeightMapValues[i] = HeightMapValues[i] * MaxHeightDifference;
		}
	}
	return HeightMapValues;
}

UTexture2D* UGeneratorHeightMapLibrary::CreateHeightMapTexture(const TArray<float>& HeightMapData, int32 Width,
                                                               int32 Height)
{
	//Create Empty Texture2D          CreateTransient -> Used to create texture in execute, the texture create in Transient can be modificated in execute.
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8/* */);
	Texture->SRGB = false;
	//Standard Red Green Blue, is false becouse the heightmap interpreted the grey color for height not like color.
	Texture->MipGenSettings = TMGS_NoMipmaps; //No multiply mipMaps

	//Ref to the first MipMap
	FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
	MipMap.SizeX = Width;
	MipMap.SizeY = Height;

	// Locks the mipmap data for read and write access, returning a raw byte pointer.
	uint8* MipData = static_cast<uint8*>(MipMap.BulkData.Lock(LOCK_READ_WRITE));
	// Reinterprets the raw byte data as an array of colors, allowing direct pixel manipulation.
	FColor* DestPtr = reinterpret_cast<FColor*>(MipData);

	// copy data for each element
	for (int32 i = 0; i < HeightMapData.Num(); ++i) // Iterate over each height value in the heightmap
	{
		float NormalizedValue = HeightMapData[i]; // Get the normalized height value
		uint8 GrayValue = FMath::Clamp(static_cast<uint8>(NormalizedValue * 255.0f), 0, 255);
		// Convert the normalized value to a grayscale value between 0 and 255
		DestPtr[i] = FColor(GrayValue, GrayValue, GrayValue, 255);
		// Assign the grayscale color to the corresponding pixel in the texture

		// Log the values for debugging
		//UE_LOG(LogTemp, Log, TEXT("HeightMapData[%d]: %f, Normalized: %f, GrayValue: %d"), i, HeightMapData[i], NormalizedValue, GrayValue);
	}
	//ESSENTIAL! Unlock the mipMap Data
	MipMap.BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

void UGeneratorHeightMapLibrary::SaveTextureToFile(UTexture2D* Texture, const FString& FilePath)
{
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Texture is null"));
		return;
	}
	Texture->UpdateResource();

	if (!Texture->GetResource())
	{
		UE_LOG(LogTemp, Error, TEXT("Texture resource is null after UpdateResource"));
		return;
	}

	// Create RenderTarget
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(UTextureRenderTarget2D::StaticClass());
	if (!RenderTarget)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create render target"));
		return;
	}
	RenderTarget->InitCustomFormat(Texture->GetSizeX(), Texture->GetSizeY(), PF_R8G8B8A8, false);
	RenderTarget->UpdateResourceImmediate(true);

	if (!RenderTarget->GetResource())
	{
		UE_LOG(LogTemp, Error, TEXT("RenderTarget resource is null after initialization"));
		return;
	}

	// copy data from texture to render data
	ENQUEUE_RENDER_COMMAND(CopyToRenderTarget)(
		[Texture, RenderTarget](FRHICommandListImmediate& RHICmdList)
		{
			if (!Texture->GetResource())
			{
				UE_LOG(LogTemp, Error, TEXT("Texture resource is null in render command"));
				return;
			}

			if (!RenderTarget->GetResource())
			{
				UE_LOG(LogTemp, Error, TEXT("RenderTarget resource is null in render command"));
				return;
			}
			FRHICopyTextureInfo CopyInfo;
			RHICmdList.CopyTexture(Texture->GetResource()->TextureRHI, RenderTarget->GetResource()->TextureRHI,
			                       CopyInfo);
		}
	);


	FlushRenderingCommands(); //Close render data

	// Export PNG
	TArray<FColor> Bitmap;
	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!RenderTargetResource)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get render target resource"));
		return;
	}
	RenderTargetResource->ReadPixels(Bitmap);

	// Check if Bitmap has the expected number of pixels
	int32 ExpectedPixelCount = Texture->GetSizeX() * Texture->GetSizeY();
	if (Bitmap.Num() != ExpectedPixelCount)
	{
		UE_LOG(LogTemp, Error, TEXT("Bitmap size mismatch. Expected: %d, Actual: %d"), ExpectedPixelCount,
		       Bitmap.Num());
		return;
	}
	// Ensure the bitmap is in grayscale
	for (int32 i = 0; i < Bitmap.Num(); ++i)
	{
		FColor& Pixel = Bitmap[i];
		uint8 GrayValue = Pixel.R; // Assuming R, G, and B are equal
		Pixel = FColor(GrayValue, GrayValue, GrayValue, 255); // Set RGB to GrayValue and Alpha to 255
	}
	// Compress and save the bitmap
	try
	{
		TArray64<uint8> CompressedBitmap;
		FImageUtils::PNGCompressImageArray(Texture->GetSizeX(), Texture->GetSizeY(), Bitmap, CompressedBitmap);

		if (FFileHelper::SaveArrayToFile(CompressedBitmap, *FilePath))
		{
			UE_LOG(LogTemp, Log, TEXT("Texture saved successfully to: %s"), *FilePath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save texture to: %s"), *FilePath);
		}
	}
	catch (const std::exception& e)
	{
		UE_LOG(LogTemp, Error, TEXT("Exception during compression or saving: %hc"), *e.what());
	}
}

void UGeneratorHeightMapLibrary::CreateHeightMap(int32 MapSize)
{
	HeightMap = GenerateHeightMapCPU(MapSize);

	UErosionLibrary::SetHeights(HeightMap);

	UTexture2D* Texture = CreateHeightMapTexture(HeightMap, MapSize, MapSize);
	FString FilePath = FPaths::ProjectDir() / TEXT("Saved/HeightMap/Heightmap.png");
	SaveTextureToFile(Texture, FilePath);
}
