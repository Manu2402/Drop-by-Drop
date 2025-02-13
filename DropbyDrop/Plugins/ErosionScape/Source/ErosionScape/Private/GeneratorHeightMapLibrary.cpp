#include "GeneratorHeightMapLibrary.h"

#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "ImageUtils.h"
#include "LandscapeProxy.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ErosionLibrary.h"
#include "FileHelpers.h"
#include "LandscapeImportHelper.h"
#include "LandscapeSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Misc/ScopedSlowTask.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "UObject/SavePackage.h"

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
//External Heightmap Param
float UGeneratorHeightMapLibrary::ScalingX = 100;
float UGeneratorHeightMapLibrary::ScalingY = 100;
float UGeneratorHeightMapLibrary::ScalingZ = 100;
bool UGeneratorHeightMapLibrary::bIsExternalHeightMap = false;
//LandScapeParam
ALandscape* UGeneratorHeightMapLibrary::StaticLandscape = nullptr;
int32 UGeneratorHeightMapLibrary::Kilometers = 1;
bool UGeneratorHeightMapLibrary::bKilometers = true;
int32 UGeneratorHeightMapLibrary::WorldPartitionGridSize = 4;
bool UGeneratorHeightMapLibrary::bDestroyLastLandscape = false;

UDataTable* UGeneratorHeightMapLibrary::ErosionTemplatesDataTable = nullptr;
#pragma endregion

#pragma region Erosion
void UGeneratorHeightMapLibrary::GenerateErosion()
{
	UErosionLibrary::SetHeights(HeightMap);

	UErosionLibrary::ErosionHandler(Size);

	TArray<uint16> ErodedHeightmapU16 = ConvertFloatArrayToUint16(UErosionLibrary::GetHeights());

	// Generate new landscape.
	const FTransform LandscapeTransform = GetNewTransform(bIsExternalHeightMap);

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

bool UGeneratorHeightMapLibrary::SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue,
													 const uint8 WindDirectionValue,
													 const float InertiaValue, const int32 CapacityValue,
													 const float MinSlopeValue, const float DepositionSpeedValue,
													 const float ErosionSpeedValue, const int32 GravityValue,
													 const float EvaporationValue, const int32 MaxPathValue,
													 const int32 ErosionRadiusValue)
{
	// Row fields.
	FErosionTemplateRow ErosionTemplateRow;
	ErosionTemplateRow.ErosionCyclesField = ErosionCyclesValue;
	ErosionTemplateRow.WindDirection = WindDirectionValue;
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

	FErosionTemplateRow* RowData = ErosionTemplatesDataTable->FindRow<FErosionTemplateRow>(FName(RowName), ContextString);

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
void UGeneratorHeightMapLibrary::CreateHeightMap(const int32 MapSize)
{
	HeightMap = GenerateHeightMapCPU(MapSize);

	UErosionLibrary::SetHeights(HeightMap);

	UTexture2D* Texture = CreateHeightMapTexture(HeightMap, MapSize, MapSize);

	if(SaveToAsset(Texture, "TextureHeightMap"))
	{
		UE_LOG(LogTemp, Log, TEXT("Save Texture"));
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("Problem in save texture"));
	}
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
	bIsExternalHeightMap = false;
	return HeightMapValues;
}

UTexture2D* UGeneratorHeightMapLibrary::CreateHeightMapTexture(const TArray<float>& HeightMapData, const int32 Width, const int32 Height)
{
	// **Creazione della texture salvabile**
	UTexture2D* Texture = NewObject<UTexture2D>(GetTransientPackage(), UTexture2D::StaticClass(), FName(*FString::Printf(TEXT("GeneratedTexture_%d"), FMath::Rand())), RF_Public | RF_Standalone);

	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create texture!"));
		return nullptr;
	}

	// **Assegniamo la PlatformData con i nuovi metodi**
	FTexturePlatformData* PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = Width;
	PlatformData->SizeY = Height;
	PlatformData->PixelFormat = PF_B8G8R8A8; // **Formato supportato**

	// **Crea un mipmap manualmente**
	FTexture2DMipMap* MipMap = new FTexture2DMipMap();
	MipMap->SizeX = Width;
	MipMap->SizeY = Height;
	PlatformData->Mips.Add(MipMap);

	// **Alloca memoria per la texture**
	MipMap->BulkData.Lock(LOCK_READ_WRITE);
	uint8* MipData = static_cast<uint8*>(MipMap->BulkData.Realloc(Width * Height * 4)); // RGBA = 4 byte per pixel
	FMemory::Memzero(MipData, Width * Height * 4); // Pulisce i dati iniziali

	// **Riempie i pixel con la heightmap**
	for (int32 i = 0; i < HeightMapData.Num(); ++i)
	{
		const uint8 GrayValue = FMath::Clamp(static_cast<uint8>(HeightMapData[i] * 255.0f), 0, 255);
		MipData[i * 4 + 0] = GrayValue; // R
		MipData[i * 4 + 1] = GrayValue; // G
		MipData[i * 4 + 2] = GrayValue; // B
		MipData[i * 4 + 3] = 255;       // A
	}

	MipMap->BulkData.Unlock();

	// **Assegna PlatformData alla texture**
	Texture->SetPlatformData(PlatformData);

	// **Impostazioni di compressione e mipmaps**
	Texture->SRGB = true;
	Texture->CompressionSettings = TC_Default;
	Texture->MipGenSettings = TMGS_FromTextureGroup;
	Texture->UpdateResource();

	return Texture;
}


void UGeneratorHeightMapLibrary::LoadHeightmapFromPNG(const FString& FilePath, TArray<uint16>& OutHeightmap, TArray<float>& OutNormalizedHeightmap)
{
    OutHeightmap.Empty();
    OutNormalizedHeightmap.Empty();

    // Carica la texture
    UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
    if (!Texture)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load PNG file: %s"), *FilePath);
        return;
    }

    // Ottieni i dati della texture
    FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
    const void* MipData = MipMap.BulkData.LockReadOnly();
    int32 Width = MipMap.SizeX;
    int32 Height = MipMap.SizeY;

    UE_LOG(LogTemp, Log, TEXT("Loaded PNG: Width=%d, Height=%d"), Width, Height);

    uint32 MinPixel = TNumericLimits<uint32>::Max();
    uint32 MaxPixel = TNumericLimits<uint32>::Min();

    OutHeightmap.SetNum(Width * Height);
    OutNormalizedHeightmap.SetNum(Width * Height);

    // Rilevamento del formato e lettura dei valori
    if (Texture->GetPixelFormat() == PF_R8G8B8A8) // RGBA formato
    {
        const uint8* PixelData = static_cast<const uint8*>(MipData);
        for (int32 i = 0; i < Width * Height; ++i)
        {
            uint8 RedValue = PixelData[i * 4];
            uint16 HeightValue = static_cast<uint16>(RedValue << 8); // Scala a 16 bit
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

    // Normalizzazione a float tra 0 e 1
    for (int32 i = 0; i < OutHeightmap.Num(); ++i)
    {
        if (MaxPixel > MinPixel)
        {
            OutNormalizedHeightmap[i] = (OutHeightmap[i] - MinPixel) / static_cast<float>(MaxPixel - MinPixel);
        }
        else
        {
            OutNormalizedHeightmap[i] = 0.0f; // Default a zero se i valori sono uniformi
        }
    }
	bIsExternalHeightMap = true;
    UE_LOG(LogTemp, Log, TEXT("Heightmap Min: %u, Max: %u"), MinPixel, MaxPixel);

}
void CompareHeightmaps(const FString& RawFilePath, const TArray<uint16>& GeneratedHeightmap, int32 Width, int32 Height)
{
	// Leggi il file RAW esportato da Unreal
	TArray<uint8> RawData; // Caricheremo i dati grezzi come uint8
	if (!FFileHelper::LoadFileToArray(RawData, *RawFilePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load RAW file: %s"), *RawFilePath);
		return;
	}

	// Assicuriamoci che la dimensione corrisponda a Width * Height * 2 (uint16 = 2 byte)
	const uint64 ExpectedSize = static_cast<uint64>(Width) * static_cast<uint64>(Height) * sizeof(uint16);
	if (RawData.Num() != ExpectedSize)
	{
		UE_LOG(LogTemp, Error, TEXT("RAW file size does not match the expected size: %llu vs %llu"),
			   static_cast<uint64>(RawData.Num()), ExpectedSize);
		return;
	}

	// Converti il TArray<uint8> in TArray<uint16>
	TArray<uint16> UnrealHeightmap;
	UnrealHeightmap.SetNum(Width * Height);
	FMemory::Memcpy(UnrealHeightmap.GetData(), RawData.GetData(), RawData.Num());

	// Confronta i valori
	for (int32 i = 0; i < FMath::Min(10, UnrealHeightmap.Num()); ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("RAW[%d]: %d, Generated[%d]: %d"),
			   i, UnrealHeightmap[i], i, GeneratedHeightmap[i]);
	}
}
TArray<uint16> UGeneratorHeightMapLibrary::ResizeHeightmapBilinear(
	const TArray<uint16>& InputHeightmap, 
	int32 SrcWidth, int32 SrcHeight, 
	int32 TargetWidth, int32 TargetHeight)
{
	TArray<uint16> ResizedHeightmap;
	ResizedHeightmap.SetNum(TargetWidth * TargetHeight);

	for (int32 Y = 0; Y < TargetHeight; ++Y)
	{
		for (int32 X = 0; X < TargetWidth; ++X)
		{
			// Calcola la posizione nella heightmap originale
			float SrcX = static_cast<float>(X) / TargetWidth * SrcWidth;
			float SrcY = static_cast<float>(Y) / TargetHeight * SrcHeight;

			int32 X0 = FMath::FloorToInt(SrcX);
			int32 Y0 = FMath::FloorToInt(SrcY);
			int32 X1 = FMath::Min(X0 + 1, SrcWidth - 1);
			int32 Y1 = FMath::Min(Y0 + 1, SrcHeight - 1);

			// Peso per l'interpolazione bilineare
			float Fx = SrcX - X0;
			float Fy = SrcY - Y0;

			// Calcolo dei valori interpolati
			uint16 P00 = InputHeightmap[Y0 * SrcWidth + X0];
			uint16 P10 = InputHeightmap[Y0 * SrcWidth + X1];
			uint16 P01 = InputHeightmap[Y1 * SrcWidth + X0];
			uint16 P11 = InputHeightmap[Y1 * SrcWidth + X1];

			uint16 InterpolatedValue = static_cast<uint16>(
				(1 - Fx) * (1 - Fy) * P00 +
				Fx * (1 - Fy) * P10 +
				(1 - Fx) * Fy * P01 +
				Fx * Fy * P11
			);

			ResizedHeightmap[Y * TargetWidth + X] = InterpolatedValue;
		}
	}

	return ResizedHeightmap;
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

	const FTransform LandscapeTransform = GetNewTransform(bIsExternalHeightMap);

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
void UGeneratorHeightMapLibrary::CreateLandscapeFromOtherHeightMap(const FString& FilePath)
{
	// Step 1: Caricare la heightmap dal file PNG
	TArray<uint16> HeightmapInt16;
	LoadHeightmapFromPNG(FilePath,HeightmapInt16, HeightMap);
	FString HeightMapPath = FPaths::ProjectDir() + TEXT("Saved/HeightMap/raw.r16");
	CompareHeightmaps(HeightMapPath, HeightmapInt16, 505, 505);
	if (HeightmapInt16.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load heightmap from PNG file: %s"), *FilePath);
		return;
	}
	const FTransform LandscapeTransform = GetNewTransform(bIsExternalHeightMap);
	// Step 2: Generare il landscape utilizzando la heightmap caricata
	StaticLandscape = GenerateLandscape(LandscapeTransform, HeightmapInt16);
	if (!StaticLandscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to generate landscape from heightmap."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Landscape successfully created from PNG: %s"), *FilePath);

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
FTransform UGeneratorHeightMapLibrary::GetNewTransform(bool bExternalHeightmap)
{
	FVector NewScale;
	if(bExternalHeightmap)
	{
		NewScale = FVector(ScalingX, ScalingY, ScalingZ);
		const FTransform LandscapeTransform = FTransform(FQuat(FRotator::ZeroRotator), FVector(-100800, -100800, 17200), NewScale);
		return LandscapeTransform;
	}
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

        FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
        bool bSucceeded = UPackage::SavePackage(PackageToSave, ExistingTexture, *PackageFileName, SaveArgs);
        if (!bSucceeded)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to save updated package '%s'!"), *PackageName);
            return false;
        }


        if (FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry"))
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
	
    UTexture2D* NewTexture = NewObject<UTexture2D>(NewPackage, UTexture2D::StaticClass(), *AssetName, RF_Public | RF_Standalone);

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

    FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
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





#pragma endregion