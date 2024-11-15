#include "GeneratorHeightMapLibrary.h"

#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "ImageUtils.h"
#include "LandscapeProxy.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ErosionLibrary.h"
#include "LandscapeImportHelper.h"
#include "LandscapeSubsystem.h"
#include "Misc/ScopedSlowTask.h"
#include "WorldPartition/WorldPartitionSubsystem.h"

#pragma region InitParam
//Heightmap Param
int32 UGeneratorHeightMapLibrary::Seed = -4314;
bool UGeneratorHeightMapLibrary::bRandomizeSeed = false;
int32 UGeneratorHeightMapLibrary::NumOctaves = 8;
float UGeneratorHeightMapLibrary::Persistence = 0.45f;
float UGeneratorHeightMapLibrary::Lacunarity = 2.f;
float UGeneratorHeightMapLibrary::InitialScale = 1.8f;
int32 UGeneratorHeightMapLibrary::Size = 505;
float UGeneratorHeightMapLibrary::MaxHeightDifference = 1;
TArray<float> UGeneratorHeightMapLibrary::HeightMap;

//LandScapeParam
ALandscape* UGeneratorHeightMapLibrary::StaticLandscape = nullptr;
int32 UGeneratorHeightMapLibrary::Kilometers = 1;
bool UGeneratorHeightMapLibrary::bKilometers = false;
int32 UGeneratorHeightMapLibrary::WorldPartitionGridSize = 4;
bool UGeneratorHeightMapLibrary::bDestroyLastLandscape = true;

UDataTable* UGeneratorHeightMapLibrary::ErosionTemplatesDataTable = nullptr;
#pragma endregion

#pragma region Erosion
void UGeneratorHeightMapLibrary::GenerateErosion()
{
	UErosionLibrary::SetHeights(HeightMap);

	UErosionLibrary::ErosionHandler(Size);

	TArray<uint16> ErodedHeightmapU16 = ConvertFloatArrayToUint16(UErosionLibrary::GetHeights());

	// Generate new landscape.
	const FTransform LandscapeTransform = GetNewTransform();

	DestroyLastLandscape();
	StaticLandscape = GenerateLandscape(LandscapeTransform, ErodedHeightmapU16);
	if (StaticLandscape)
	{
		UE_LOG(LogTemp, Log, TEXT("Landscape created successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create landscape."));
	}
}

void UGeneratorHeightMapLibrary::ErodeLandscapeProxy(ALandscapeProxy* LandscapeProxy)
{
	TArray<uint16> HeightmapData;

	if (!LandscapeProxy)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid LandscapeProxy."));
		return;
	}

	TArray<ULandscapeComponent*> LandscapeComponents;
	LandscapeProxy->GetComponents<ULandscapeComponent>(LandscapeComponents);

	//TODO: Finish the method.
}

void UGeneratorHeightMapLibrary::SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue,
													 const float InertiaValue, const int32 CapacityValue,
													 const float MinSlopeValue, const float DepositionSpeedValue,
													 const float ErosionSpeedValue, const int32 GravityValue,
													 const float EvaporationValue, const int32 MaxPathValue,
													 const int32 ErosionRadiusValue)
{
	UE_LOG(LogTemp, Warning, TEXT("%s, %d, %f, %d, %f, %f, %f, %d, %f, %d, %d"), *TemplateName,
		ErosionCyclesValue, InertiaValue, CapacityValue, MinSlopeValue, DepositionSpeedValue, ErosionSpeedValue,
		GravityValue, EvaporationValue, MaxPathValue, ErosionRadiusValue)

	// Row fields.
	FErosionTemplateRow ErosionTemplateRow;
	ErosionTemplateRow.ErosionCyclesField = ErosionCyclesValue;
	ErosionTemplateRow.InertiaField = InertiaValue;
	ErosionTemplateRow.CapacityField = CapacityValue;
	ErosionTemplateRow.MinSlopeField = MinSlopeValue;
	ErosionTemplateRow.DepositionSpeedField = DepositionSpeedValue;
	ErosionTemplateRow.ErosionSpeedField = ErosionSpeedValue;
	ErosionTemplateRow.GravityField = GravityValue;
	ErosionTemplateRow.EvaporationField = EvaporationValue;
	ErosionTemplateRow.MaxPathField = MaxPathValue;
	ErosionTemplateRow.ErosionRadiusField = ErosionRadiusValue;

	ErosionTemplatesDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Custom/ErosionTemplates/DT_ErosionTemplate.DT_ErosionTemplate"));

	if(!ErosionTemplatesDataTable)
	{
		return;
	}
	
	ErosionTemplatesDataTable->AddRow(FName(TemplateName), ErosionTemplateRow);
}
#pragma endregion

#pragma region HeightMap
void UGeneratorHeightMapLibrary::CreateHeightMap(const int32 MapSize)
{
	HeightMap = GenerateHeightMapCPU(MapSize);

	UErosionLibrary::SetHeights(HeightMap);

	UTexture2D* Texture = CreateHeightMapTexture(HeightMap, MapSize, MapSize);
	const FString FilePath = FPaths::ProjectDir() / TEXT("Saved/HeightMap/Heightmap.png");
	SaveTextureToFile(Texture, FilePath);
}

TArray<float> UGeneratorHeightMapLibrary::GenerateHeightMapCPU(const int32 MapSize)
{
	TArray<float> HeightMapValues;
	HeightMapValues.SetNum(MapSize * MapSize);
	
	Seed = bRandomizeSeed ? FMath::RandRange(-10000, 10000) : Seed;
	const FRandomStream RandomStream(Seed);
	
	TArray<FVector2D> Offsets;
	Offsets.SetNum(NumOctaves);

	for (int32 i = 0; i < NumOctaves; i++)
	{
		Offsets[i] = FVector2D(RandomStream.FRandRange(-1000.0f, 1000.0f), RandomStream.FRandRange(-1000.0f, 1000.0f));
	}
	
	float MinValue = TNumericLimits<float>::Max();
	float MaxValue = TNumericLimits<float>::Min();
	
	for (int32 y = 0; y < MapSize; y++)
	{
		for (int32 x = 0; x < MapSize; x++)
		{
			float NoiseValue = 0.0f;
			float Scale = InitialScale;
			float Weight = 1.0f;
			
			for (int32 i = 0; i < NumOctaves; i++)
			{
				FVector2D P = Offsets[i] + FVector2D(x, y) / static_cast<float>(MapSize) * Scale;
				//Create a vector(x,y) contains the current position of the pixel in the map end normalized the vector {0,1}
				NoiseValue += FMath::PerlinNoise2D(P) * Weight; //Value of Perlin Noise in position P
				Weight *= Persistence; //Multiply the weight with the current with of octave
				Scale *= Lacunarity; //Scale the position P with the frequency of the octave
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

UTexture2D* UGeneratorHeightMapLibrary::CreateHeightMapTexture(const TArray<float>& HeightMapData,const int32 Width,
                                                               const int32 Height)
{
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8);
	Texture->SRGB = false;
	Texture->MipGenSettings = TMGS_NoMipmaps; //No multiply mipMaps
	
	FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
	MipMap.SizeX = Width;
	MipMap.SizeY = Height;

	// Locks the mipmap data for read and write access, returning a raw byte pointer.
	uint8* MipData = static_cast<uint8*>(MipMap.BulkData.Lock(LOCK_READ_WRITE));
	// Reinterprets the raw byte data as an array of colors, allowing direct pixel manipulation.
	FColor* DestPtr = reinterpret_cast<FColor*>(MipData);
	
	for (int32 i = 0; i < HeightMapData.Num(); ++i) 
	{
		const float NormalizedValue = HeightMapData[i];
		const uint8 GrayValue = FMath::Clamp(static_cast<uint8>(NormalizedValue * 255.0f), 0, 255);
		// Convert the normalized value to a grayscale value between 0 and 255
		DestPtr[i] = FColor(GrayValue, GrayValue, GrayValue, 255);
	}
	//ESSENTIAL! Unlock the mipMap Data
	MipMap.BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}
#pragma endregion

#pragma region Landscape
void UGeneratorHeightMapLibrary::GenerateLandscapeFromPNG(const FString& HeightmapPath)
{
	if(bRandomizeSeed)
	{
		HeightMap = GenerateHeightMapCPU(Size);
	}
	
	const UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get editor world"));
		return;
	}
	
	TArray<uint16> HeightData = ConvertFloatArrayToUint16(HeightMap);

	const FTransform LandscapeTransform = GetNewTransform();

	DestroyLastLandscape();
	StaticLandscape = GenerateLandscape(LandscapeTransform, HeightData);

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
	if (!StaticLandscape) return;

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

	//After fixing the WorldPartition that part of code can be delated, for now leave that part for a future fix.
	StaticLandscape->PostEditChange();
	if (GEditor && GEditor->GetEditorWorldContext().World())
	{
		const UWorld* World = GEditor->GetEditorWorldContext().World();
		if (UWorldPartitionSubsystem* WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>())
		{
			WorldPartitionSubsystem->UpdateStreamingState();
			GEditor->RedrawAllViewports();
		}
	}
}

ALandscape* UGeneratorHeightMapLibrary::GenerateLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap)
{
	int32 SubSectionSizeQuads;
	int32 NumSubsections;
	int32 MaxX;
	int32 MaxY;

	if (!SetLandscapeSizeParam(SubSectionSizeQuads, NumSubsections, MaxX, MaxY)) return nullptr;
	
	TMap<FGuid, TArray<uint16>> HeightDataPerLayer;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;
	TArray<FLandscapeImportLayerInfo> MaterialImportLayers; //Empty for the moment TODO: Create MaterialLayers
	MaterialImportLayers.Reserve(0);
	MaterialLayerDataPerLayer.Add(FGuid(), MoveTemp(MaterialImportLayers));
	
	if (Heightmap.Num() == MaxX * MaxY)
	{
		HeightDataPerLayer.Add(FGuid(), MoveTemp(Heightmap));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Heightmap dimensions do not match the generated landscape: %d != %d x %d"),
		       Heightmap.Num(), MaxX, MaxY);
		return nullptr;
	}
	
	UWorld* World;
	{
		const FWorldContext& EditorWorldContext = GEditor->GetEditorWorldContext();
		World = EditorWorldContext.World();
	}
	ALandscape* Landscape = World->SpawnActor<ALandscape>();
	Landscape->bCanHaveLayersContent = true;
	Landscape->LandscapeMaterial = nullptr;
	// Assign an appropriate material if needed
	Landscape->SetActorTransform(LandscapeTransform);

	UE_LOG(LogTemp, Error, TEXT("NumSubSections: %d; SubSectionSizeQuads:%d; MaxX,Y: %d;"), NumSubsections,
	       SubSectionSizeQuads, MaxY);

	Landscape->Import(
		FGuid::NewGuid(),
		0, 0,
		MaxX - 1, MaxY - 1,
		NumSubsections,
		SubSectionSizeQuads,
		HeightDataPerLayer,
		nullptr,
		MaterialLayerDataPerLayer,
		ELandscapeImportAlphamapType::Additive);
	
	Landscape->PostEditChange();
	return Landscape;
}

bool UGeneratorHeightMapLibrary::SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX, int32& MaxY)
{
	const int32 HeightmapSize = Size;
	FIntPoint NewLandscapeComponentCount;
	FLandscapeImportHelper::ChooseBestComponentSizeForImport(HeightmapSize, HeightmapSize, SubSectionSizeQuads, NumSubsections,
															 NewLandscapeComponentCount);

	const int32 ComponentCountX = NewLandscapeComponentCount.X;
	const int32 ComponentCountY = NewLandscapeComponentCount.Y;
	const int32 QuadsPerComponent = NumSubsections * SubSectionSizeQuads;
	MaxX = ComponentCountX * QuadsPerComponent + 1;
	MaxY = ComponentCountY * QuadsPerComponent + 1;

	if (MaxX != HeightmapSize || MaxY != HeightmapSize)
	{
		UE_LOG(LogTemp, Error, TEXT("Dimensioni calcolate (%d x %d) non corrispondono alla dimensione dell'heightmap (%d x %d)."))
		return false;
	}
	
	return true;
}
#pragma endregion

#pragma region Utilities
FTransform UGeneratorHeightMapLibrary::GetNewTransform()
{
	FVector NewScale;
	if (bKilometers)
	{
		const FVector CurrentScale = FVector(100, 100, 100);
		const float CurrentSizeInUnits = Size * CurrentScale.X;
		const float DesiredSizeInUnits = Kilometers * 1000.0f * 100.0f;
		const float ScaleFactor = DesiredSizeInUnits / CurrentSizeInUnits;
		NewScale = CurrentScale * ScaleFactor;
	}
	else
	{
		NewScale = FVector(100, 100, 100);
	}

	const FTransform LandscapeTransform = FTransform(FQuat(FRotator::ZeroRotator), FVector(-100800, -100800, 17200), NewScale);
	return LandscapeTransform;
}

void UGeneratorHeightMapLibrary::DestroyLastLandscape()
{
	if(bDestroyLastLandscape)
	{
		if(StaticLandscape != nullptr)
		{
			StaticLandscape->Destroy();
		}
	}
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


	FlushRenderingCommands();
	
	TArray<FColor> Bitmap;
	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!RenderTargetResource)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get render target resource"));
		return;
	}
	RenderTargetResource->ReadPixels(Bitmap);

	// Check if Bitmap has the expected number of pixels
	const int32 ExpectedPixelCount = Texture->GetSizeX() * Texture->GetSizeY();
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
		const uint8 GrayValue = Pixel.R; // Assuming R, G, and B are equal
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
#pragma endregion