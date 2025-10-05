#include "GeneratorHeightMapLibrary.h"

#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "ImageUtils.h"
#include "LandscapeProxy.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "ErosionLibrary.h"
#include "ErosionScapeSettings.h"
#include "IDesktopPlatform.h"
#include "LandscapeImportHelper.h"
#include "LandscapeSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "UObject/SavePackage.h"
#include "GeneratorHeightMapLibrary.h"
#include "HAL/IConsoleManager.h"


#pragma region InitParam

float UGeneratorHeightMapLibrary::WindPreviewScale = 1.0f;
UDataTable* UGeneratorHeightMapLibrary::ErosionTemplatesDataTable = nullptr;
#pragma endregion

#pragma region Erosion
void UGeneratorHeightMapLibrary::GenerateErosion(const FExternalHeightMapSettings& ExternalSettings,
                                                 FLandscapeGenerationSettings& LandscapeSettings,
                                                 const FHeightMapGenerationSettings& HeightMapSettings,
                                                 int32 HeightMapSize)
{
	FScopedSlowTask SlowTask(100, FText::FromString("Erosion in progress..."));
	SlowTask.MakeDialog(true);

	FErosionContext ErosionContext;
	UErosionLibrary::SetHeights(ErosionContext, HeightMapSettings.HeightMap);
	UErosionLibrary::Erosion(ErosionContext, HeightMapSettings.Size);

	SlowTask.EnterProgressFrame(50, FText::FromString("Adapting into the landscape..."));

	TArray<uint16> ErodedHeightmapU16 = ConvertFloatArrayToUint16(UErosionLibrary::GetHeights(ErosionContext));

	// Generate new landscape.
	const FTransform LandscapeTransform = GetNewTransform(ExternalSettings, LandscapeSettings, HeightMapSize);

	if (LandscapeSettings.bDestroyLastLandscape && LandscapeSettings.TargetLandscape)
	{
		LandscapeSettings.TargetLandscape->Destroy();
	}

	LandscapeSettings.TargetLandscape = GenerateLandscape(LandscapeTransform, ErodedHeightmapU16);

	SlowTask.EnterProgressFrame(50);

	if (LandscapeSettings.TargetLandscape)
	{
		UE_LOG(LogTemp, Log, TEXT("Landscape created successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create landscape."));
	}
}


bool UGeneratorHeightMapLibrary::SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue,
                                                     const float InertiaValue, const int32 CapacityValue,
                                                     const float MinSlopeValue, const float DepositionSpeedValue,
                                                     const float ErosionSpeedValue, const int32 GravityValue,
                                                     const float EvaporationValue, const int32 MaxPathValue,
                                                     const int32 ErosionRadiusValue)
{
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

	ErosionTemplatesDataTable->AddRow(FName(TemplateName), ErosionTemplateRow);
	return SaveErosionTemplates();
}

FErosionTemplateRow* UGeneratorHeightMapLibrary::LoadErosionTemplate(const FString& RowName)
{
	FString ContextString = TEXT("DataTable Context");

	FErosionTemplateRow* RowData = ErosionTemplatesDataTable->FindRow<FErosionTemplateRow>(
		FName(RowName), ContextString);

	if (!RowData)
	{
		return nullptr;
	}

	return RowData;
}

UDataTable* UGeneratorHeightMapLibrary::GetErosionTemplates()
{
	return ErosionTemplatesDataTable;
}

void UGeneratorHeightMapLibrary::SetErosionTemplates(const TCHAR* DataTablePath)
{
	ErosionTemplatesDataTable = LoadObject<UDataTable>(nullptr, DataTablePath);
}

void UGeneratorHeightMapLibrary::LoadRowIntoErosionFields(const FErosionTemplateRow* TemplateDatas)
{
	UErosionLibrary::SetErosion(TemplateDatas->ErosionCyclesField);

	UErosionLibrary::SetWindDirection(static_cast<EWindDirection>(TemplateDatas->WindDirection));
	UErosionLibrary::SetInertia(TemplateDatas->InertiaField);
	UErosionLibrary::SetCapacity(TemplateDatas->CapacityField);
	UErosionLibrary::SetMinimalSlope(TemplateDatas->MinSlopeField);
	UErosionLibrary::SetDepositionSpeed(TemplateDatas->DepositionSpeedField);
	UErosionLibrary::SetErosionSpeed(TemplateDatas->ErosionSpeedField);
	UErosionLibrary::SetGravity(TemplateDatas->GravityField);
	UErosionLibrary::SetEvaporation(TemplateDatas->EvaporationField);
	UErosionLibrary::SetMaxPath(TemplateDatas->MaxPathField);
	UErosionLibrary::SetErosionRadius(TemplateDatas->ErosionRadiusField);
}

bool UGeneratorHeightMapLibrary::DeleteErosionTemplate(const FString& TemplateName)
{
	ErosionTemplatesDataTable->RemoveRow(FName(TemplateName));
	return SaveErosionTemplates();
}

bool UGeneratorHeightMapLibrary::SaveErosionTemplates()
{
	UPackage* ErostionTemplatesPackage = ErosionTemplatesDataTable->GetPackage();
	if (!ErostionTemplatesPackage)
	{
		return false;
	}

	ErostionTemplatesPackage->MarkPackageDirty();

	return GEditor->GetEditorSubsystem<UEditorAssetSubsystem>()->SaveLoadedAsset(ErosionTemplatesDataTable);
}
#pragma endregion

#pragma region HeightMap
bool UGeneratorHeightMapLibrary::CreateAndSaveHeightMap(FHeightMapGenerationSettings& Settings)
{
	Settings.HeightMap = CreateHeightMapArray(Settings);


	UTexture2D* Texture = CreateHeightMapTexture(Settings.HeightMap, Settings.Size, Settings.Size);

	//Saving Asset in -> /Game/SavedAssets/
	if (SaveToAsset(Texture, "TextureHeightMap"))
	{
		return true;
	}
	return false;
}

TArray<float> UGeneratorHeightMapLibrary::CreateHeightMapArray(const FHeightMapGenerationSettings& Settings)
{
	const int32 MapSize = Settings.Size;
	TArray<float> HeightMapValues;
	HeightMapValues.SetNum(MapSize * MapSize);

	//Seed
	const int32 ActualSeed = Settings.bRandomizeSeed ? FMath::RandRange(-10000, 10000) : Settings.Seed;
	const FRandomStream RandomStream(ActualSeed);

	//Random offset foreach octave
	TArray<FVector2D> Offsets;
	Offsets.SetNum(Settings.NumOctaves);

	for (int32 i = 0; i < Settings.NumOctaves; ++i)
	{
		Offsets[i] = FVector2D(
			RandomStream.FRandRange(-1000.0f, 1000.0f),
			RandomStream.FRandRange(-1000.0f, 1000.0f));
	}

	float MinValue = TNumericLimits<float>::Max();
	float MaxValue = TNumericLimits<float>::Lowest();

	//Generate Perlin Noise for each point (X,Y)
	for (int32 y = 0; y < MapSize; ++y)
	{
		for (int32 x = 0; x < MapSize; ++x)
		{
			float NoiseValue = 0.0f;
			float Scale = Settings.InitialScale;
			float Weight = 1.0f;

			for (int32 i = 0; i < Settings.NumOctaves; ++i)
			{
				FVector2D P = Offsets[i] + FVector2D(x, y) / static_cast<float>(MapSize) * Scale;
				NoiseValue += FMath::PerlinNoise2D(P) * Weight;

				Weight *= Settings.Persistence;
				Scale *= Settings.Lacunarity;
			}

			const int32 Index = y * MapSize + x;
			HeightMapValues[Index] = NoiseValue;
			MinValue = FMath::Min(MinValue, NoiseValue);
			MaxValue = FMath::Max(MaxValue, NoiseValue);
		}
	}

	//Normalizing in range [0, MaxHeightDifference]
	if (!FMath::IsNearlyEqual(MinValue, MaxValue))
	{
		for (float& Value : HeightMapValues)
		{
			Value = (Value - MinValue) / (MaxValue - MinValue);
			Value *= Settings.MaxHeightDifference;
		}
	}
	return HeightMapValues;
}

UTexture2D* UGeneratorHeightMapLibrary::CreateHeightMapTexture(const TArray<float>& HeightMapData, const int32 Width,
                                                               const int32 Height)
{
	UTexture2D* Texture = NewObject<UTexture2D>(GetTransientPackage(), UTexture2D::StaticClass(),
	                                            FName(*FString::Printf(TEXT("GeneratedTexture_%d"), FMath::Rand())),
	                                            RF_Public | RF_Standalone);

	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create texture!"));
		return nullptr;
	}

	//Create and set the PlatformData
	FTexturePlatformData* PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = Width;
	PlatformData->SizeY = Height;
	PlatformData->PixelFormat = PF_B8G8R8A8; // Only PF_B8G8R8A8 format is supported

	//Create MipMap
	FTexture2DMipMap* MipMap = new FTexture2DMipMap();
	MipMap->SizeX = Width;
	MipMap->SizeY = Height;
	PlatformData->Mips.Add(MipMap);

	//Allocate memory for create texture (without using Python this part of the code is essential for write a texture).
	MipMap->BulkData.Lock(LOCK_READ_WRITE);
	uint8* MipData = MipMap->BulkData.Realloc(Width * Height * 4);
	FMemory::Memzero(MipData, Width * Height * 4); //Clear initial data

	//Writing pixel for the texture. 
	for (int32 i = 0; i < HeightMapData.Num(); ++i)
	{
		const uint8 GrayValue = FMath::Clamp(static_cast<uint8>(HeightMapData[i] * 255.0f), 0, 255);
		MipData[i * 4 + 0] = GrayValue; // R
		MipData[i * 4 + 1] = GrayValue; // G
		MipData[i * 4 + 2] = GrayValue; // B
		MipData[i * 4 + 3] = 255; // A
	}

	MipMap->BulkData.Unlock();

	//Set PlatformData to texture
	Texture->SetPlatformData(PlatformData);

	//Compressing and set MipMap
	Texture->SRGB = true;
	Texture->CompressionSettings = TC_Default;
	Texture->MipGenSettings = TMGS_FromTextureGroup;
	Texture->UpdateResource();

	return Texture;
}


void UGeneratorHeightMapLibrary::LoadHeightmapFromPNG(const FString& FilePath, TArray<uint16>& OutHeightmap,
                                                      TArray<float>& OutNormalizedHeightmap,
                                                      FExternalHeightMapSettings& Settings)
{
	OutHeightmap.Empty();
	OutNormalizedHeightmap.Empty();

	//Get Texture from filePath
	UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load PNG file: %s"), *FilePath);
		return;
	}

	//Get data from Texture
	FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
	const void* MipData = MipMap.BulkData.LockReadOnly();
	int32 Width = MipMap.SizeX;
	int32 Height = MipMap.SizeY;

	UE_LOG(LogTemp, Log, TEXT("Loaded PNG: Width=%d, Height=%d"), Width, Height);

	uint32 MinPixel = TNumericLimits<uint32>::Max();
	uint32 MaxPixel = TNumericLimits<uint32>::Min();

	OutHeightmap.SetNum(Width * Height);
	OutNormalizedHeightmap.SetNum(Width * Height);

	//Get format and data
	if (Texture->GetPixelFormat() == PF_R8G8B8A8) // RGBA
	{
		const uint8* PixelData = static_cast<const uint8*>(MipData);
		for (int32 i = 0; i < Width * Height; ++i)
		{
			uint8 RedValue = PixelData[i * 4];
			uint16 HeightValue = static_cast<uint16>(RedValue << 8); // Scale to 16 bit
			OutHeightmap[i] = HeightValue;
			MinPixel = FMath::Min<uint32>(MinPixel, HeightValue);
			MaxPixel = FMath::Max<uint32>(MaxPixel, HeightValue);
		}
	}
	else if (Texture->GetPixelFormat() == PF_G16) // Grayscale 16 bit
	{
		const uint16* PixelData = static_cast<const uint16*>(MipData);
		for (int32 i = 0; i < Width * Height; ++i)
		{
			OutHeightmap[i] = PixelData[i];
			MinPixel = FMath::Min<uint32>(MinPixel, OutHeightmap[i]);
			MaxPixel = FMath::Max<uint32>(MaxPixel, OutHeightmap[i]);
		}
	}
	else if (Texture->GetPixelFormat() == PF_R32_FLOAT) // Grayscale 32 bit float
	{
		const float* PixelData = static_cast<const float*>(MipData);
		for (int32 i = 0; i < Width * Height; ++i)
		{
			uint16 HeightValue = static_cast<uint16>(PixelData[i] * 65535.0f);
			OutHeightmap[i] = HeightValue;
			MinPixel = FMath::Min<uint32>(MinPixel, HeightValue);
			MaxPixel = FMath::Max<uint32>(MaxPixel, HeightValue);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Unsupported pixel format in PNG!"));
		MipMap.BulkData.Unlock();
		return;
	}

	MipMap.BulkData.Unlock();

	// Normalization float from 0 to 1
	for (int32 i = 0; i < OutHeightmap.Num(); ++i)
	{
		if (MaxPixel > MinPixel)
		{
			OutNormalizedHeightmap[i] = (OutHeightmap[i] - MinPixel) / static_cast<float>(MaxPixel - MinPixel);
		}
		else
		{
			OutNormalizedHeightmap[i] = 0.0f;
		}
	}
	Settings.bIsExternalHeightMap = true;
	UE_LOG(LogTemp, Log, TEXT("Heightmap Min: %u, Max: %u"), MinPixel, MaxPixel);
}

void CompareHeightmaps(const FString& RawFilePath, const TArray<uint16>& GeneratedHeightmap, int32 Width, int32 Height)
{
	//Get RAW data from Unreal
	TArray<uint8> RawData;
	if (!FFileHelper::LoadFileToArray(RawData, *RawFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load RAW file: %s"), *RawFilePath);
		return;
	}

	// let's make sure that the size is Width * Height * 2 (uint16 = 2 bytes)
	const uint64 ExpectedSize = static_cast<uint64>(Width) * static_cast<uint64>(Height) * sizeof(uint16);
	if (RawData.Num() != ExpectedSize)
	{
		UE_LOG(LogTemp, Error, TEXT("RAW file size does not match the expected size: %llu vs %llu"),
		       static_cast<uint64>(RawData.Num()), ExpectedSize);
		return;
	}

	//Convert TArray<uint8> to TArray<uint16>
	TArray<uint16> UnrealHeightmap;
	UnrealHeightmap.SetNum(Width * Height);
	FMemory::Memcpy(UnrealHeightmap.GetData(), RawData.GetData(), RawData.Num());

	//Compare the values
	for (int32 i = 0; i < FMath::Min(10, UnrealHeightmap.Num()); ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("RAW[%d]: %d, Generated[%d]: %d"),
		       i, UnrealHeightmap[i], i, GeneratedHeightmap[i]);
	}
}
#pragma endregion

#pragma region Landscape
void UGeneratorHeightMapLibrary::GenerateLandscapeFromPNG(const FString& HeightmapPath,
                                                          FHeightMapGenerationSettings& HeightmapSettings,
                                                          FExternalHeightMapSettings& ExternalSettings,
                                                          FLandscapeGenerationSettings& LandscapeSettings)
{
	ExternalSettings.bIsExternalHeightMap = false;
	if (HeightmapSettings.HeightMap.Num() == 0)
	{
		if (HeightmapSettings.bRandomizeSeed)
		{
			FHeightMapGenerationSettings TempSettings = HeightmapSettings;
			TempSettings.Seed = FMath::RandRange(-10000, 10000);
			HeightmapSettings.HeightMap = CreateHeightMapArray(TempSettings);
		}
		else
		{
			HeightmapSettings.HeightMap = CreateHeightMapArray(HeightmapSettings);
		}
	}

	const UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get editor world"));
		return;
	}

	//From float to uint16 (Unreal requisition)
	TArray<uint16> HeightData = ConvertFloatArrayToUint16(HeightmapSettings.HeightMap);

	const FTransform LandscapeTransform = GetNewTransform(ExternalSettings, LandscapeSettings, HeightmapSettings.Size);

	//If destroy last landscape
	if (LandscapeSettings.bDestroyLastLandscape && LandscapeSettings.TargetLandscape)
	{
		LandscapeSettings.TargetLandscape->Destroy();
	}
	LandscapeSettings.TargetLandscape = GenerateLandscape(LandscapeTransform, HeightData);

	if (LandscapeSettings.TargetLandscape)
	{
		UE_LOG(LogTemp, Log, TEXT("Landscape created successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create landscape."));
	}
}

void UGeneratorHeightMapLibrary::GenerateLandscapeAuto(
	FHeightMapGenerationSettings& HeightmapSettings,
	FExternalHeightMapSettings& ExternalSettings,
	FLandscapeGenerationSettings& LandscapeSettings)
{
	if (ExternalSettings.bIsExternalHeightMap && !ExternalSettings.LastPNGPath.IsEmpty())
	{
		CreateLandscapeFromOtherHeightMap(
			ExternalSettings.LastPNGPath,
			ExternalSettings,
			LandscapeSettings,
			HeightmapSettings
		);
		return;
	}

	const FString DummyPath;
	GenerateLandscapeFromPNG(
		DummyPath,
		HeightmapSettings,
		ExternalSettings,
		LandscapeSettings
	);
}


void UGeneratorHeightMapLibrary::CreateLandscapeFromOtherHeightMap(const FString& FilePath,
                                                                   FExternalHeightMapSettings& ExternalSettings,
                                                                   FLandscapeGenerationSettings& LandscapeSettings,
                                                                   FHeightMapGenerationSettings& HeightmapSettings)
{
	ExternalSettings.bIsExternalHeightMap = true;
	ExternalSettings.LastPNGPath = FilePath;

	//If destroy last landscape
	if (LandscapeSettings.bDestroyLastLandscape && LandscapeSettings.TargetLandscape)
	{
		LandscapeSettings.TargetLandscape->Destroy();
	}
	// Load HeightMap from PNG
	TArray<uint16> HeightmapInt16;
	LoadHeightmapFromPNG(FilePath, HeightmapInt16, HeightmapSettings.HeightMap, ExternalSettings);

	FString HeightMapPath = FPaths::ProjectDir() + TEXT("Saved/HeightMap/raw.r16");
	CompareHeightmaps(HeightMapPath, HeightmapInt16, 505, 505);
	if (HeightmapInt16.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load heightmap from PNG file: %s"), *FilePath);
		return;
	}
	const FTransform LandscapeTransform = GetNewTransform(ExternalSettings, LandscapeSettings, HeightmapSettings.Size);
	// Create Landscape using ExternalSetting for Transform (Avoid Kilometers)
	LandscapeSettings.TargetLandscape = GenerateLandscape(LandscapeTransform, HeightmapInt16);
	if (!LandscapeSettings.TargetLandscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to generate landscape from heightmap."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Landscape successfully created from PNG: %s"), *FilePath);
}

void UGeneratorHeightMapLibrary::SplitLandscapeIntoProxies(FLandscapeGenerationSettings& LandscapeSettings)
{
	if (!LandscapeSettings.TargetLandscape)
	{
		return;
	}

	ULandscapeInfo* LandscapeInfo = LandscapeSettings.TargetLandscape->GetLandscapeInfo();
	LandscapeInfo->UpdateLayerInfoMap(LandscapeSettings.TargetLandscape);

	LandscapeSettings.TargetLandscape->RegisterAllComponents();
	FScopedSlowTask SlowTask(100, FText::FromString("Split Landscape"));
	SlowTask.MakeDialog(true);
	if (ULandscapeSubsystem* LandscapeSubsystem = GEditor->GetEditorWorldContext().World()->GetSubsystem<
		ULandscapeSubsystem>())
	{
		LandscapeSubsystem->ChangeGridSize(LandscapeInfo, LandscapeSettings.WorldPartitionCellSize);
	}

	SlowTask.EnterProgressFrame(100);

	//After fixing the WorldPartition that part of code can be delated, for now leave that part for a future fix.
	LandscapeSettings.TargetLandscape->PostEditChange();
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

ALandscape* UGeneratorHeightMapLibrary::GenerateLandscape(const FTransform& LandscapeTransform,
                                                          TArray<uint16>& Heightmap)
{
	int32 SubSectionSizeQuads;
	int32 NumSubsections;
	int32 MaxX;
	int32 MaxY;

	if (!SetLandscapeSizeParam(SubSectionSizeQuads, NumSubsections, MaxX, MaxY))
	{
		return nullptr;
	}

	TMap<FGuid, TArray<uint16>> HeightDataPerLayer;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;
	TArray<FLandscapeImportLayerInfo> MaterialImportLayers;
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

bool UGeneratorHeightMapLibrary::SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX,
                                                       int32& MaxY, const int32 Size)
{
	const int32 HeightmapSize = Size;
	FIntPoint NewLandscapeComponentCount;
	FLandscapeImportHelper::ChooseBestComponentSizeForImport(HeightmapSize, HeightmapSize, SubSectionSizeQuads,
	                                                         NumSubsections,
	                                                         NewLandscapeComponentCount);

	const int32 ComponentCountX = NewLandscapeComponentCount.X;
	const int32 ComponentCountY = NewLandscapeComponentCount.Y;
	const int32 QuadsPerComponent = NumSubsections * SubSectionSizeQuads;
	MaxX = ComponentCountX * QuadsPerComponent + 1;
	MaxY = ComponentCountY * QuadsPerComponent + 1;

	if (MaxX != HeightmapSize || MaxY != HeightmapSize)
	{
		UE_LOG(LogTemp, Error,
		       TEXT("Size calculated (%d x %d) not match with size of HeightMap (%d x %d)."))
		return false;
	}

	return true;
}
#pragma endregion

#pragma region Utilities
FTransform UGeneratorHeightMapLibrary::GetNewTransform(const FExternalHeightMapSettings& ExternalSettings,
                                                       const FLandscapeGenerationSettings& LandscapeSettings,
                                                       int32 HeightmapSize)
{
	FVector NewScale;

	if (ExternalSettings.bIsExternalHeightMap)
	{
		NewScale = FVector(ExternalSettings.ScalingX, ExternalSettings.ScalingY, ExternalSettings.ScalingZ);
	}
	else if (LandscapeSettings.bKilometers)
	{
		const FVector BaseScale = FVector(100, 100, 100);
		const float CurrentSizeInUnits = HeightmapSize * BaseScale.X;
		const float DesiredSizeInUnits = LandscapeSettings.Kilometers * 1000.f * 100.f;
		const float ScaleFactor = DesiredSizeInUnits / CurrentSizeInUnits;
		NewScale = BaseScale * ScaleFactor;
	}
	else
	{
		NewScale = FVector(100, 100, 100);
	}

	return FTransform(FQuat::Identity, FVector(-100800, -100800, 17200), NewScale);
}

void UGeneratorHeightMapLibrary::DestroyLastLandscape(const FLandscapeGenerationSettings& LandscapeSettings)
{
	if (LandscapeSettings.bDestroyLastLandscape)
	{
		if (LandscapeSettings.TargetLandscape != nullptr)
		{
			LandscapeSettings.TargetLandscape->Destroy();
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

bool UGeneratorHeightMapLibrary::SaveToAsset(UTexture2D* Texture, const FString& AssetName)
{
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Texture is null!"));
		return false;
	}

	// --- Preview-friendly flags: grayscale look, no mips/streaming/srgb, clamp ---
	auto ApplyPreviewTextureFlags = [](UTexture2D* T)
	{
		T->SRGB = true;
		// Use an un-intrusive compression setting for UI preview (no BC artifacts).
		T->CompressionSettings = TC_Default; // very light/no compression for UI icons
		T->MipGenSettings = TMGS_NoMipmaps;
		T->LODGroup = TEXTUREGROUP_Pixels2D;
		T->NeverStream = true;
		T->AddressX = TA_Clamp;
		T->AddressY = TA_Clamp;
	};

	// ----------------------------
	// 1) Read raw pixels from source (preferred) or platform mip[0].
	//    We'll convert them to an 8-bit Gray buffer.
	// ----------------------------
	int32 W = 0, H = 0;
	TArray64<uint8> Raw;
	enum class ERawFmt { Unknown, BGRA8, RGBA8, G8, G16, R32F, RGBA16F } InFmt = ERawFmt::Unknown;

	// Try Texture Source first
	{
		const int32 SrcW = Texture->Source.GetSizeX();
		const int32 SrcH = Texture->Source.GetSizeY();
		if (SrcW > 0 && SrcH > 0 && Texture->Source.GetNumMips() > 0)
		{
			bool bOk = Texture->Source.GetMipData(Raw, 0);
			if (!bOk || Raw.Num() == 0)
			{
				void* Ptr = Texture->Source.LockMip(0);
				const int64 DataSize = Texture->Source.CalcMipSize(0);
				if (Ptr && DataSize > 0)
				{
					Raw.SetNumUninitialized(DataSize);
					FMemory::Memcpy(Raw.GetData(), Ptr, DataSize);
				}
				Texture->Source.UnlockMip(0);
			}
			if (Raw.Num() > 0)
			{
				W = SrcW;
				H = SrcH;
				switch (Texture->Source.GetFormat())
				{
				case TSF_BGRA8: InFmt = ERawFmt::BGRA8;
					break;
				case TSF_RGBA8: InFmt = ERawFmt::RGBA8;
					break;
				case TSF_G8: InFmt = ERawFmt::G8;
					break;
				case TSF_G16: InFmt = ERawFmt::G16;
					break;
				case TSF_RGBA16F: InFmt = ERawFmt::RGBA16F;
					break;
				default: InFmt = ERawFmt::Unknown;
					break;
				}
			}
		}
	}

	// Fallback: PlatformData
	if (InFmt == ERawFmt::Unknown)
	{
		FTexturePlatformData* PD = Texture->GetPlatformData();
		if (!PD || PD->Mips.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("No Texture Source and no PlatformData available."));
			return false;
		}

		FTexture2DMipMap& Mip = PD->Mips[0];
		const int64 DataSize = Mip.BulkData.GetBulkDataSize();
		if (DataSize <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Platform mip has no data."));
			return false;
		}

		const void* MipData = Mip.BulkData.LockReadOnly();
		if (!MipData)
		{
			Mip.BulkData.Unlock();
			UE_LOG(LogTemp, Error, TEXT("Failed to lock platform mip data."));
			return false;
		}

		W = Mip.SizeX;
		H = Mip.SizeY;
		const EPixelFormat PF = Texture->GetPixelFormat();
		const int64 NumPx = (int64)W * H;

		if (PF == PF_B8G8R8A8 || PF == PF_R8G8B8A8)
		{
			InFmt = ERawFmt::BGRA8;
			Raw.SetNumUninitialized(NumPx * 4);
			FMemory::Memcpy(Raw.GetData(), MipData, Raw.Num());
		}
		else if (PF == PF_G16)
		{
			InFmt = ERawFmt::G16;
			Raw.SetNumUninitialized(NumPx * 2);
			FMemory::Memcpy(Raw.GetData(), MipData, Raw.Num());
		}
		else if (PF == PF_R32_FLOAT)
		{
			InFmt = ERawFmt::R32F;
			Raw.SetNumUninitialized(NumPx * 4);
			FMemory::Memcpy(Raw.GetData(), MipData, Raw.Num());
		}
		else
		{
			InFmt = ERawFmt::Unknown;
		}

		Mip.BulkData.Unlock();
	}

	if (InFmt == ERawFmt::Unknown || Raw.Num() == 0 || W <= 0 || H <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Unsupported or empty source format when saving preview asset."));
		return false;
	}

	// ----------------------------
	// 2) Convert ANY input to Gray (8-bit) then replicate Gray to BGRA (B=G=R=Gray, A=255).
	// ----------------------------
	const int64 NumPx = (int64)W * H;
	TArray64<uint8> Gray;
	Gray.SetNumUninitialized(NumPx);

	switch (InFmt)
	{
	case ERawFmt::BGRA8:
		{
			const uint8* p = Raw.GetData();
			for (int64 i = 0; i < NumPx; ++i)
			{
				// BGRA: B=0,G=1,R=2,A=3 
				Gray[i] = p[i * 4 + 2];
			}
			break;
		}
	case ERawFmt::RGBA8:
		{
			const uint8* p = Raw.GetData();
			for (int64 i = 0; i < NumPx; ++i)
			{
				// RGBA: R=0,G=1,B=2,A=3
				Gray[i] = p[i * 4 + 0];
			}
			break;
		}
	case ERawFmt::G8:
		{
			FMemory::Memcpy(Gray.GetData(), Raw.GetData(), Gray.Num());
			break;
		}
	case ERawFmt::G16:
		{
			const uint16* p = reinterpret_cast<const uint16*>(Raw.GetData());
			for (int64 i = 0; i < NumPx; ++i)
			{
				Gray[i] = uint8(p[i] >> 8); // keep MSB
			}
			break;
		}
	case ERawFmt::R32F:
		{
			const float* p = reinterpret_cast<const float*>(Raw.GetData());
			for (int64 i = 0; i < NumPx; ++i)
			{
				const float v = FMath::Clamp(p[i], 0.f, 1.f);
				Gray[i] = uint8(FMath::RoundToInt(v * 255.f));
			}
			break;
		}
	case ERawFmt::RGBA16F:
		{
			const FFloat16Color* p = reinterpret_cast<const FFloat16Color*>(Raw.GetData());
			for (int64 i = 0; i < NumPx; ++i)
			{
				const float R = FMath::Clamp(p[i].R, 0.f, 1.f);
				Gray[i] = uint8(FMath::RoundToInt(R * 255.f));
			}
			break;
		}
	default:
		UE_LOG(LogTemp, Error, TEXT("Unexpected raw format during grayscale conversion."));
		return false;
	}

	// Build BGRA8 from Gray (B=G=R=Gray, A=255) so Slate sees neutral grayscale.
	TArray64<uint8> BGRA;
	BGRA.SetNumUninitialized(NumPx * 4);
	for (int64 i = 0; i < NumPx; ++i)
	{
		const uint8 g = Gray[i];
		const int64 o = i * 4;
		BGRA[o + 0] = g; // B
		BGRA[o + 1] = g; // G
		BGRA[o + 2] = g; // R
		BGRA[o + 3] = 255; // A
	}

	// ----------------------------
	// 3) Create or update the asset as BGRA8 (with preview flags).
	// ----------------------------
	const FString PackageName = FString::Printf(TEXT("/Game/SavedAssets/%s"), *AssetName);

	UPackage* ExistingPackage = FindPackage(nullptr, *PackageName);
	UTexture2D* TargetTexture = ExistingPackage
		                            ? Cast<UTexture2D>(
			                            StaticFindObject(UTexture2D::StaticClass(), ExistingPackage, *AssetName))
		                            : nullptr;

	auto SaveAndRescan = [&](UPackage* Pkg, UObject* Asset)-> bool
	{
		FlushRenderingCommands();

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		const FString PkgFilename = FPackageName::LongPackageNameToFilename(
			PackageName, FPackageName::GetAssetPackageExtension());

		if (!UPackage::SavePackage(Pkg, Asset, *PkgFilename, SaveArgs))
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save package '%s'"), *PackageName);
			return false;
		}

		if (FAssetRegistryModule* ARM = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry"))
		{
			TArray<FString> PathsToScan{TEXT("/Game/SavedAssets")};
			ARM->Get().ScanModifiedAssetFiles(PathsToScan);
		}

		UE_LOG(LogTemp, Log, TEXT("Saved texture to: %s"), *PkgFilename);
		return true;
	};

	auto WriteBGRA8ToTexture = [&](UTexture2D* T)
	{
		T->Modify();

		// Bake as BGRA8 so Slate always sees grayscale (R=G=B) instead of a red-only channel.
		T->Source.Init(W, H, 1, 1, TSF_BGRA8);
		void* Dest = T->Source.LockMip(0);
		const int64 DestSize = NumPx * 4;
		FMemory::Memcpy(Dest, BGRA.GetData(), DestSize);
		T->Source.UnlockMip(0);

		ApplyPreviewTextureFlags(T);
		T->PostEditChange();
		T->UpdateResource();
		(void)T->MarkPackageDirty();
	};

	if (TargetTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("Texture '%s' already exists, updating asset..."), *AssetName);
		WriteBGRA8ToTexture(TargetTexture);
		return SaveAndRescan(TargetTexture->GetOutermost(), TargetTexture);
	}
	else
	{
		UPackage* NewPackage = CreatePackage(*PackageName);
		NewPackage->FullyLoad();

		UTexture2D* NewTexture = NewObject<UTexture2D>(
			NewPackage, UTexture2D::StaticClass(), *AssetName, RF_Public | RF_Standalone);

		WriteBGRA8ToTexture(NewTexture);
		FAssetRegistryModule::AssetCreated(NewTexture);
		return SaveAndRescan(NewPackage, NewTexture);
	}
}


void UGeneratorHeightMapLibrary::OpenHeightmapFileDialog(
	TSharedPtr<FExternalHeightMapSettings> ExternalSettings,
	TSharedPtr<FLandscapeGenerationSettings> LandscapeSettings,
	TSharedPtr<FHeightMapGenerationSettings> HeightMapSettings)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform) return;

	TArray<FString> OutFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	bool bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Select PNG Heightmap"),
		FPaths::ProjectDir(),
		TEXT(""),
		TEXT("PNG files (*.png)|*.png"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (bOpened && OutFiles.Num() > 0)
	{
		const FString& SelectedFile = OutFiles[0];

		if (ExternalSettings.IsValid())
		{
			ExternalSettings->bIsExternalHeightMap = true;
			ExternalSettings->LastPNGPath = SelectedFile;
		}

		/*CreateLandscapeFromOtherHeightMap(
			SelectedFile, *ExternalSettings, *LandscapeSettings, *HeightMapSettings
		); */
	}
	const FString& SelectedFile = OutFiles[0];

	if (UTexture2D* Imported = FImageUtils::ImportFileAsTexture2D(SelectedFile))
	{
		if (SaveToAsset(Imported, TEXT("TextureHeightMap")))
		{
			UE_LOG(LogTemp, Log, TEXT("Preview overwritten from external PNG: %s"), *SelectedFile);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to overwrite preview from PNG: %s"), *SelectedFile);
		}
	}

	//CreateLandscapeFromOtherHeightMap(SelectedFile, *ExternalSettings, *LandscapeSettings, *HeightMapSettings);
}
static FVector GetWindPreviewStart(const FLandscapeGenerationSettings& LS, UWorld* World)
{
    // If we have a target landscape, use its bounds center; else use world origin slightly lifted.
    if (LS.TargetLandscape)
    {
        FBox Bounds(ForceInitToZero);
        TArray<ULandscapeComponent*> Comps;
        LS.TargetLandscape->GetComponents(Comps);
        for (ULandscapeComponent* C : Comps)
        {
            if (!C) continue;
            Bounds += C->Bounds.GetBox();
        }
        if (Bounds.IsValid)
        {
            FVector Center = Bounds.GetCenter();
            Center.Z += 200.f;
            return Center;
        }

        // Fallback: actor location
        FVector Loc = LS.TargetLandscape->GetActorLocation();
        Loc.Z += 200.f;
        return Loc;
    }

    // Last resort: world origin
    return FVector(0,0,200);
}

void UGeneratorHeightMapLibrary::DrawWindDirectionPreview(
    const FLandscapeGenerationSettings& LandscapeSettings,
    float ArrowLength,
    float ArrowThickness,
    float ArrowHeadSize,
    float Duration,
    bool  bAlsoDrawCone,
    float ConeHalfAngleDeg)
{
#if WITH_EDITOR
    const float Scale = WindPreviewScale;

    UWorld* World = nullptr;
    if (GEditor)
    {
        const FWorldContext& Ctx = GEditor->GetEditorWorldContext();
        World = Ctx.World();
    }
    if (!World) return;

    const FVector Start = GetWindPreviewStart(LandscapeSettings, World);

    const float MeanDeg = UErosionLibrary::GetWindMeanAngleDegrees();
    const FVector2D Dir2D = UErosionLibrary::GetWindUnitVectorFromAngle(MeanDeg);
    const FVector Dir3D(Dir2D.X, Dir2D.Y, 0.f);
    const FVector End = Start + Dir3D * (ArrowLength * Scale);

    DrawDebugDirectionalArrow(World, Start, End, ArrowHeadSize * Scale,
                              FColor::Cyan, false, Duration, 0, ArrowThickness * Scale);

    // Cardinal cross
    const float Cross = ArrowLength * 0.25f * Scale;
    DrawDebugLine(World, Start - FVector(Cross,0,0), Start + FVector(Cross,0,0),
                  FColor(128,128,128), false, Duration, 0, 1.f * Scale);
    DrawDebugLine(World, Start - FVector(0,Cross,0), Start + FVector(0,Cross,0),
                  FColor(128,128,128), false, Duration, 0, 1.f * Scale);

    DrawDebugString(World, Start + FVector(Cross,0,50*Scale),  TEXT("E"), nullptr, FColor::White, Duration, false);
    DrawDebugString(World, Start + FVector(-Cross,0,50*Scale), TEXT("W"), nullptr, FColor::White, Duration, false);
    DrawDebugString(World, Start + FVector(0,Cross,50*Scale),  TEXT("N"), nullptr, FColor::White, Duration, false);
    DrawDebugString(World, Start + FVector(0,-Cross,50*Scale), TEXT("S"), nullptr, FColor::White, Duration, false);

    // Bias cone
    if (bAlsoDrawCone && UErosionLibrary::GetWindBias())
    {
        const int32 Spokes = 6;
        for (int32 i = 0; i <= Spokes; ++i)
        {
            const float T   = (Spokes == 0) ? 0.f : float(i)/float(Spokes);
            const float Deg = MeanDeg - ConeHalfAngleDeg + (2.f * ConeHalfAngleDeg) * T;

            const FVector2D S2D = UErosionLibrary::GetWindUnitVectorFromAngle(Deg);
            const FVector   S3D(S2D.X, S2D.Y, 0.f);
            const FVector   SEnd = Start + S3D * (ArrowLength * 0.85f * Scale);

            DrawDebugDirectionalArrow(World, Start, SEnd, ArrowHeadSize*0.75f*Scale,
                                      FColor(0,200,255), false, Duration, 0, ArrowThickness*0.75f*Scale);
        }
    }
#endif
}
#pragma endregion
