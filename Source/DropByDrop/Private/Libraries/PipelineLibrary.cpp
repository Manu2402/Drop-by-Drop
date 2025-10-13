// © Manuel Solano
// © Roberto Capparelli

#include "Libraries/PipelineLibrary.h"

#include "WorldPartition/WorldPartitionSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/LandscapeInfoComponent.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Libraries/ErosionLibrary.h"
#include "DesktopPlatformModule.h"
#include "LandscapeImportHelper.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/SavePackage.h"
#include "LandscapeSubsystem.h"
#include "DropByDropLogger.h"
#include "ImageUtils.h"
#include "Landscape.h"

#define EMPTY_STRING ""
#define HEIGHTMAP_PATH_SUFFIX "Saved/HeightMap/raw.r16" 
#define HEIGHTMAP_ASSET_PREFIX "/DropByDrop/SavedAssets/"

#pragma region Erosion(Templates)

void UPipelineLibrary::GenerateErosion(TObjectPtr<ALandscape> ActiveLandscape, FErosionSettings& ErosionSettings)
{
	ULandscapeInfoComponent* ActiveLandscapeInfoComponent = ActiveLandscape->FindComponentByClass<ULandscapeInfoComponent>();
	if (!ActiveLandscapeInfoComponent)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("The \"Active Landscape\" resource is invalid!"));
		return;
	}

	FScopedSlowTask SlowTask(100, FText::FromString("Erosion in progress..."));
	SlowTask.MakeDialog(true);

	FErosionContext ErosionContext;
	UErosionLibrary::SetHeights(ErosionContext, ActiveLandscapeInfoComponent->GetHeightMapSettings().HeightMap);
	UErosionLibrary::Erosion(ErosionContext, ErosionSettings, ActiveLandscapeInfoComponent->GetHeightMapSettings().Size);

	SlowTask.EnterProgressFrame(50, FText::FromString("Applying on the landscape..."));

	TArray<uint16> ErodedHeightmapU16 = ConvertArrayFromFloatToUInt16(UErosionLibrary::GetHeights(ErosionContext));
	const FTransform LandscapeTransform = GetNewTransform(ActiveLandscapeInfoComponent->GetExternalSettings(), ActiveLandscapeInfoComponent->GetLandscapeSettings(), ActiveLandscapeInfoComponent->GetHeightMapSettings().Size);

	SlowTask.EnterProgressFrame(50);

	TObjectPtr<ALandscape> NewLandscape = GenerateLandscape(LandscapeTransform, ErodedHeightmapU16);

	if (!IsValid(NewLandscape))
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape!"));
		return;
	}

	ULandscapeInfoComponent* ErodedLandscapeInfoComponent = NewLandscape->FindComponentByClass<ULandscapeInfoComponent>();
	if (!ErodedLandscapeInfoComponent)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape!"));
		return;
	}

	ErodedLandscapeInfoComponent->SetIsEroded(true);
	ErodedLandscapeInfoComponent->SetExternalSettings(ActiveLandscapeInfoComponent->GetExternalSettings());
	ErodedLandscapeInfoComponent->SetHeightMapSettings(ActiveLandscapeInfoComponent->GetHeightMapSettings());
	ErodedLandscapeInfoComponent->SetLandscapeSettings(ActiveLandscapeInfoComponent->GetLandscapeSettings());

	UE_LOG(LogDropByDropLandscape, Log, TEXT("Landscape generated successfully!"));
}

bool UPipelineLibrary::SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue, const float InertiaValue, const int32 CapacityValue, const float MinSlopeValue, const float DepositionSpeedValue, const float ErosionSpeedValue, const int32 GravityValue, const float EvaporationValue, const int32 MaxPathValue, const int32 ErosionRadiusValue)
{
	FErosionTemplateRow ErosionTemplateRow;

	ErosionTemplateRow.ErosionCyclesField = ErosionCyclesValue;
	ErosionTemplateRow.InertiaField = InertiaValue;
	ErosionTemplateRow.CapacityField = CapacityValue;
	ErosionTemplateRow.MinimalSlopeField = MinSlopeValue;
	ErosionTemplateRow.DepositionSpeedField = DepositionSpeedValue;
	ErosionTemplateRow.ErosionSpeedField = ErosionSpeedValue;
	ErosionTemplateRow.GravityField = GravityValue;
	ErosionTemplateRow.EvaporationField = EvaporationValue;
	ErosionTemplateRow.MaxPathField = MaxPathValue;
	ErosionTemplateRow.ErosionRadiusField = ErosionRadiusValue;

	UDataTable* ErosionTemplatesDT = FDropByDropSettings::Get().GetErosionTemplatesDT();
	if (!ErosionTemplatesDT)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Templates DataTable\" resource is invalid!"));
		return false;
	}

	ErosionTemplatesDT->AddRow(FName(TemplateName), ErosionTemplateRow);

	return SaveErosionTemplates(ErosionTemplatesDT);
}

bool UPipelineLibrary::SaveErosionTemplates(UDataTable* ErosionTemplatesDT)
{
	UPackage* ErosionTemplatesPackage = ErosionTemplatesDT->GetPackage();
	if (!ErosionTemplatesPackage)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Templates Package\" resource is invalid!"));
		return false;
	}

	ErosionTemplatesPackage->MarkPackageDirty();

	return GEditor->GetEditorSubsystem<UEditorAssetSubsystem>()->SaveLoadedAsset(ErosionTemplatesDT);
}

FErosionTemplateRow* UPipelineLibrary::LoadErosionTemplate(const FString& RowName)
{
	UDataTable* ErosionTemplatesDT = FDropByDropSettings::Get().GetErosionTemplatesDT();
	if (!ErosionTemplatesDT)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Templates DataTable\" resource is invalid!"));
		return nullptr;
	}

	const FString ContextString(TEXT("Erosion Template Context"));

	FErosionTemplateRow* RowData = ErosionTemplatesDT->FindRow<FErosionTemplateRow>(FName(RowName), ContextString);
	if (!RowData)
	{
		return nullptr;
	}

	return RowData;
}

void UPipelineLibrary::LoadRowIntoErosionFields(TSharedPtr<FErosionSettings>& OutErosionSettings, const FErosionTemplateRow* TemplateDatas)
{
	if (!TemplateDatas)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Template Datas\" resource is invalid!"));
		return;
	}

	OutErosionSettings->ErosionCycles = TemplateDatas->ErosionCyclesField;
	OutErosionSettings->WindDirection = TemplateDatas->WindDirection;
	OutErosionSettings->Inertia = TemplateDatas->InertiaField;
	OutErosionSettings->Capacity = TemplateDatas->CapacityField;
	OutErosionSettings->MinimalSlope = TemplateDatas->MinimalSlopeField;
	OutErosionSettings->DepositionSpeed = TemplateDatas->DepositionSpeedField;
	OutErosionSettings->ErosionSpeed = TemplateDatas->ErosionSpeedField;
	OutErosionSettings->Gravity = TemplateDatas->GravityField;
	OutErosionSettings->Evaporation = TemplateDatas->EvaporationField;
	OutErosionSettings->MaxPath = TemplateDatas->MaxPathField;
	OutErosionSettings->ErosionRadius = TemplateDatas->ErosionRadiusField;
}

bool UPipelineLibrary::DeleteErosionTemplate(const FString& TemplateName)
{
	UDataTable* ErosionTemplatesDT = FDropByDropSettings::Get().GetErosionTemplatesDT();

	if (!ErosionTemplatesDT)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Templates DataTable\" resource is invalid!"));
		return false;
	}

	ErosionTemplatesDT->RemoveRow(FName(TemplateName));

	return SaveErosionTemplates(ErosionTemplatesDT);
}

#pragma endregion

#pragma region HeightMap

bool UPipelineLibrary::CreateAndSaveHeightMap(FHeightMapGenerationSettings& Settings)
{
	Settings.HeightMap = CreateHeightMapArray(Settings);
	UTexture2D* Texture = CreateHeightMapTexture(Settings.HeightMap, Settings.Size, Settings.Size);

	if (!Texture)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to create the heightmap texture!"));
		return false;
	}

	if (SaveToAsset(Texture, "TextureHeightMap"))
	{
		UE_LOG(LogDropByDropHeightmap, Log, TEXT("Heightmap texture saved successfully!"));
		return true;
	}

	UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to save the heightmap texture!"));

	return false;
}

TArray<float> UPipelineLibrary::CreateHeightMapArray(const FHeightMapGenerationSettings& Settings)
{
	const int32 MapSize = Settings.Size;

	TArray<float> HeightMapValues;
	HeightMapValues.SetNum(MapSize * MapSize);

	const int32 CurrentSeed = Settings.bRandomizeSeed ? FMath::RandRange(-10000, 10000) : Settings.Seed;
	const FRandomStream RandomStream(CurrentSeed);

	// Random offset for each octave.
	TArray<FVector2D> Offsets;
	Offsets.SetNum(Settings.NumOctaves);

	for (uint32 Index = 0; Index < Settings.NumOctaves; Index++)
	{
		Offsets[Index] = FVector2D(RandomStream.FRandRange(-1000.0f, 1000.0f), RandomStream.FRandRange(-1000.0f, 1000.0f));
	}

	float MinValue = TNumericLimits<float>::Max();
	float MaxValue = TNumericLimits<float>::Lowest();

	// Generate Perlin Noise for each point (Width, Height).
	for (int32 Height = 0; Height < MapSize; Height++)
	{
		for (int32 Width = 0; Width < MapSize; Width++)
		{
			float NoiseValue = 0.0f;
			float Scale = Settings.InitialScale;
			float Weight = 1.0f;

			for (uint32 OctaveIndex = 0; OctaveIndex < Settings.NumOctaves; OctaveIndex++)
			{
				FVector2D Location = Offsets[OctaveIndex] + FVector2D(Width, Height) / static_cast<float>(MapSize) * Scale;
				NoiseValue += FMath::PerlinNoise2D(Location) * Weight;

				Weight *= Settings.Persistence;
				Scale *= Settings.Lacunarity;
			}

			const int32 Index = Height * MapSize + Width;
			HeightMapValues[Index] = NoiseValue;

			MinValue = FMath::Min(MinValue, NoiseValue);
			MaxValue = FMath::Max(MaxValue, NoiseValue);
		}
	}

	// Normalizing in range [0, MaxHeightDifference].
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

UTexture2D* UPipelineLibrary::CreateHeightMapTexture(const TArray<float>& HeightMapData, const int32 Width, const int32 Height)
{
	UTexture2D* Texture = NewObject<UTexture2D>(GetTransientPackage(), UTexture2D::StaticClass(), FName(*FString::Printf(TEXT("GeneratedTexture_%d"), FMath::Rand())), RF_Public | RF_Standalone);

	if (!Texture)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to create texture!"));
		return nullptr;
	}

	// Create and set the "PlatformData".
	FTexturePlatformData* PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = Width;
	PlatformData->SizeY = Height;
	PlatformData->PixelFormat = PF_B8G8R8A8; // Only PF_B8G8R8A8 format is supported.

	//Create "MipMap".
	FTexture2DMipMap* MipMap = new FTexture2DMipMap();
	MipMap->SizeX = Width;
	MipMap->SizeY = Height;
	PlatformData->Mips.Add(MipMap);

	// Allocate memory for create texture (without using Python this part of the code is essential for write a texture).
	MipMap->BulkData.Lock(LOCK_READ_WRITE);
	uint8* MipData = MipMap->BulkData.Realloc(Width * Height * 4);
	FMemory::Memzero(MipData, Width * Height * 4); // Clear initial data.

	// Writing pixel for the texture. 
	for (int32 Index = 0; Index < HeightMapData.Num(); Index++)
	{
		const uint8 GrayValue = FMath::Clamp(static_cast<uint8>(HeightMapData[Index] * 255.0f), 0, 255);
		MipData[Index * 4 + 0] = GrayValue; // R
		MipData[Index * 4 + 1] = GrayValue; // G
		MipData[Index * 4 + 2] = GrayValue; // B
		MipData[Index * 4 + 3] = 255; // A
	}

	MipMap->BulkData.Unlock();

	// Set "PlatformData" to texture.
	Texture->SetPlatformData(PlatformData);

	// Compressing and set "MipMap".
	Texture->SRGB = true;
	Texture->CompressionSettings = TC_Default;
	Texture->MipGenSettings = TMGS_FromTextureGroup;
	Texture->UpdateResource();

	return Texture;
}

bool UPipelineLibrary::OpenHeightmapFileDialog(TSharedPtr<FExternalHeightMapSettings> ExternalSettings)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Failed to open file dialog!"));
		return false;
	}

	TArray<FString> OutFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	bool bOpened = DesktopPlatform->OpenFileDialog(ParentWindowHandle, TEXT("Select Heightmap (PNG)"), FPaths::ProjectDir(), TEXT(EMPTY_STRING), TEXT("PNG files (*.png)|*.png"), EFileDialogFlags::None, OutFiles);

	if (!bOpened || OutFiles.Num() <= 0)
	{
		return false;
	}

	// First result.
	const FString& SelectedFile = OutFiles[0];

	if (UTexture2D* Imported = FImageUtils::ImportFileAsTexture2D(SelectedFile))
	{
		if (!Imported)
		{
			UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Failed to overwrite preview from PNG: %s"), *SelectedFile);
			return false;
		}

		if (!SaveToAsset(Imported, TEXT("TextureHeightMap")))
		{
			UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Failed to overwrite preview from PNG: %s"), *SelectedFile);
			return false;
		}

		UE_LOG(LogDropByDropHeightmap, Log, TEXT("Preview overwritten from external PNG: %s"), *SelectedFile);

		ExternalSettings->bIsExternalHeightMap = true;
		ExternalSettings->LastPNGPath = SelectedFile;

		return true;
	}

	return false;
}

void UPipelineLibrary::LoadHeightmapFromFileSystem(const FString& FilePath, TArray<uint16>& OutHeightMap, TArray<float>& OutNormalizedHeightmap, FExternalHeightMapSettings& Settings)
{
	OutHeightMap.Empty();
	OutNormalizedHeightmap.Empty();

	// Get Texture from "FilePath".
	UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
	if (!Texture)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to load PNG file: %s"), *FilePath);
		return;
	}

	// Get data from "Texture".
	FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
	const void* MipData = MipMap.BulkData.LockReadOnly();
	int32 Width = MipMap.SizeX;
	int32 Height = MipMap.SizeY;

	UE_LOG(LogDropByDropHeightmap, Log, TEXT("Loaded Texture from PNG: Width = %d, Height = %d"), Width, Height);

	uint32 MinPixel = TNumericLimits<uint32>::Max();
	uint32 MaxPixel = TNumericLimits<uint32>::Min();

	OutHeightMap.SetNum(Width * Height);
	OutNormalizedHeightmap.SetNum(Width * Height);

	// Get format and data.
	switch (Texture->GetPixelFormat())
	{
		case PF_R8G8B8A8:
		{
			const uint8* PixelData = static_cast<const uint8*>(MipData);
			for (int32 Index = 0; Index < Width * Height; Index++)
			{
				uint8 RedValue = PixelData[Index * 4];
				uint16 HeightValue = static_cast<uint16>(RedValue << 8); // Scale to 16 bit.

				OutHeightMap[Index] = HeightValue;

				MinPixel = FMath::Min<uint32>(MinPixel, HeightValue);
				MaxPixel = FMath::Max<uint32>(MaxPixel, HeightValue);
			}
			
			break;
		}
		case PF_G16:
		{
			const uint16* PixelData = static_cast<const uint16*>(MipData);

			for (int32 Index = 0; Index < Width * Height; Index++)
			{
				OutHeightMap[Index] = PixelData[Index];

				MinPixel = FMath::Min<uint32>(MinPixel, OutHeightMap[Index]);
				MaxPixel = FMath::Max<uint32>(MaxPixel, OutHeightMap[Index]);
			}

			break;
		}
		case PF_R32_FLOAT:
		{
			const float* PixelData = static_cast<const float*>(MipData);

			for (int32 Index = 0; Index < Width * Height; Index++)
			{
				uint16 HeightValue = static_cast<uint16>(PixelData[Index] * 65535.0f);

				OutHeightMap[Index] = HeightValue;

				MinPixel = FMath::Min<uint32>(MinPixel, HeightValue);
				MaxPixel = FMath::Max<uint32>(MaxPixel, HeightValue);
			}

			break;
		}
		default:
		{
			UE_LOG(LogDropByDropHeightmap, Error, TEXT("Unsupported pixel format in PNG!"));
			MipMap.BulkData.Unlock();

			return;
		}
	}

	MipMap.BulkData.Unlock();

	// Normalization float from 0 to 1.
	for (int32 Index = 0; Index < OutHeightMap.Num(); Index++)
	{
		OutNormalizedHeightmap[Index] = (MaxPixel > MinPixel) ? (OutHeightMap[Index] - MinPixel) / static_cast<float>(MaxPixel - MinPixel) : 0.0f;
	}

	Settings.bIsExternalHeightMap = true;

	UE_LOG(LogDropByDropHeightmap, Log, TEXT("Heightmap Min: %u, Max: %u"), MinPixel, MaxPixel);
}

void UPipelineLibrary::CompareHeightmaps(const FString& RawFilePath, const TArray<uint16>& GeneratedHeightmap, int32 Width, int32 Height)
{
	// Get RAW data from Unreal.
	TArray<uint8> RawData;
	if (!FFileHelper::LoadFileToArray(RawData, *RawFilePath))
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to load RAW file at: %s"), *RawFilePath);
		return;
	}

	// Let's make sure that the size is Width * Height * 2 (uint16 = 2 bytes).
	const uint64 ExpectedSize = static_cast<uint64>(Width) * static_cast<uint64>(Height) * sizeof(uint16);
	if (RawData.Num() != ExpectedSize)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("RAW file size doesn't match the expected size: %llu VS %llu"), static_cast<uint64>(RawData.Num()), ExpectedSize); 
		return;
	}

	// Convert TArray<uint8> to TArray<uint16>.
	TArray<uint16> UnrealHeightmap;
	UnrealHeightmap.SetNum(Width * Height);
	FMemory::Memcpy(UnrealHeightmap.GetData(), RawData.GetData(), RawData.Num());

	// Compare the values.
	for (int32 Index = 0; Index < FMath::Min(10, UnrealHeightmap.Num()); Index++)
	{
		UE_LOG(LogDropByDropHeightmap, Log, TEXT("RAW[%d]: %d, Generated[%d]: %d"), Index, UnrealHeightmap[Index], Index, GeneratedHeightmap[Index]);
	}
}

#pragma endregion

#pragma region Landscape

bool UPipelineLibrary::InitLandscape(TArray<uint16>& HeightData, FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings)
{
	const FTransform LandscapeTransform = GetNewTransform(ExternalSettings, LandscapeSettings, HeightmapSettings.Size);

	TObjectPtr<ALandscape> NewLandscape = GenerateLandscape(LandscapeTransform, HeightData);
	if (!IsValid(NewLandscape))
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape!"));
		return false;
	}

	ULandscapeInfoComponent* NewLandscapeInfo = NewLandscape->FindComponentByClass<class ULandscapeInfoComponent>();
	if (!NewLandscapeInfo)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape!"));
		return false;
	}

	NewLandscapeInfo->SetHeightMapSettings(HeightmapSettings);
	NewLandscapeInfo->SetExternalSettings(ExternalSettings);
	NewLandscapeInfo->SetLandscapeSettings(LandscapeSettings);

	UE_LOG(LogDropByDropLandscape, Log, TEXT("Landscape created successfully!"));

	return true;
}

void UPipelineLibrary::CreateLandscapeFromInternalHeightMap(FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings)
{
	ExternalSettings.bIsExternalHeightMap = false;

	if (HeightmapSettings.HeightMap.Num() <= 0)
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
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape! \"World\" resource is invalid!"));
		return;
	}

	// From float to uint16 (Unreal requisition).
	TArray<uint16> HeightData = ConvertArrayFromFloatToUInt16(HeightmapSettings.HeightMap);
	InitLandscape(HeightData, HeightmapSettings, ExternalSettings, LandscapeSettings);
}

void UPipelineLibrary::CreateLandscapeFromExternalHeightMap(const FString& FilePath, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings, FHeightMapGenerationSettings& HeightmapSettings)
{
	ExternalSettings.bIsExternalHeightMap = true;
	ExternalSettings.LastPNGPath = FilePath;

	// Load "HeightMap" from External PNG.
	TArray<uint16> HeightMapInt16;
	LoadHeightmapFromFileSystem(FilePath, HeightMapInt16, HeightmapSettings.HeightMap, ExternalSettings);

	FString HeightMapPath = FPaths::ProjectDir() + TEXT(HEIGHTMAP_PATH_SUFFIX);
	CompareHeightmaps(HeightMapPath, HeightMapInt16, 505, 505);

	if (HeightMapInt16.Num() <= 0)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to load heightmap from PNG file!"));
		return;
	}

	InitLandscape(HeightMapInt16, HeightmapSettings, ExternalSettings, LandscapeSettings);
}

void UPipelineLibrary::GenerateLandscapeAuto(FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings)
{
	if (ExternalSettings.bIsExternalHeightMap && !ExternalSettings.LastPNGPath.IsEmpty())
	{
		CreateLandscapeFromExternalHeightMap(ExternalSettings.LastPNGPath, ExternalSettings, LandscapeSettings, HeightmapSettings);
		return;
	}

	CreateLandscapeFromInternalHeightMap(HeightmapSettings, ExternalSettings, LandscapeSettings);
}

TObjectPtr<ALandscape> UPipelineLibrary::GenerateLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap)
{
	int32 SubSectionSizeQuads;
	int32 NumSubsections;
	int32 MaxX, MaxY;

	if (!SetLandscapeSizeParam(SubSectionSizeQuads, NumSubsections, MaxX, MaxY))
	{
		return nullptr;
	}

	TMap<FGuid, TArray<uint16>> HeightDataPerLayer;
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;
	TArray<FLandscapeImportLayerInfo> MaterialImportLayers;
	MaterialImportLayers.Reserve(0);
	MaterialLayerDataPerLayer.Add(FGuid(), MoveTemp(MaterialImportLayers));

	if (Heightmap.Num() != MaxX * MaxY)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Heightmap dimensions don't match the generated landscape: %d != %d x %d"), Heightmap.Num(), MaxX, MaxY);
		return nullptr;
	}

	HeightDataPerLayer.Add(FGuid(), MoveTemp(Heightmap));

	const FWorldContext& EditorWorldContext = GEditor->GetEditorWorldContext();
	UWorld* World = EditorWorldContext.World();

	if (!World)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape! \"World\" resource is invalid!"));
		return nullptr;
	}

	ALandscape* Landscape = World->SpawnActor<ALandscape>();

	ULandscapeInfoComponent* LandscapeInfo = NewObject<ULandscapeInfoComponent>(Landscape);
	LandscapeInfo->RegisterComponent();
	Landscape->AddInstanceComponent(LandscapeInfo);

	Landscape->bCanHaveLayersContent = true;
	Landscape->LandscapeMaterial = nullptr;
	Landscape->SetActorTransform(LandscapeTransform);

	UE_LOG(LogDropByDropLandscape, Error, TEXT("NumSubSections: %d; SubSectionSizeQuads: %d; MaxY: %d"), NumSubsections, SubSectionSizeQuads, MaxY);

	TArray<FLandscapeLayer> LandscapeLayers;
	Landscape->Import(FGuid::NewGuid(), 0, 0, MaxX - 1, MaxY - 1, NumSubsections, SubSectionSizeQuads, HeightDataPerLayer, nullptr, MaterialLayerDataPerLayer, ELandscapeImportAlphamapType::Additive, MakeArrayView(LandscapeLayers));

	Landscape->PostEditChange();

	return Landscape;
}

void UPipelineLibrary::SplitLandscapeIntoProxies(ALandscape& ActiveLandscape)
{
	ULandscapeInfo* LandscapeInfo = ActiveLandscape.GetLandscapeInfo();

	if (!LandscapeInfo)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to split the landscape! \"LandscapeInfo\" resource is invalid!"));
		return;
	}

	LandscapeInfo->UpdateLayerInfoMap(&ActiveLandscape);
	ActiveLandscape.RegisterAllComponents();

	FScopedSlowTask SlowTask(100, FText::FromString("Splitting Landscape..."));
	SlowTask.MakeDialog(true);

	ULandscapeInfoComponent* LandscapeInfoComponent = nullptr;
	UWorld* World = GEditor->GetEditorWorldContext().World();

	if (World)
	{
		if (ULandscapeSubsystem* LandscapeSubsystem = World->GetSubsystem<ULandscapeSubsystem>())
		{
			LandscapeInfoComponent = ActiveLandscape.FindComponentByClass<ULandscapeInfoComponent>();
			LandscapeSubsystem->ChangeGridSize(LandscapeInfo, LandscapeInfoComponent->GetLandscapeSettings().WorldPartitionCellSize);
		}
	}

	SlowTask.EnterProgressFrame(100);

	ActiveLandscape.PostEditChange();

	if (World)
	{
		if (UWorldPartitionSubsystem* WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>())
		{
			if (IStreamingWorldSubsystemInterface* StreamingWorldPartitionSubsystemInterface = Cast<IStreamingWorldSubsystemInterface>(WorldPartitionSubsystem))
			{
				StreamingWorldPartitionSubsystemInterface->OnUpdateStreamingState();
				GEditor->RedrawAllViewports();
			}
		}
		
	}

	if (!LandscapeInfoComponent)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to split the landscape!"));
		return;
	}

	LandscapeInfoComponent->SetIsSplittedIntoProxies(true);
}

bool UPipelineLibrary::SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX, int32& MaxY, const int32 Size)
{
	const int32 HeightmapSize = Size;
	FIntPoint NewLandscapeComponentCount;
	FLandscapeImportHelper::ChooseBestComponentSizeForImport(HeightmapSize, HeightmapSize, SubSectionSizeQuads, NumSubsections, NewLandscapeComponentCount);

	const int32 ComponentCountX = NewLandscapeComponentCount.X;
	const int32 ComponentCountY = NewLandscapeComponentCount.Y;
	const int32 QuadsPerComponent = NumSubsections * SubSectionSizeQuads;
	MaxX = ComponentCountX * QuadsPerComponent + 1;
	MaxY = ComponentCountY * QuadsPerComponent + 1;

	if (MaxX != HeightmapSize || MaxY != HeightmapSize)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Size calculated (%d x %d) not match with size of HeightMap (%d x %d)"))
		return false;
	}

	return true;
}

#pragma endregion

#pragma region Utilities
FTransform UPipelineLibrary::GetNewTransform(const FExternalHeightMapSettings& ExternalSettings, const FLandscapeGenerationSettings& LandscapeSettings, int32 HeightmapSize)
{
	FVector NewScale;

	if (ExternalSettings.bIsExternalHeightMap)
	{
		NewScale = FVector(ExternalSettings.ScalingX, ExternalSettings.ScalingY, ExternalSettings.ScalingZ);
	}
	else
	{
		const FVector BaseScale = FVector(100, 100, 100);
		const float CurrentSizeInUnits = HeightmapSize * BaseScale.X;
		const float DesiredSizeInUnits = LandscapeSettings.Kilometers * 1000.f * 100.f;
		const float ScaleFactor = DesiredSizeInUnits / CurrentSizeInUnits;
		NewScale = BaseScale * ScaleFactor;
	}

	return FTransform(FQuat::Identity, FVector(-100800, -100800, 17200), NewScale);
}

TArray<uint16> UPipelineLibrary::ConvertArrayFromFloatToUInt16(const TArray<float>& FloatData)
{
	TArray<uint16> UInt16Data;
	UInt16Data.AddUninitialized(FloatData.Num());

	for (int32 Index = 0; Index < FloatData.Num(); Index++)
	{
		// Scaling value from [0, 1] to [0, 65535].
		UInt16Data[Index] = FMath::Clamp<uint16>(FMath::RoundToInt(FloatData[Index] * 65535.0f), 0, 65535);
	}

	return UInt16Data;
}

bool UPipelineLibrary::SaveToAsset(UTexture2D* Texture, const FString& AssetName)
{
	if (!Texture)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"Texture\" resource is invalid!"));
		return false;
	}

	auto ApplyPreviewTextureFlags = [](UTexture2D* T)
	{
		T->SRGB = true;
		// Use an un-intrusive compression setting for UI preview (no BC artifacts).
		T->CompressionSettings = TC_Default; // Very light or no compression for UI icons.
		T->MipGenSettings = TMGS_NoMipmaps;
		T->LODGroup = TEXTUREGROUP_Pixels2D;
		T->NeverStream = true;
		T->AddressX = TA_Clamp;
		T->AddressY = TA_Clamp;
	};

	// 1) Read raw pixels from source (preferred) or platform mip[0]. We'll convert them to an 8-bit Gray buffer.
	TArray64<uint8> Raw;
	int32 Width = 0, Height = 0;
	ERawFormat InFormat = ERawFormat::Unknown;

	// Try "Texture Source" first.
	const int32 SrcWidth = Texture->Source.GetSizeX();
	const int32 SrcHeight = Texture->Source.GetSizeY();

	if (SrcWidth > 0 && SrcHeight > 0 && Texture->Source.GetNumMips() > 0)
	{
		bool bOk = Texture->Source.GetMipData(Raw, 0);

		if (!bOk || Raw.Num() <= 0)
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
			Width = SrcWidth;
			Height = SrcHeight;

			switch (Texture->Source.GetFormat())
			{
				case TSF_BGRA8: 
					InFormat = ERawFormat::BGRA8;
					break;
				case TSF_G8: 
					InFormat = ERawFormat::G8;
					break;
				case TSF_G16: 
					InFormat = ERawFormat::G16;
					break;
				case TSF_RGBA16F: 
					InFormat = ERawFormat::RGBA16F;
					break;
				default: 
					InFormat = ERawFormat::Unknown;
					break;
			}
		}
	}

	int64 NumPixels = 0;

	// Fallback: "PlatformData".
	if (InFormat == ERawFormat::Unknown)
	{
		FTexturePlatformData* PlatformData = Texture->GetPlatformData();
		if (!PlatformData || PlatformData->Mips.Num() <= 0)
		{
			UE_LOG(LogDropByDropHeightmap, Error, TEXT("No \"Texture Source\" and no \"Platform Data\" available."));
			return false;
		}

		FTexture2DMipMap& Mip = PlatformData->Mips[0];
		const int64 DataSize = Mip.BulkData.GetBulkDataSize();
		if (DataSize <= 0)
		{
			UE_LOG(LogDropByDropHeightmap, Error, TEXT("\"Platform Mip\" has no data."));
			return false;
		}

		const void* MipData = Mip.BulkData.LockReadOnly();
		if (!MipData)
		{
			Mip.BulkData.Unlock();

			UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to lock \"Platform Mip Data\"."));
			return false;
		}

		Width = Mip.SizeX;
		Height = Mip.SizeY;

		NumPixels = static_cast<int64>(Width) * Height;
		const EPixelFormat PixelFormat = Texture->GetPixelFormat();

		switch (PixelFormat)
		{
			case PF_B8G8R8A8:
			case PF_R8G8B8A8:
			{
				InFormat = ERawFormat::BGRA8;
				Raw.SetNumUninitialized(NumPixels * 4);

				break;
			}
			case PF_G16:
			{
				InFormat = ERawFormat::G16;
				Raw.SetNumUninitialized(NumPixels * 2);

				break;
			}
			case PF_R32_FLOAT:
			{
				InFormat = ERawFormat::R32F;
				Raw.SetNumUninitialized(NumPixels * 4);

				break;
			}
			default:
			{
				InFormat = ERawFormat::Unknown;
				break;
			}
		}

		if (InFormat != ERawFormat::Unknown)
		{
			FMemory::Memcpy(Raw.GetData(), MipData, Raw.Num());
		}

		Mip.BulkData.Unlock();
	}

	if (InFormat == ERawFormat::Unknown || Raw.Num() <= 0 || Width <= 0 || Height <= 0)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Unsupported or empty source format when saving preview asset!"));
		return false;
	}

	// 2) Convert ANY input to Gray (8-bit) then replicate Gray to BGRA (B = G = R = Gray, A = 255).
	TArray64<uint8> Greys;
	Greys.SetNumUninitialized(NumPixels);

	switch (InFormat)
	{
		case ERawFormat::BGRA8:
		{
			const uint8* Ptr = Raw.GetData();
			for (int64 Index = 0; Index < NumPixels; Index++)
			{
				// BGRA: B = 0, G = 1, R = 2, A = 3 
				Greys[Index] = Ptr[Index * 4 + 2];
			}

			break;
		}
		case ERawFormat::RGBA8:
		{
			const uint8* Ptr = Raw.GetData();
			for (int64 Index = 0; Index < NumPixels; Index++)
			{
				// RGBA: R = 0, G = 1, B = 2, A = 3
				Greys[Index] = Ptr[Index * 4 + 0];
			}

			break;
		}
		case ERawFormat::G8:
		{
			FMemory::Memcpy(Greys.GetData(), Raw.GetData(), Greys.Num());

			break;
		}
		case ERawFormat::G16:
		{
			const uint16* Ptr = reinterpret_cast<const uint16*>(Raw.GetData());
			for (int64 Index = 0; Index < NumPixels; Index++)
			{
				Greys[Index] = static_cast<uint8>(Ptr[Index] >> 8); // Keeping MSB.
			}

			break;
		}
		case ERawFormat::R32F:
		{
			const float* Ptr = reinterpret_cast<const float*>(Raw.GetData());
			for (int64 Index = 0; Index < NumPixels; Index++)
			{
				const float Value = FMath::Clamp(Ptr[Index], 0.f, 1.f);
				Greys[Index] = static_cast<uint8>(FMath::RoundToInt(Value * 255.f));
			}

			break;
		}
		case ERawFormat::RGBA16F:
		{
			const FFloat16Color* Ptr = reinterpret_cast<const FFloat16Color*>(Raw.GetData());
			for (int64 Index = 0; Index < NumPixels; Index++)
			{
				const float Red = FMath::Clamp(Ptr[Index].R, 0.f, 1.f);
				Greys[Index] = static_cast<uint8>(FMath::RoundToInt(Red * 255.f));
			}

			break;
		}
		default:
		{
			UE_LOG(LogDropByDropHeightmap, Error, TEXT("Unexpected raw format during grayscale conversion!"));
			return false;
		}
	}

	// Build BGRA8 from Gray (B = G = R = Gray, A = 255) so Slate sees neutral grayscale.
	TArray64<uint8> BGRA;
	BGRA.SetNumUninitialized(NumPixels * 4);

	for (int64 Index = 0; Index < NumPixels; Index++)
	{
		const uint8 Gray = Greys[Index];
		const int64 Offset = Index * 4;
		BGRA[Offset] = Gray; // B
		BGRA[Offset + 1] = Gray; // G
		BGRA[Offset + 2] = Gray; // R
		BGRA[Offset + 3] = 255; // A
	}

	// 3) Create or update the asset as BGRA8 (with preview flags).
	const FString PackageName = FString::Printf(TEXT("/DropByDrop/SavedAssets/%s"), *AssetName);

	UPackage* ExistingPackage = FindPackage(nullptr, *PackageName);
	UTexture2D* TargetTexture = ExistingPackage ? Cast<UTexture2D>(StaticFindObject(UTexture2D::StaticClass(), ExistingPackage, *AssetName)) : nullptr;

	auto SaveAndRescan = [&](UPackage* Package, UObject* Asset)
	{
		FlushRenderingCommands();

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

		if (!UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs))
		{
			UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to save package \"%s\""), *PackageName);
			return false;
		}

		if (FAssetRegistryModule* ARM = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry"))
		{
			TArray<FString> PathsToScan = { TEXT(HEIGHTMAP_ASSET_PREFIX) };
			ARM->Get().ScanModifiedAssetFiles(PathsToScan);
		}
		else
		{
			UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Failed to save texture to: %s"), *PackageFilename);
			return false;
		}

		UE_LOG(LogDropByDropHeightmap, Log, TEXT("Saved texture to: %s"), *PackageFilename);
		return true;
	};

	auto WriteBGRA8ToTexture = [&](UTexture2D* T)
	{
		T->Modify();

		// Bake as BGRA8 so Slate always sees grayscale (R = G = B) instead of a red-only channel.
		T->Source.Init(Width, Height, 1, 1, TSF_BGRA8);
		void* Dest = T->Source.LockMip(0);
		const int64 DestSize = NumPixels * 4;
		FMemory::Memcpy(Dest, BGRA.GetData(), DestSize);
		T->Source.UnlockMip(0);

		ApplyPreviewTextureFlags(T);
		T->PostEditChange();
		T->UpdateResource();
		static_cast<void>(T->MarkPackageDirty());
	};

	if (TargetTexture)
	{
		UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Texture '%s' already exists, updating asset..."), *AssetName);
		
		WriteBGRA8ToTexture(TargetTexture);
		return SaveAndRescan(TargetTexture->GetOutermost(), TargetTexture);
	}
	else
	{
		UPackage* NewPackage = CreatePackage(*PackageName);

		if (!NewPackage)
		{
			UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to create package \"%s\""), *PackageName);
			return false;
		}

		NewPackage->FullyLoad();

		UTexture2D* NewTexture = NewObject<UTexture2D>(NewPackage, UTexture2D::StaticClass(), *AssetName, RF_Public | RF_Standalone);

		if (!NewTexture)
		{
			UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to create texture in package \"%s\""), *PackageName);
			return false;
		}

		WriteBGRA8ToTexture(NewTexture);
		FAssetRegistryModule::AssetCreated(NewTexture);

		return SaveAndRescan(NewPackage, NewTexture);
	}
}

FVector UPipelineLibrary::GetWindPreviewStart(const ALandscape* ActiveLandscape, UWorld* World)
{
	FBox Bounds(ForceInitToZero);
	TArray<ULandscapeComponent*> Components;
	ActiveLandscape->GetComponents(Components);

	for (ULandscapeComponent* Component : Components)
	{
		if (!Component)
		{
			continue;
		}

		Bounds += Component->Bounds.GetBox();
	}

	if (Bounds.IsValid)
	{
		FVector Center = Bounds.GetCenter();
		Center.Z += 200.f;

		return Center;
	}

	FVector Location = ActiveLandscape->GetActorLocation();
	Location.Z += 200.f;

	return Location;
}

float UPipelineLibrary::GetWindPreviewScale()
{
	return FDropByDropSettings::Get().GetWindPreviewScale();
}

void UPipelineLibrary::SetWindPreviewScale(const float NewScale)
{
	FDropByDropSettings::Get().SetWindPreviewScale(NewScale);
}

void UPipelineLibrary::DrawWindDirectionPreview(const FErosionSettings& ErosionSettings, const ALandscape* ActiveLandscape, float ArrowLength, float ArrowThickness, float ArrowHeadSize, float Duration, bool  bAlsoDrawCone, float ConeHalfAngleDeg)
{
#if WITH_EDITOR
	const FVector LandscapeScale = ActiveLandscape->GetActorScale3D();
	const float AverageLandscapeScale = ((LandscapeScale.X / 198.f /* 1 KM ~= 198 CM */ ) + (LandscapeScale.Y / 198.f /* 1 KM ~= 198 CM */ )) * 0.5f;
	const float Scale = FDropByDropSettings::Get().GetWindPreviewScale() * AverageLandscapeScale;

	UWorld* World = nullptr;

	if (GEditor)
	{
		const FWorldContext& Context = GEditor->GetEditorWorldContext();
		World = Context.World();
	}
	
	if (!World)
	{
		UE_LOG(LogDropByDropErosion, Error, TEXT("Failed to draw wind preview! \"World\" resource is invalid!"));
		return;
	}

	const FVector Start = GetWindPreviewStart(ActiveLandscape, World);
	float MeanDeg = 0.f;

	if (!UErosionLibrary::TryGetWindMeanAngleDegrees(ErosionSettings, MeanDeg))
	{
		return;
	}

	FVector2D Direction2D = UErosionLibrary::GetWindUnitVectorFromAngle(MeanDeg);
	FVector Direction3D(Direction2D.X, Direction2D.Y, 0.f);

	const FRotator LandscapeRotation = ActiveLandscape->GetActorRotation();
	const FQuat LandscapeQuat = FQuat(FRotator(0.f, LandscapeRotation.Yaw, 0.f));
	Direction3D = LandscapeQuat.RotateVector(Direction3D);

	const FVector End = Start + Direction3D * (ArrowLength * Scale);

	const FVector ArrowBodyEnd = Start + Direction3D * (ArrowLength * Scale * 0.6f);
	DrawDebugLine(World, Start, ArrowBodyEnd, FColor::Cyan, false, Duration, 0, ArrowThickness * Scale * 3.f);

	const float HugeHeadSize = ArrowHeadSize * Scale * 8.f;
	const FVector ConeStart = End - (Direction3D * HugeHeadSize);

	DrawDebugCone(World, ConeStart, Direction3D, HugeHeadSize, FMath::DegreesToRadians(30.f), FMath::DegreesToRadians(30.f), 24, FColor::Yellow, false, Duration, 0, 4.f * Scale);
	DrawDebugCone(World, ConeStart - Direction3D * (5.f * Scale), Direction3D, HugeHeadSize * 1.1f, FMath::DegreesToRadians(32.f), FMath::DegreesToRadians(32.f), 24, FColor::Black, false, Duration, 1, 2.f * Scale);
	DrawDebugCone(World, ConeStart, Direction3D, HugeHeadSize * 0.7f, FMath::DegreesToRadians(25.f), FMath::DegreesToRadians(25.f), 16, FColor::Red, false, Duration, 2, 3.f * Scale);

	DrawDebugSphere(World, End, 80.f * Scale, 16, FColor::Orange, false, Duration, 0, 5.f * Scale);
	DrawDebugSphere(World, End, 50.f * Scale, 12, FColor::Red, false, Duration, 1, 4.f * Scale);

	for (int32 Index = 0; Index < 8; Index++)
	{
		const float AngleDeg = (360.f / 8.f) * Index;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);
		const FVector Offset(FMath::Cos(AngleRad) * 100.f * Scale, FMath::Sin(AngleRad) * 100.f * Scale, 0.f);

		DrawDebugLine(World, End + Offset, End, FColor::Yellow, false, Duration, 0, 2.f * Scale);
	}

	const float Cross = ArrowLength * 0.25f * Scale;
	FVector CrossX(Cross, 0, 0);
	FVector CrossY(0, Cross, 0);

	CrossX = LandscapeQuat.RotateVector(CrossX);
	CrossY = LandscapeQuat.RotateVector(CrossY);

	DrawDebugLine(World, Start - CrossX, Start + CrossX, FColor(128, 128, 128), false, Duration, 0, 1.f * Scale);
	DrawDebugLine(World, Start - CrossY, Start + CrossY, FColor(128, 128, 128), false, Duration, 0, 1.f * Scale);

	if (bAlsoDrawCone && ErosionSettings.bWindBias)
	{
		const int32 Spokes = 6;
		for (int32 Index = 0; Index <= Spokes; Index++)
		{
			const float NormalizedIndex = static_cast<float>(Index) / static_cast<float>(Spokes);
			const float Deg = MeanDeg - ConeHalfAngleDeg + (2.f * ConeHalfAngleDeg) * NormalizedIndex;

			Direction2D = UErosionLibrary::GetWindUnitVectorFromAngle(Deg);
			Direction3D = FVector(Direction2D.X, Direction2D.Y, 0.f);

			Direction3D = LandscapeQuat.RotateVector(Direction3D);

			const FVector SEnd = Start + Direction3D * (ArrowLength * 0.85f * Scale);
			DrawDebugDirectionalArrow(World, Start, SEnd, ArrowHeadSize * 0.75f * Scale, FColor(0, 200, 255), false, Duration, 0, ArrowThickness * 0.75f * Scale);
		}
	}
#endif
}

#pragma endregion
