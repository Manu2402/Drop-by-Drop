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
//External Heightmap Param
float UGeneratorHeightMapLibrary::ScalingX = 100;
float UGeneratorHeightMapLibrary::ScalingY = 100;
float UGeneratorHeightMapLibrary::ScalingZ = 100;
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
	const FTransform LandscapeTransform = GetNewTransform(false);

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

bool UGeneratorHeightMapLibrary::SaveErosionTemplate(UDataTable* ErosionTemplates, const FString& TemplateName, const int32 ErosionCyclesValue,
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
	
	ErosionTemplates->AddRow(FName(TemplateName), ErosionTemplateRow);
	return true;
}

FErosionTemplateRow* UGeneratorHeightMapLibrary::LoadErosionTemplate(const FName& RowName)
{
	// Code repetition.
	FString ContextString = TEXT("DataTable Context");
	ErosionTemplatesDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Custom/ErosionTemplates/DT_ErosionTemplate.DT_ErosionTemplate"));

	if (!ErosionTemplatesDataTable)
	{
		return nullptr;
	}

	FErosionTemplateRow* RowData = ErosionTemplatesDataTable->FindRow<FErosionTemplateRow>(RowName, ContextString);

	if (!RowData)
	{
		return nullptr;
	}

	return RowData;
}

bool UGeneratorHeightMapLibrary::LoadRowIntoErosionFields(const FErosionTemplateRow* TemplateDatas)
{
	UErosionLibrary::SetErosion(TemplateDatas->ErosionCyclesField);
	UErosionLibrary::SetInertia(TemplateDatas->InertiaField);
	UErosionLibrary::SetCapacity(TemplateDatas->CapacityField);
	UErosionLibrary::SetMinimalSlope(TemplateDatas->MinSlopeField);
	UErosionLibrary::SetDepositionSpeed(TemplateDatas->DepositionSpeedField);
	UErosionLibrary::SetErosionSpeed(TemplateDatas->ErosionSpeedField);
	UErosionLibrary::SetGravity(TemplateDatas->GravityField);
	UErosionLibrary::SetEvaporation(TemplateDatas->EvaporationField);
	UErosionLibrary::SetMaxPath(TemplateDatas->MaxPathField);
	UErosionLibrary::SetErosionRadius(TemplateDatas->ErosionRadiusField);

	return true;
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
TArray<uint16> UGeneratorHeightMapLibrary::LoadHeightmapFromPNG(const FString& FilePath)
{
	TArray<uint16> Heightmap;

    // Carica la texture
    UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
    if (!Texture)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load PNG file: %s"), *FilePath);
        return Heightmap;
    }

    // Ottieni i dati della texture
    FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
    const void* MipData = MipMap.BulkData.LockReadOnly();
    int32 Width = MipMap.SizeX;
    int32 Height = MipMap.SizeY;

    UE_LOG(LogTemp, Log, TEXT("Loaded PNG: Width=%d, Height=%d"), Width, Height);

    uint32 MinPixel = TNumericLimits<uint32>::Max();
    uint32 MaxPixel = TNumericLimits<uint32>::Min();

    Heightmap.SetNum(Width * Height);

    // Rilevamento del formato e lettura dei valori
    if (Texture->GetPixelFormat() == PF_R8G8B8A8) // RGBA formato
    {
        const uint8* PixelData = static_cast<const uint8*>(MipData);
        for (int32 i = 0; i < Width * Height; ++i)
        {
            // Usa solo il canale R come valore di altezza
            uint8 RedValue = PixelData[i * 4];
            Heightmap[i] = static_cast<uint16>(RedValue << 8); // Scala a 16 bit
            MinPixel = FMath::Min<uint32>(MinPixel, Heightmap[i]);
            MaxPixel = FMath::Max<uint32>(MaxPixel, Heightmap[i]);
        }
    }
    else if (Texture->GetPixelFormat() == PF_G16) // Grayscale 16 bit
    {
        const uint16* PixelData = static_cast<const uint16*>(MipData);
        for (int32 i = 0; i < Width * Height; ++i)
        {
            Heightmap[i] = PixelData[i];
            MinPixel = FMath::Min<uint32>(MinPixel, Heightmap[i]);
            MaxPixel = FMath::Max<uint32>(MaxPixel, Heightmap[i]);
        }
    }
    else if (Texture->GetPixelFormat() == PF_R32_FLOAT) // Grayscale 32 bit float
    {
        const float* PixelData = static_cast<const float*>(MipData);
        for (int32 i = 0; i < Width * Height; ++i)
        {
            Heightmap[i] = static_cast<uint16>(PixelData[i] * 65535.0f); // Normalizza a 16 bit
            MinPixel = FMath::Min<uint32>(MinPixel, Heightmap[i]);
            MaxPixel = FMath::Max<uint32>(MaxPixel, Heightmap[i]);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unsupported pixel format in PNG!"));
        MipMap.BulkData.Unlock();
        return Heightmap;
    }

    MipMap.BulkData.Unlock();

    UE_LOG(LogTemp, Log, TEXT("Heightmap Min: %u, Max: %u"), MinPixel, MaxPixel);

    // Debug dei primi 10 valori
    for (int32 i = 0; i < FMath::Min(10, Heightmap.Num()); ++i)
    {
        UE_LOG(LogTemp, Log, TEXT("Heightmap[%d]: %u"), i, Heightmap[i]);
    }

    return Heightmap;
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

	const FTransform LandscapeTransform = GetNewTransform(false);

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
ALandscape* UGeneratorHeightMapLibrary::CreateLandscapeFromOtherHeightMap(const FString& FilePath)
{
	// Step 1: Caricare la heightmap dal file PNG
	TArray<uint16> Heightmap = LoadHeightmapFromPNG(FilePath);
	FString HeightMapPath = FPaths::ProjectDir() + TEXT("Saved/HeightMap/raw.r16");
	CompareHeightmaps(HeightMapPath, Heightmap, 505, 505);
	if (Heightmap.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load heightmap from PNG file: %s"), *FilePath);
		return nullptr;
	}
	const FTransform LandscapeTransform = GetNewTransform(true);
	// Step 2: Generare il landscape utilizzando la heightmap caricata
	ALandscape* Landscape = GenerateLandscape(LandscapeTransform, Heightmap);
	if (!Landscape)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to generate landscape from heightmap."));
		return nullptr;
	}

	UE_LOG(LogTemp, Log, TEXT("Landscape successfully created from PNG: %s"), *FilePath);
	return Landscape;
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
		//NewScale = FVector(16887.37, 16887.37, 798.44);
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
/*
void UGeneratorHeightMapLibrary::ExportAssetLandScape()
{
	UE_LOG(LogTemp, Warning, TEXT("ExportAssetLandScape called"));
	
	const FString DestinationPath = FPaths::ProjectDir() / TEXT("Saved/FBX/test.fbx");
	bool bOutSuccess;
	FString OutInfoMessage;
	UObject* AssetToExport = StaticLandscape;

	const FString Extension = FPaths::GetExtension(DestinationPath).ToLower();
	UExporter* Exporter = UExporter::FindExporter(AssetToExport, *Extension);
	if (FModuleManager::Get().IsModuleLoaded("FBX"))
	{
		UE_LOG(LogTemp, Warning, TEXT("FBX module is loaded"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("FBX module is NOT loaded"));
	}
	for (TObjectIterator<UExporter> It; It; ++It)
	{
		UExporter* ExporterCandidate = *It;
		UE_LOG(LogTemp, Warning, TEXT("Exporter: %s, Formats: %s"),
			*ExporterCandidate->GetName(),
			*FString::Join(ExporterCandidate->FormatExtension, TEXT(", ")));
	}
	if(AssetToExport == nullptr)
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Export Asset Failed - AsssetToExport is not valid"));
		return;
	}
	
	if(Exporter == nullptr)
	{
		bOutSuccess = false;
		OutInfoMessage = FString::Printf(TEXT("Export Asset Failed - No Exporter found for extension"));
		return;
	}
	UAssetExportTask* ExportTask = NewObject<UAssetExportTask>();
	FGCObjectScopeGuard ExportTaskGuard(ExportTask);

	Exporter->ExportTask = ExportTask;
	ExportTask->Exporter = Exporter;

	//parameters
	ExportTask->Object = AssetToExport;
	ExportTask->Filename = DestinationPath;
	ExportTask->bReplaceIdentical = true;
	ExportTask->bPrompt = false; //No pop-up
	ExportTask->bAutomated = true;
	ExportTask->bUseFileArchive = false;
	ExportTask->bWriteEmptyFiles = false;
	ExportTask->bSelected = false; 
	ExportTask->IgnoreObjectList = TArray<TObjectPtr<UObject>>(); //For Assets tht have multiple obj

	//Set the exporter parameters
	if(Extension == "fbx")
	{
		//FBX OPTIONS!
		UFbxExportOption* Options = NewObject<UFbxExportOption>();
		Options->FbxExportCompatibility = EFbxExportCompatibility::FBX_2013;
		Options->bASCII = false;
		Options->bForceFrontXAxis = false;
		Options->VertexColor = true;
		Options->LevelOfDetail = true;
		Options->Collision = true;
		Options->bExportMorphTargets = true;
		Options->bExportPreviewMesh = false;
		Options->MapSkeletalMotionToRoot = false;
		Options->bExportLocalTime = true;

		//Link it to the task
		ExportTask->Options = Options;
	}

	bOutSuccess = UExporter::RunAssetExportTask(ExportTask);

	if(!bOutSuccess)
	{
		OutInfoMessage = FString::Printf(TEXT("Export Asset Failed - Errors: '%s"), *FString::Join(ExportTask->Errors, TEXT(", ")));
		return;
	}
	bOutSuccess = true;
	OutInfoMessage = FString::Printf(TEXT("Export Asset Successed"));
} */



#pragma endregion