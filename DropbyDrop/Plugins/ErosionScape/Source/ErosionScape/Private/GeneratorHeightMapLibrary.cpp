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

#pragma region InitParam

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
                                                      TArray<float>& OutNormalizedHeightmap, FExternalHeightMapSettings& Settings)
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

void UGeneratorHeightMapLibrary::CreateLandscapeFromOtherHeightMap(const FString& FilePath,
	FExternalHeightMapSettings& ExternalSettings,
	 FLandscapeGenerationSettings& LandscapeSettings , FHeightMapGenerationSettings& HeightmapSettings)
{
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
		LandscapeSubsystem->ChangeGridSize(LandscapeInfo, LandscapeSettings.WorldPartitionGridSize);
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

	// create packet name
	FString PackageName = FString::Printf(TEXT("/Game/SavedAssets/%s"), *AssetName);

	//check if already exsist
	UPackage* ExistingPackage = FindPackage(nullptr, *PackageName);
	UTexture2D* ExistingTexture = nullptr;
	if (ExistingPackage)
	{
		ExistingTexture = Cast<UTexture2D>(StaticFindObject(UTexture2D::StaticClass(), ExistingPackage, *AssetName));
	}

	//if texture already exist, modify the texture
	if (ExistingTexture)
	{
		UE_LOG(LogTemp, Warning, TEXT("Texture '%s' already exists, updating asset..."), *AssetName);

		ExistingTexture->Modify();

		if (Texture->GetPlatformData())
		{
			FTexturePlatformData* SrcPD = Texture->GetPlatformData();
			//New platform data
			FTexturePlatformData* NewPD = new FTexturePlatformData();
			NewPD->SizeX = SrcPD->SizeX;
			NewPD->SizeY = SrcPD->SizeY;
			NewPD->PixelFormat = SrcPD->PixelFormat;

			if (SrcPD->Mips.Num() > 0)
			{
				const FTexture2DMipMap& SourceMip = SrcPD->Mips[0];
				FTexture2DMipMap* NewMip = new FTexture2DMipMap();
				NewMip->SizeX = SourceMip.SizeX;
				NewMip->SizeY = SourceMip.SizeY;
				int32 DataSize = SourceMip.BulkData.GetBulkDataSize();
				NewMip->BulkData.Lock(LOCK_READ_WRITE);
				void* DestData = NewMip->BulkData.Realloc(DataSize);
				const void* SrcData = SourceMip.BulkData.LockReadOnly();
				FMemory::Memcpy(DestData, SrcData, DataSize);
				SourceMip.BulkData.Unlock();
				NewMip->BulkData.Unlock();
				NewPD->Mips.Add(NewMip);
			}
			ExistingTexture->SetPlatformData(NewPD);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT(" Original texture has no PlatformData!"));
			return false;
		}

		// Copyh infos
		ExistingTexture->SRGB = Texture->SRGB;
		ExistingTexture->CompressionSettings = Texture->CompressionSettings;
		ExistingTexture->MipGenSettings = Texture->MipGenSettings;

		if (ExistingTexture->GetPlatformData() && ExistingTexture->GetPlatformData()->Mips.Num() > 0)
		{
			int32 Width = ExistingTexture->GetPlatformData()->SizeX;
			int32 Height = ExistingTexture->GetPlatformData()->SizeY;
			ExistingTexture->Source.Init(Width, Height, 1, 1, TSF_BGRA8);

			FTexture2DMipMap& Mip = ExistingTexture->GetPlatformData()->Mips[0];
			int32 DataSize = Mip.BulkData.GetBulkDataSize();
			void* DestSourceData = ExistingTexture->Source.LockMip(0);
			TArray64<uint8> TempData;
			TempData.SetNumUninitialized(DataSize);
			Mip.BulkData.Lock(LOCK_READ_ONLY);
			FMemory::Memcpy(TempData.GetData(), Mip.BulkData.Realloc(DataSize), DataSize);
			Mip.BulkData.Unlock();
			FMemory::Memcpy(DestSourceData, TempData.GetData(), DataSize);
			ExistingTexture->Source.UnlockMip(0);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("No mip data available in PlatformData!"));
		}

		ExistingTexture->PostEditChange();
		ExistingTexture->UpdateResource();
		(void)ExistingTexture->MarkPackageDirty();

		UPackage* PackageToSave = ExistingTexture->GetOutermost();
		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		FString PackageFileName = FPackageName::LongPackageNameToFilename(
			PackageName, FPackageName::GetAssetPackageExtension());
		bool bSucceeded = UPackage::SavePackage(PackageToSave, ExistingTexture, *PackageFileName, SaveArgs);
		if (!bSucceeded)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save updated package '%s'!"), *PackageName);
			return false;
		}


		if (FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>(
			"AssetRegistry"))
		{
			TArray<FString> PathsToScan;
			PathsToScan.Add(TEXT("/Game/SavedAssets"));
			AssetRegistryModule->Get().ScanModifiedAssetFiles(PathsToScan);
		}

		UE_LOG(LogTemp, Log, TEXT("Successfully updated texture asset at: %s"), *PackageFileName);
		return true;
	}

	//If texture doesn't exist, create
	UPackage* NewPackage = CreatePackage(*PackageName);
	NewPackage->FullyLoad();

	UTexture2D* NewTexture = NewObject<UTexture2D>(NewPackage, UTexture2D::StaticClass(), *AssetName,
	                                               RF_Public | RF_Standalone);

	if (Texture->GetPlatformData())
	{
		FTexturePlatformData* SrcPD = Texture->GetPlatformData();
		FTexturePlatformData* NewPD = new FTexturePlatformData();
		NewPD->SizeX = SrcPD->SizeX;
		NewPD->SizeY = SrcPD->SizeY;
		NewPD->PixelFormat = SrcPD->PixelFormat;
		if (SrcPD->Mips.Num() > 0)
		{
			const FTexture2DMipMap& SourceMip = SrcPD->Mips[0];
			FTexture2DMipMap* NewMip = new FTexture2DMipMap();
			NewMip->SizeX = SourceMip.SizeX;
			NewMip->SizeY = SourceMip.SizeY;
			int32 DataSize = SourceMip.BulkData.GetBulkDataSize();
			NewMip->BulkData.Lock(LOCK_READ_WRITE);
			void* DestData = NewMip->BulkData.Realloc(DataSize);
			const void* SrcData = SourceMip.BulkData.LockReadOnly();
			FMemory::Memcpy(DestData, SrcData, DataSize);
			SourceMip.BulkData.Unlock();
			NewMip->BulkData.Unlock();
			NewPD->Mips.Add(NewMip);
		}
		NewTexture->SetPlatformData(NewPD);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Original texture has no PlatformData!"));
		return false;
	}

	NewTexture->SRGB = Texture->SRGB;
	NewTexture->CompressionSettings = Texture->CompressionSettings;
	NewTexture->MipGenSettings = Texture->MipGenSettings;

	if (NewTexture->GetPlatformData() && NewTexture->GetPlatformData()->Mips.Num() > 0)
	{
		int32 Width = NewTexture->GetPlatformData()->SizeX;
		int32 Height = NewTexture->GetPlatformData()->SizeY;
		NewTexture->Source.Init(Width, Height, 1, 1, TSF_BGRA8);
		FTexture2DMipMap& NewMip = NewTexture->GetPlatformData()->Mips[0];
		int32 DataSize = NewMip.BulkData.GetBulkDataSize();
		void* DestSourceData = NewTexture->Source.LockMip(0);
		TArray64<uint8> TempData;
		TempData.SetNumUninitialized(DataSize);
		NewMip.BulkData.Lock(LOCK_READ_ONLY);
		FMemory::Memcpy(TempData.GetData(), NewMip.BulkData.Realloc(DataSize), DataSize);
		NewMip.BulkData.Unlock();
		FMemory::Memcpy(DestSourceData, TempData.GetData(), DataSize);
		NewTexture->Source.UnlockMip(0);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No mip data available in PlatformData!"));
	}

	NewTexture->PostEditChange();
	NewTexture->UpdateResource();
	(void)NewTexture->MarkPackageDirty();

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_NoError;

	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName,
	                                                                  FPackageName::GetAssetPackageExtension());
	bool bSucceeded = UPackage::SavePackage(NewPackage, NewTexture, *PackageFileName, SaveArgs);
	if (!bSucceeded)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save package '%s'!"), *PackageName);
		return false;
	}

	if (FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry"))
	{
		TArray<FString> PathsToScan;
		PathsToScan.Add(TEXT("/Game/SavedAssets"));
		AssetRegistryModule->Get().ScanModifiedAssetFiles(PathsToScan);
	}

	UE_LOG(LogTemp, Log, TEXT("Successfully saved texture to: %s"), *PackageFileName);
	return true;
}

void UGeneratorHeightMapLibrary::OpenHeightmapFileDialog(TSharedRef<FExternalHeightMapSettings> ExternalSettings,
												  TSharedRef<FLandscapeGenerationSettings> LandscapeSettings,
												  TSharedRef<FHeightMapGenerationSettings> HeightMapSettings)
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
		CreateLandscapeFromOtherHeightMap(
			SelectedFile, *ExternalSettings, *LandscapeSettings, *HeightMapSettings
		);
	}
}

#pragma endregion
