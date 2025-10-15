// © Manuel Solano
// © Roberto Capparelli

#include "Libraries/PipelineLibrary.h"

// Unreal Engine includes for world partitioning, asset management, and landscape functionality.
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

// Constants for file paths and asset management.
#define EMPTY_STRING ""
#define HEIGHTMAP_PATH_SUFFIX "Saved/HeightMap/raw.r16" 
#define HEIGHTMAP_ASSET_PREFIX "/DropByDrop/SavedAssets/"

#pragma region Erosion(Templates)

/**
 * Applies hydraulic erosion simulation to an existing landscape.
 * This function:
 * 1. Validates the landscape and extracts heightmap data.
 * 2. Runs the erosion simulation.
 * 3. Creates a new landscape with the eroded heightmap.
 * 4. Preserves all settings from the original landscape.
 */
void UPipelineLibrary::GenerateErosion(TObjectPtr<ALandscape> ActiveLandscape, FErosionSettings& ErosionSettings)
{
	// Retrieve the landscape info component which stores heightmap and generation settings.
	ULandscapeInfoComponent* ActiveLandscapeInfoComponent = ActiveLandscape->FindComponentByClass<ULandscapeInfoComponent>();
	if (!ActiveLandscapeInfoComponent)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("The \"Active Landscape\" resource is invalid!"));
		return;
	}

	// Create a progress dialog for long-running erosion operation.
	FScopedSlowTask SlowTask(100, FText::FromString("Erosion in progress..."));
	SlowTask.MakeDialog(true);

	// Initialize erosion context with current heightmap data.
	FErosionContext ErosionContext;
	UErosionLibrary::SetHeights(ErosionContext, ActiveLandscapeInfoComponent->GetHeightMapSettings().HeightMap);

	// Run the erosion simulation algorithm.
	UErosionLibrary::Erosion(ErosionContext, ErosionSettings, ActiveLandscapeInfoComponent->GetHeightMapSettings().Size);

	SlowTask.EnterProgressFrame(50, FText::FromString("Applying on the landscape..."));

	// Convert eroded heightmap from normalized float to 16-bit unsigned integer format required by Unreal.
	TArray<uint16> ErodedHeightmapU16 = ConvertArrayFromFloatToUInt16(UErosionLibrary::GetHeights(ErosionContext));

	// Calculate the transform (position, rotation, scale) for the new landscape.
	const FTransform LandscapeTransform = GetNewTransform(ActiveLandscapeInfoComponent->GetExternalSettings(), ActiveLandscapeInfoComponent->GetLandscapeSettings(), ActiveLandscapeInfoComponent->GetHeightMapSettings().Size);

	SlowTask.EnterProgressFrame(50);

	// Spawn the new landscape actor with the eroded heightmap.
	TObjectPtr<ALandscape> NewLandscape = GenerateLandscape(LandscapeTransform, ErodedHeightmapU16);

	if (!IsValid(NewLandscape))
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape!"));
		return;
	}

	// Get the info component from the newly created landscape.
	ULandscapeInfoComponent* ErodedLandscapeInfoComponent = NewLandscape->FindComponentByClass<ULandscapeInfoComponent>();
	if (!ErodedLandscapeInfoComponent)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape!"));
		return;
	}

	// Mark this landscape as having been eroded and copy all settings from the original.
	ErodedLandscapeInfoComponent->SetIsEroded(true);
	ErodedLandscapeInfoComponent->SetExternalSettings(ActiveLandscapeInfoComponent->GetExternalSettings());
	ErodedLandscapeInfoComponent->SetHeightMapSettings(ActiveLandscapeInfoComponent->GetHeightMapSettings());
	ErodedLandscapeInfoComponent->SetLandscapeSettings(ActiveLandscapeInfoComponent->GetLandscapeSettings());

	UE_LOG(LogDropByDropLandscape, Log, TEXT("Landscape generated successfully!"));
}

/**
 * Saves a new erosion preset template with specified parameters.
 * Templates allow users to save and reuse erosion configurations.
 */
bool UPipelineLibrary::SaveErosionTemplate(const FString& TemplateName, const int32 ErosionCyclesValue, const float InertiaValue, const int32 CapacityValue, const float MinSlopeValue, const float DepositionSpeedValue, const float ErosionSpeedValue, const int32 GravityValue, const float EvaporationValue, const int32 MaxPathValue, const int32 ErosionRadiusValue)
{
	// Create a new template row and populate it with the provided parameters.
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

	// Get the global erosion templates data table.
	UDataTable* ErosionTemplatesDT = FDropByDropSettings::Get().GetErosionTemplatesDT();
	if (!ErosionTemplatesDT)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Templates DataTable\" resource is invalid!"));
		return false;
	}

	// Add the new template row to the data table.
	ErosionTemplatesDT->AddRow(FName(TemplateName), ErosionTemplateRow);

	// Persist the data table to disk.
	return SaveErosionTemplates(ErosionTemplatesDT);
}

/**
 * Saves the erosion templates data table to disk.
 * Marks the package as dirty and triggers the asset save operation.
 */
bool UPipelineLibrary::SaveErosionTemplates(UDataTable* ErosionTemplatesDT)
{
	// Get the package containing the data table.
	UPackage* ErosionTemplatesPackage = ErosionTemplatesDT->GetPackage();
	if (!ErosionTemplatesPackage)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Templates Package\" resource is invalid!"));
		return false;
	}

	// Mark as modified so Unreal knows it needs saving.
	ErosionTemplatesPackage->MarkPackageDirty();

	// Use the editor asset subsystem to save the asset.
	return GEditor->GetEditorSubsystem<UEditorAssetSubsystem>()->SaveLoadedAsset(ErosionTemplatesDT);
}

/**
 * Loads a previously saved erosion template by name.
 * @return Pointer to template data if found, nullptr otherwise.
 */
FErosionTemplateRow* UPipelineLibrary::LoadErosionTemplate(const FString& RowName)
{
	// Access the global templates data table.
	UDataTable* ErosionTemplatesDT = FDropByDropSettings::Get().GetErosionTemplatesDT();
	if (!ErosionTemplatesDT)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Templates DataTable\" resource is invalid!"));
		return nullptr;
	}

	// Context string for debugging/logging purposes.
	const FString ContextString(TEXT("Erosion Template Context"));

	// Search for the template row by name.
	FErosionTemplateRow* RowData = ErosionTemplatesDT->FindRow<FErosionTemplateRow>(FName(RowName), ContextString);
	if (!RowData)
	{
		return nullptr;
	}

	return RowData;
}

/**
 * Populates erosion settings from a loaded template.
 * Copies all erosion parameters from the template to the active settings.
 */
void UPipelineLibrary::LoadRowIntoErosionFields(TSharedPtr<FErosionSettings>& OutErosionSettings, const FErosionTemplateRow* TemplateDatas)
{
	if (!TemplateDatas)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Template Datas\" resource is invalid!"));
		return;
	}

	// Copy all erosion parameters from template to settings structure.
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

/**
 * Deletes a saved erosion template by name.
 * Removes the template from the data table and saves the changes.
 */
bool UPipelineLibrary::DeleteErosionTemplate(const FString& TemplateName)
{
	UDataTable* ErosionTemplatesDT = FDropByDropSettings::Get().GetErosionTemplatesDT();

	if (!ErosionTemplatesDT)
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"Erosion Templates DataTable\" resource is invalid!"));
		return false;
	}

	// Remove the row from the data table.
	ErosionTemplatesDT->RemoveRow(FName(TemplateName));

	// Persist the change to disk.
	return SaveErosionTemplates(ErosionTemplatesDT);
}

#pragma endregion

#pragma region HeightMap

/**
 * Generates a heightmap using procedural noise and saves it as a texture asset.
 * This creates both the raw heightmap data and a visual texture representation.
 */
bool UPipelineLibrary::CreateAndSaveHeightMap(FHeightMapGenerationSettings& Settings)
{
	// Generate the heightmap array using Perlin noise.
	Settings.HeightMap = CreateHeightMapArray(Settings);

	// Create a visual texture from the heightmap data for preview and export.
	UTexture2D* Texture = CreateHeightMapTexture(Settings.HeightMap, Settings.Size, Settings.Size);

	if (!Texture)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to create the heightmap texture!"));
		return false;
	}

	// Save the texture as a persistent asset.
	if (SaveToAsset(Texture, "TextureHeightMap"))
	{
		UE_LOG(LogDropByDropHeightmap, Log, TEXT("Heightmap texture saved successfully!"));
		return true;
	}

	UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to save the heightmap texture!"));

	return false;
}

/**
 * Generates a heightmap using multi-octave Perlin noise algorithm.
 * Implements:
 * - "Seed-based" random generation for reproducibility.
 * - Multiple octaves for terrain detail at different scales.
 * - Persistence and lacunarity for controlling octave influence.
 * - Normalization to specified height range.
 */
TArray<float> UPipelineLibrary::CreateHeightMapArray(const FHeightMapGenerationSettings& Settings)
{
	const int32 MapSize = Settings.Size;

	// Pre-allocate array for all height values.
	TArray<float> HeightMapValues;
	HeightMapValues.SetNum(MapSize * MapSize);

	// Determine seed: either use random seed or fixed seed for reproducibility.
	const int32 CurrentSeed = Settings.bRandomizeSeed ? FMath::RandRange(-10000, 10000) : Settings.Seed;
	const FRandomStream RandomStream(CurrentSeed);

	// Generate random offset for each octave to ensure variety.
	TArray<FVector2D> Offsets;
	Offsets.SetNum(Settings.NumOctaves);

	for (uint32 Index = 0; Index < Settings.NumOctaves; Index++)
	{
		Offsets[Index] = FVector2D(RandomStream.FRandRange(-1000.0f, 1000.0f), RandomStream.FRandRange(-1000.0f, 1000.0f));
	}

	// Track min/max values for normalization.
	float MinValue = TNumericLimits<float>::Max();
	float MaxValue = TNumericLimits<float>::Lowest();

	// Generate Perlin Noise for each point in the heightmap.
	for (int32 Height = 0; Height < MapSize; Height++)
	{
		for (int32 Width = 0; Width < MapSize; Width++)
		{
			float NoiseValue = 0.0f;
			float Scale = Settings.InitialScale;
			float Weight = 1.0f;

			// Sum multiple octaves of Perlin noise.
			// Each octave adds detail at a different frequency/scale.
			for (uint32 OctaveIndex = 0; OctaveIndex < Settings.NumOctaves; OctaveIndex++)
			{
				// Calculate sample location with offset and scale.
				FVector2D Location = Offsets[OctaveIndex] + FVector2D(Width, Height) / static_cast<float>(MapSize) * Scale;

				// Add weighted noise contribution from this octave.
				NoiseValue += FMath::PerlinNoise2D(Location) * Weight;

				// Persistence: reduces the amplitude of each subsequent octave.
				Weight *= Settings.Persistence;

				// Lacunarity: increases the frequency of each subsequent octave.
				Scale *= Settings.Lacunarity;
			}

			const int32 Index = Height * MapSize + Width;
			HeightMapValues[Index] = NoiseValue;

			// Track range for normalization.
			MinValue = FMath::Min(MinValue, NoiseValue);
			MaxValue = FMath::Max(MaxValue, NoiseValue);
		}
	}

	// Normalize values to [0, MaxHeightDifference] range.
	if (!FMath::IsNearlyEqual(MinValue, MaxValue))
	{
		for (float& Value : HeightMapValues)
		{
			// Map from [MinValue, MaxValue] to [0, 1].
			Value = (Value - MinValue) / (MaxValue - MinValue);

			// Scale to desired height range.
			Value *= Settings.MaxHeightDifference;
		}
	}

	return HeightMapValues;
}

/**
 * Creates a "Texture2D" asset from heightmap data for visualization.
 * Converts normalized float height values to grayscale BGRA8 texture format.
 */
UTexture2D* UPipelineLibrary::CreateHeightMapTexture(const TArray<float>& HeightMapData, const int32 Width, const int32 Height)
{
	// Create a new transient texture object.
	UTexture2D* Texture = NewObject<UTexture2D>(GetTransientPackage(), UTexture2D::StaticClass(), FName(*FString::Printf(TEXT("GeneratedTexture_%d"), FMath::Rand())), RF_Public | RF_Standalone);

	if (!Texture)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to create texture!"));
		return nullptr;
	}

	// Initialize platform-specific texture data.
	FTexturePlatformData* PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = Width;
	PlatformData->SizeY = Height;
	PlatformData->PixelFormat = PF_B8G8R8A8; // BGRA 8-bit per channel format.

	// Create the first (and only) mipmap level.
	FTexture2DMipMap* MipMap = new FTexture2DMipMap();
	MipMap->SizeX = Width;
	MipMap->SizeY = Height;
	PlatformData->Mips.Add(MipMap);

	// Allocate memory for pixel data (4 bytes per pixel for BGRA).
	MipMap->BulkData.Lock(LOCK_READ_WRITE);
	uint8* MipData = MipMap->BulkData.Realloc(Width * Height * 4);
	FMemory::Memzero(MipData, Width * Height * 4); // Initialize to zero.

	// Convert float heightmap values to grayscale pixels.
	for (int32 Index = 0; Index < HeightMapData.Num(); Index++)
	{
		// Convert normalized float [0, 1] to 8-bit grayscale [0, 255].
		const uint8 GrayValue = FMath::Clamp(static_cast<uint8>(HeightMapData[Index] * 255.0f), 0, 255);

		// Write grayscale value to all RGB channels (BGRA order).
		MipData[Index * 4 + 0] = GrayValue; // B (Blue).
		MipData[Index * 4 + 1] = GrayValue; // G (Green).
		MipData[Index * 4 + 2] = GrayValue; // R (Red).
		MipData[Index * 4 + 3] = 255;       // A (Alpha - fully opaque).
	}

	MipMap->BulkData.Unlock();

	// Assign platform data to texture.
	Texture->SetPlatformData(PlatformData);

	// Configure texture compression and sampling settings.
	Texture->SRGB = true;
	Texture->CompressionSettings = TC_Default;
	Texture->MipGenSettings = TMGS_FromTextureGroup;
	Texture->UpdateResource();

	return Texture;
}

/**
 * Opens a native file dialog for the user to select an external heightmap (PNG).
 * Imports the selected file as a texture and saves it as an asset.
 */
bool UPipelineLibrary::OpenHeightmapFileDialog(TSharedPtr<FExternalHeightMapSettings> ExternalSettings)
{
	// Get the desktop platform interface for native dialogs.
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Failed to open file dialog!"));
		return false;
	}

	TArray<FString> OutFiles;
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	// Show file picker dialog filtered for PNG files.
	bool bOpened = DesktopPlatform->OpenFileDialog(ParentWindowHandle, TEXT("Select Heightmap (PNG)"), FPaths::ProjectDir(), TEXT(EMPTY_STRING), TEXT("PNG files (*.png)|*.png"), EFileDialogFlags::None, OutFiles);

	if (!bOpened || OutFiles.Num() <= 0)
	{
		return false;
	}

	// Get the selected file (first result if multiple selected).
	const FString& SelectedFile = OutFiles[0];

	// Import the PNG file as a "Texture2D".
	if (UTexture2D* Imported = FImageUtils::ImportFileAsTexture2D(SelectedFile))
	{
		if (!Imported)
		{
			UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Failed to overwrite preview from PNG: %s"), *SelectedFile);
			return false;
		}

		// Save the imported texture as a persistent asset.
		if (!SaveToAsset(Imported, TEXT("TextureHeightMap")))
		{
			UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Failed to overwrite preview from PNG: %s"), *SelectedFile);
			return false;
		}

		UE_LOG(LogDropByDropHeightmap, Log, TEXT("Preview overwritten from external PNG: %s"), *SelectedFile);

		// Mark that we're now using an external heightmap.
		ExternalSettings->bIsExternalHeightMap = true;
		ExternalSettings->LastPNGPath = SelectedFile;

		return true;
	}

	return false;
}

/**
 * Loads heightmap data from an external PNG file on the file system.
 * Supports multiple pixel formats: RGBA8, BGRA8 G16, R32F and other common formats.
 * Outputs both raw uint16 data and normalized float data [0, 1].
 */
void UPipelineLibrary::LoadHeightmapFromFileSystem(const FString& FilePath, TArray<uint16>& OutHeightMap, TArray<float>& OutNormalizedHeightmap, FExternalHeightMapSettings& Settings)
{
	OutHeightMap.Empty();
	OutNormalizedHeightmap.Empty();

	// Import the PNG file as a texture.
	UTexture2D* Texture = FImageUtils::ImportFileAsTexture2D(FilePath);
	if (!Texture)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to load PNG file: %s"), *FilePath);
		return;
	}

	// Access the texture's first mipmap level (full resolution).
	FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
	const void* MipData = MipMap.BulkData.LockReadOnly();
	int32 Width = MipMap.SizeX;
	int32 Height = MipMap.SizeY;

	UE_LOG(LogDropByDropHeightmap, Log, TEXT("Loaded Texture from PNG: Width = %d, Height = %d"), Width, Height);

	// Track min/max pixel values for normalization.
	uint32 MinPixel = TNumericLimits<uint32>::Max();
	uint32 MaxPixel = TNumericLimits<uint32>::Min();

	OutHeightMap.SetNum(Width * Height);
	OutNormalizedHeightmap.SetNum(Width * Height);

	// Determine which channel to use based on format
	// Per heightmap, usiamo il canale RED (indipendentemente dall'ordine dei byte)
	const EPixelFormat PixelFormat = Texture->GetPixelFormat();

	// Process pixel data based on the texture's pixel format.
	switch (PixelFormat)
	{
		case PF_R8G8B8A8:  // RGBA 8-bit
		{
			const uint8* PixelData = static_cast<const uint8*>(MipData);
			for (int32 Index = 0; Index < Width * Height; Index++)
			{
				// RGBA order: R = 0, G = 1, B = 2, A = 3
				uint8 RedValue = PixelData[Index * 4 + 0];
				uint16 HeightValue = static_cast<uint16>(RedValue << 8);

				OutHeightMap[Index] = HeightValue;
				MinPixel = FMath::Min<uint32>(MinPixel, HeightValue);
				MaxPixel = FMath::Max<uint32>(MaxPixel, HeightValue);
			}

			break;
		}
		case PF_B8G8R8A8:  // BGRA 8-bit
		{
			const uint8* PixelData = static_cast<const uint8*>(MipData);
			for (int32 Index = 0; Index < Width * Height; Index++)
			{
				// BGRA order: B = 0, G = 1, R = 2, A = 3
				uint8 RedValue = PixelData[Index * 4 + 2];
				uint16 HeightValue = static_cast<uint16>(RedValue << 8);

				OutHeightMap[Index] = HeightValue;
				MinPixel = FMath::Min<uint32>(MinPixel, HeightValue);
				MaxPixel = FMath::Max<uint32>(MaxPixel, HeightValue);
			}

			break;
		}
		case PF_G16:  // 16-bit grayscale (native heightmap format)
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
		case PF_R32_FLOAT:  // 32-bit floating point format (HDR heightmaps)
		{
			const float* PixelData = static_cast<const float*>(MipData);
			for (int32 Index = 0; Index < Width * Height; Index++)
			{
				uint16 HeightValue = static_cast<uint16>(FMath::Clamp(PixelData[Index], 0.f, 1.f) * 65535.0f);
				OutHeightMap[Index] = HeightValue;
				MinPixel = FMath::Min<uint32>(MinPixel, HeightValue);
				MaxPixel = FMath::Max<uint32>(MaxPixel, HeightValue);
			}

			break;
		}
		case PF_G8:  // 8-bit grayscale
		{
			const uint8* PixelData = static_cast<const uint8*>(MipData);
			for (int32 Index = 0; Index < Width * Height; Index++)
			{
				uint8 GrayValue = PixelData[Index];
				uint16 HeightValue = static_cast<uint16>(GrayValue << 8);

				OutHeightMap[Index] = HeightValue;
				MinPixel = FMath::Min<uint32>(MinPixel, HeightValue);
				MaxPixel = FMath::Max<uint32>(MaxPixel, HeightValue);
			}

			break;
		}
		default:
		{
			MipMap.BulkData.Unlock();
			UE_LOG(LogDropByDropHeightmap, Error, TEXT("Unsupported pixel format in PNG! Format: %d"), static_cast<int32>(PixelFormat));
			UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Supported formats: RGBA8, BGRA8, G16, G8, R32F"));
			return;
		}
	}

	MipMap.BulkData.Unlock();

	// Normalize heightmap values to [0, 1] range.
	for (int32 Index = 0; Index < OutHeightMap.Num(); Index++)
	{
		OutNormalizedHeightmap[Index] = (MaxPixel > MinPixel) ? (OutHeightMap[Index] - MinPixel) / static_cast<float>(MaxPixel - MinPixel) : 0.0f;
	}

	Settings.bIsExternalHeightMap = true;

	UE_LOG(LogDropByDropHeightmap, Log, TEXT("Heightmap loaded successfully. Min: %u, Max: %u"), MinPixel, MaxPixel);
}

/**
 * Debug utility to compare a generated heightmap with a RAW file exported by Unreal.
 * Used for validation and troubleshooting heightmap generation.
 */
void UPipelineLibrary::CompareHeightmaps(const FString& RawFilePath, const TArray<uint16>& GeneratedHeightmap, int32 Width, int32 Height)
{
	// Load RAW file (16-bit heightmap format used by Unreal).
	TArray<uint8> RawData;
	if (!FFileHelper::LoadFileToArray(RawData, *RawFilePath))
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to load RAW file at: %s"), *RawFilePath);
		return;
	}

	// Verify file size matches expected dimensions (uint16 = 2 bytes per pixel).
	const uint64 ExpectedSize = static_cast<uint64>(Width) * static_cast<uint64>(Height) * sizeof(uint16);
	if (RawData.Num() != ExpectedSize)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("RAW file size doesn't match the expected size: %llu VS %llu"), static_cast<uint64>(RawData.Num()), ExpectedSize);
		return;
	}

	// Convert byte array to uint16 array (reinterpret cast).
	TArray<uint16> UnrealHeightmap;
	UnrealHeightmap.SetNum(Width * Height);
	FMemory::Memcpy(UnrealHeightmap.GetData(), RawData.GetData(), RawData.Num());

	// Log first 10 values for comparison
	for (int32 Index = 0; Index < FMath::Min(10, UnrealHeightmap.Num()); Index++)
	{
		UE_LOG(LogDropByDropHeightmap, Log, TEXT("RAW[%d]: %d, Generated[%d]: %d"), Index, UnrealHeightmap[Index], Index, GeneratedHeightmap[Index]);
	}
}

#pragma endregion

#pragma region Landscape

/**
 * Internal function to initialize and create a landscape with given heightmap data.
 * Sets up the landscape and stores generation settings in its info component.
 */
bool UPipelineLibrary::InitLandscape(TArray<uint16>& HeightData, FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings)
{
	// Calculate world transform for the new landscape.
	const FTransform LandscapeTransform = GetNewTransform(ExternalSettings, LandscapeSettings, HeightmapSettings.Size);

	// Spawn the landscape with heightmap data.
	TObjectPtr<ALandscape> NewLandscape = GenerateLandscape(LandscapeTransform, HeightData);
	if (!IsValid(NewLandscape))
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape!"));
		return false;
	}

	// Get the landscape info component.
	ULandscapeInfoComponent* NewLandscapeInfo = NewLandscape->FindComponentByClass<ULandscapeInfoComponent>();
	if (!NewLandscapeInfo)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape!"));
		return false;
	}

	// Store all generation settings in the info component for later reference.
	NewLandscapeInfo->SetHeightMapSettings(HeightmapSettings);
	NewLandscapeInfo->SetExternalSettings(ExternalSettings);
	NewLandscapeInfo->SetLandscapeSettings(LandscapeSettings);

	UE_LOG(LogDropByDropLandscape, Log, TEXT("Landscape created successfully!"));

	return true;
}

/**
 * Creates a landscape from internally generated (procedural) heightmap data.
 * Uses Perlin noise generation if heightmap hasn't been generated yet.
 */
void UPipelineLibrary::CreateLandscapeFromInternalHeightMap(FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings)
{
	// Mark that we're not using an external heightmap.
	ExternalSettings.bIsExternalHeightMap = false;

	// Generate heightmap if it hasn't been created yet.
	if (HeightmapSettings.HeightMap.Num() <= 0)
	{
		if (HeightmapSettings.bRandomizeSeed)
		{
			// Create temporary settings with a random seed for variation.
			FHeightMapGenerationSettings TempSettings = HeightmapSettings;
			TempSettings.Seed = FMath::RandRange(-10000, 10000);
			HeightmapSettings.HeightMap = CreateHeightMapArray(TempSettings);
		}
		else
		{
			// Use the specified seed for reproducible generation.
			HeightmapSettings.HeightMap = CreateHeightMapArray(HeightmapSettings);
		}
	}

	// Validate world context.
	const UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape! \"World\" resource is invalid!"));
		return;
	}

	// Convert normalized float heightmap to uint16 format required by Unreal's landscape system.
	TArray<uint16> HeightData = ConvertArrayFromFloatToUInt16(HeightmapSettings.HeightMap);
	InitLandscape(HeightData, HeightmapSettings, ExternalSettings, LandscapeSettings);
}

/**
 * Creates a landscape from an external heightmap file (PNG).
 * Loads the file, validates it, and creates the landscape.
 */
void UPipelineLibrary::CreateLandscapeFromExternalHeightMap(const FString& FilePath, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings, FHeightMapGenerationSettings& HeightmapSettings)
{
	// Mark that we're using an external heightmap source.
	ExternalSettings.bIsExternalHeightMap = true;
	ExternalSettings.LastPNGPath = FilePath;

	// Load heightmap data from the PNG file.
	TArray<uint16> HeightMapInt16;
	LoadHeightmapFromFileSystem(FilePath, HeightMapInt16, HeightmapSettings.HeightMap, ExternalSettings);

	// Debug: Optional comparison with reference RAW file (non-blocking).
	FString HeightMapPath = FPaths::ProjectDir() + TEXT(HEIGHTMAP_PATH_SUFFIX);
	if (FPaths::FileExists(HeightMapPath))
	{
		CompareHeightmaps(HeightMapPath, HeightMapInt16, 505, 505);
	}
	else
	{
		UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Reference RAW file not found for validation: %s"), *HeightMapPath);
	}

	// Validate that heightmap was loaded successfully.
	if (HeightMapInt16.Num() <= 0)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to load heightmap from PNG file!"));
		return;
	}

	// Create the landscape with the loaded heightmap.
	InitLandscape(HeightMapInt16, HeightmapSettings, ExternalSettings, LandscapeSettings);
}

/**
 * Automatic landscape generation router.
 * Decides whether to use external or internal heightmap based on settings.
 */
void UPipelineLibrary::GenerateLandscapeAuto(FHeightMapGenerationSettings& HeightmapSettings, FExternalHeightMapSettings& ExternalSettings, FLandscapeGenerationSettings& LandscapeSettings)
{
	// If external heightmap is specified and valid, use it.
	if (ExternalSettings.bIsExternalHeightMap && !ExternalSettings.LastPNGPath.IsEmpty() && FPaths::FileExists(ExternalSettings.LastPNGPath))
	{
		CreateLandscapeFromExternalHeightMap(ExternalSettings.LastPNGPath, ExternalSettings, LandscapeSettings, HeightmapSettings);
		return;
	}

	if (ExternalSettings.bIsExternalHeightMap && !ExternalSettings.LastPNGPath.IsEmpty())
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("External heightmap file not found: %s"), *ExternalSettings.LastPNGPath);
		ExternalSettings.bIsExternalHeightMap = false;
	}

	// Otherwise, generate procedurally.
	CreateLandscapeFromInternalHeightMap(HeightmapSettings, ExternalSettings, LandscapeSettings);
}

/**
 * Core landscape generation function.
 * Spawns a landscape and imports heightmap data using Unreal's landscape API.
 * Handles component size calculation, layer setup, and actor initialization.
 */
TObjectPtr<ALandscape> UPipelineLibrary::GenerateLandscape(const FTransform& LandscapeTransform, TArray<uint16>& Heightmap)
{
	int32 SubSectionSizeQuads;
	int32 NumSubsections;
	int32 MaxX, MaxY;

	// Calculate optimal landscape component parameters.
	if (!SetLandscapeSizeParam(SubSectionSizeQuads, NumSubsections, MaxX, MaxY))
	{
		return nullptr;
	}

	// Prepare heightmap data per layer (using default layer with empty GUID).
	TMap<FGuid, TArray<uint16>> HeightDataPerLayer;

	// Prepare material layer data (empty for basic landscape).
	TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayer;
	TArray<FLandscapeImportLayerInfo> MaterialImportLayers;
	MaterialImportLayers.Reserve(0);
	MaterialLayerDataPerLayer.Add(FGuid(), MoveTemp(MaterialImportLayers));

	// Validate heightmap dimensions match expected landscape size.
	if (Heightmap.Num() != MaxX * MaxY)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Heightmap dimensions don't match the generated landscape: %d != %d x %d"), Heightmap.Num(), MaxX, MaxY);
		return nullptr;
	}

	// Move heightmap data into the per-layer map (efficient transfer without copying).
	HeightDataPerLayer.Add(FGuid(), MoveTemp(Heightmap));

	// Get editor world context.
	const FWorldContext& EditorWorldContext = GEditor->GetEditorWorldContext();
	UWorld* World = EditorWorldContext.World();

	if (!World)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to generate the landscape! \"World\" resource is invalid!"));
		return nullptr;
	}

	// Spawn the landscape in the world.
	ALandscape* Landscape = World->SpawnActor<ALandscape>();

	// Create and attach the custom "LandscapeInfoComponent" for storing generation metadata.
	ULandscapeInfoComponent* LandscapeInfo = NewObject<ULandscapeInfoComponent>(Landscape);
	LandscapeInfo->RegisterComponent();
	Landscape->AddInstanceComponent(LandscapeInfo);

	// Configure landscape properties.
	Landscape->bCanHaveLayersContent = true; // Enable layer system for editing.
	Landscape->LandscapeMaterial = nullptr;  // No material assigned by default.
	Landscape->SetActorTransform(LandscapeTransform);

	UE_LOG(LogDropByDropLandscape, Error, TEXT("NumSubSections: %d; SubSectionSizeQuads: %d; MaxY: %d"), NumSubsections, SubSectionSizeQuads, MaxY);

	// Import heightmap data into the landscape using Unreal's import system.
	TArray<FLandscapeLayer> LandscapeLayers;
	Landscape->Import(
		FGuid::NewGuid(),              // Unique landscape GUID.
		0, 0,                          // Import offset (top-left corner).
		MaxX - 1, MaxY - 1,            // Import dimensions (inclusive max values).
		NumSubsections,                // Number of subsections per component.
		SubSectionSizeQuads,           // Quads per subsection.
		HeightDataPerLayer,            // Heightmap data.
		nullptr,                       // Optional landscape info map name.
		MaterialLayerDataPerLayer,     // Material layer data.
		ELandscapeImportAlphamapType::Additive, // Alpha blending mode.
		MakeArrayView(LandscapeLayers) // Layer definitions.
	);

	// Finalize landscape creation.
	Landscape->PostEditChange();

	return Landscape;
}

/**
 * Splits a large landscape into multiple streaming proxies for world partition.
 * This enables efficient streaming and editing of large terrains.
 * Uses the "LandscapeSubsystem" to partition the landscape into grid cells.
 */
void UPipelineLibrary::SplitLandscapeIntoProxies(ALandscape& ActiveLandscape)
{
	// Get the landscape info which contains component and layer data.
	ULandscapeInfo* LandscapeInfo = ActiveLandscape.GetLandscapeInfo();

	if (!LandscapeInfo)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to split the landscape! \"LandscapeInfo\" resource is invalid!"));
		return;
	}

	// Update layer info and register all components before splitting.
	LandscapeInfo->UpdateLayerInfoMap(&ActiveLandscape);
	ActiveLandscape.RegisterAllComponents();

	// Show progress dialog for potentially long operation.
	FScopedSlowTask SlowTask(100, FText::FromString("Splitting Landscape..."));
	SlowTask.MakeDialog(true);

	ULandscapeInfoComponent* LandscapeInfoComponent = nullptr;
	UWorld* World = GEditor->GetEditorWorldContext().World();

	if (World)
	{
		// Use the landscape subsystem to change grid size (splits into proxies).
		if (ULandscapeSubsystem* LandscapeSubsystem = World->GetSubsystem<ULandscapeSubsystem>())
		{
			LandscapeInfoComponent = ActiveLandscape.FindComponentByClass<ULandscapeInfoComponent>();

			// Change grid size triggers proxy creation based on world partition cell size.
			LandscapeSubsystem->ChangeGridSize(LandscapeInfo, LandscapeInfoComponent->GetLandscapeSettings().WorldPartitionCellSize);
		}
	}

	SlowTask.EnterProgressFrame(100);

	// Notify Unreal that the landscape has been modified.
	ActiveLandscape.PostEditChange();

	// Update world partition streaming state to reflect new proxies.
	if (World)
	{
		if (UWorldPartitionSubsystem* WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>())
		{
			// Cast to streaming interface and update streaming state.
			if (IStreamingWorldSubsystemInterface* StreamingWorldPartitionSubsystemInterface = Cast<IStreamingWorldSubsystemInterface>(WorldPartitionSubsystem))
			{
				StreamingWorldPartitionSubsystemInterface->OnUpdateStreamingState();
				GEditor->RedrawAllViewports(); // Refresh editor viewports to show changes.
			}
		}

	}

	// Verify info component was created successfully.
	if (!LandscapeInfoComponent)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Failed to split the landscape!"));
		return;
	}

	// Mark the landscape as having been split into proxies.
	LandscapeInfoComponent->SetIsSplittedIntoProxies(true);
}

/**
 * Calculates optimal landscape component parameters based on heightmap size.
 * Unreal landscapes have strict size requirements (power-of-2 based).
 * Uses "FLandscapeImportHelper" to determine best component configuration.
 */
bool UPipelineLibrary::SetLandscapeSizeParam(int32& SubSectionSizeQuads, int32& NumSubsections, int32& MaxX, int32& MaxY, const int32 Size)
{
	const int32 HeightmapSize = Size;
	FIntPoint NewLandscapeComponentCount;

	// Let Unreal determine the best component subdivision.
	FLandscapeImportHelper::ChooseBestComponentSizeForImport(HeightmapSize, HeightmapSize, SubSectionSizeQuads, NumSubsections, NewLandscapeComponentCount);

	// Calculate total landscape dimensions.
	const int32 ComponentCountX = NewLandscapeComponentCount.X;
	const int32 ComponentCountY = NewLandscapeComponentCount.Y;
	const int32 QuadsPerComponent = NumSubsections * SubSectionSizeQuads;

	// Add 1 because vertices = quads + 1.
	MaxX = ComponentCountX * QuadsPerComponent + 1;
	MaxY = ComponentCountY * QuadsPerComponent + 1;

	// Verify calculated size matches input heightmap size.
	if (MaxX != HeightmapSize || MaxY != HeightmapSize)
	{
		UE_LOG(LogDropByDropLandscape, Error, TEXT("Size calculated (%d x %d) not match with size of HeightMap (%d x %d)"))
		return false;
	}

	return true;
}

#pragma endregion

#pragma region Utilities

/**
 * Calculates the world transform for a new landscape.
 * Handles both external heightmap scaling and procedural landscape kilometer-based scaling.
 */
FTransform UPipelineLibrary::GetNewTransform(const FExternalHeightMapSettings& ExternalSettings, const FLandscapeGenerationSettings& LandscapeSettings, int32 HeightmapSize)
{
	FVector NewScale;

	if (ExternalSettings.bIsExternalHeightMap)
	{
		// Use explicit scaling values from external heightmap settings.
		NewScale = FVector(ExternalSettings.ScalingX, ExternalSettings.ScalingY, ExternalSettings.ScalingZ);
	}
	else
	{
		// Calculate scale to achieve desired size in kilometers.
		const FVector BaseScale = FVector(100, 100, 100); // Base Unreal landscape scale.
		const float CurrentSizeInUnits = HeightmapSize * BaseScale.X;

		// Convert kilometers to Unreal units (1 km = 100,000 cm).
		const float DesiredSizeInUnits = LandscapeSettings.Kilometers * 1000.f * 100.f;
		const float ScaleFactor = DesiredSizeInUnits / CurrentSizeInUnits;

		NewScale = BaseScale * ScaleFactor;
	}

	// Return transform with identity rotation, fixed position, and calculated scale.
	// Position is hardcoded for consistent placement in the world.
	return FTransform(FQuat::Identity, FVector(-100800, -100800, 17200), NewScale);
}

/**
 * Converts normalized float heightmap values [0, 1] to 16-bit unsigned integers [0, 65535].
 * This conversion is required by Unreal's landscape system which uses uint16 for heights.
 */
TArray<uint16> UPipelineLibrary::ConvertArrayFromFloatToUInt16(const TArray<float>& FloatData)
{
	TArray<uint16> UInt16Data;
	UInt16Data.AddUninitialized(FloatData.Num());

	for (int32 Index = 0; Index < FloatData.Num(); Index++)
	{
		// Scale from [0, 1] to [0, 65535] and clamp to valid range.
		UInt16Data[Index] = FMath::Clamp<uint16>(FMath::RoundToInt(FloatData[Index] * 65535.0f), 0, 65535);
	}

	return UInt16Data;
}

/**
 * Saves a texture as a persistent Unreal asset.
 * Handles complex pixel format conversions, ensuring grayscale heightmaps display correctly.
 *
 * Process:
 * 1. Extracts pixel data from texture source or platform data.
 * 2. Converts any format to grayscale (8-bit).
 * 3. Replicates grayscale to BGRA8 for proper Slate/UI display.
 * 4. Creates or updates the asset package.
 * 5. Saves to disk and updates asset registry.
 */
bool UPipelineLibrary::SaveToAsset(UTexture2D* Texture, const FString& AssetName)
{
	if (!Texture)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"Texture\" resource is invalid!"));
		return false;
	}

	// Lambda to apply texture settings optimized for UI preview display.
	auto ApplyPreviewTextureFlags = [](UTexture2D* T)
		{
			T->SRGB = true;
			T->CompressionSettings = TC_Default; // Minimal compression for quality.
			T->MipGenSettings = TMGS_NoMipmaps;  // No mipmaps needed for previews.
			T->LODGroup = TEXTUREGROUP_Pixels2D;
			T->NeverStream = true;               // Always keep in memory.
			T->AddressX = TA_Clamp;              // Clamp texture edges.
			T->AddressY = TA_Clamp;
		};

	// Step 1: Extract raw pixel data from texture source (preferred) or platform data (fallback).
	TArray64<uint8> Raw;
	int32 Width = 0, Height = 0;
	ERawFormat InFormat = ERawFormat::Unknown;

	// Try to get data from texture source (original import data).
	const int32 SrcWidth = Texture->Source.GetSizeX();
	const int32 SrcHeight = Texture->Source.GetSizeY();

	if (SrcWidth > 0 && SrcHeight > 0 && Texture->Source.GetNumMips() > 0)
	{
		// Attempt to get mip data directly.
		bool bOk = Texture->Source.GetMipData(Raw, 0);

		// Fallback: lock mip manually if "GetMipData" fails.
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

		// Determine source format if we got data.
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

	// Fallback: Try platform data if source data unavailable.
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

		// Lock platform mip data for reading.
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

		// Allocate buffer based on pixel format.
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

		// Copy platform data to our buffer.
		if (InFormat != ERawFormat::Unknown)
		{
			FMemory::Memcpy(Raw.GetData(), MipData, Raw.Num());
		}

		Mip.BulkData.Unlock();
	}

	// Validate we have usable data.
	if (InFormat == ERawFormat::Unknown || Raw.Num() <= 0 || Width <= 0 || Height <= 0)
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("Unsupported or empty source format when saving preview asset!"));
		return false;
	}

	// Step 2: Convert any pixel format to 8-bit grayscale.
	NumPixels = static_cast<int64>(Width) * Height;
	TArray64<uint8> Greys;
	Greys.SetNumUninitialized(NumPixels);

	switch (InFormat)
	{
	case ERawFormat::BGRA8:
	{
		// Extract red channel from BGRA format.
		const uint8* Ptr = Raw.GetData();
		for (int64 Index = 0; Index < NumPixels; Index++)
		{
			// BGRA order: B = 0, G = 1, R = 2, A = 3
			Greys[Index] = Ptr[Index * 4 + 2]; // Red channel.
		}

		break;
	}
	case ERawFormat::RGBA8:
	{
		// Extract red channel from RGBA format.
		const uint8* Ptr = Raw.GetData();
		for (int64 Index = 0; Index < NumPixels; Index++)
		{
			// RGBA order: R = 0, G = 1, B = 2, A = 3
			Greys[Index] = Ptr[Index * 4 + 0]; // Red channel.
		}

		break;
	}
	case ERawFormat::G8:
	{
		// Already grayscale, just copy.
		FMemory::Memcpy(Greys.GetData(), Raw.GetData(), Greys.Num());

		break;
	}
	case ERawFormat::G16:
	{
		// Convert 16-bit grayscale to 8-bit by keeping most significant byte.
		const uint16* Ptr = reinterpret_cast<const uint16*>(Raw.GetData());
		for (int64 Index = 0; Index < NumPixels; Index++)
		{
			Greys[Index] = static_cast<uint8>(Ptr[Index] >> 8); // Keep upper 8 bits.
		}

		break;
	}
	case ERawFormat::R32F:
	{
		// Convert 32-bit float [0, 1] to 8-bit integer [0, 255].
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
		// Convert HDR float to 8-bit, using red channel.
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

	// Step 3: Convert grayscale to BGRA8 format.
	// This ensures Slate UI displays neutral grayscale instead of colored channels.
	TArray64<uint8> BGRA;
	BGRA.SetNumUninitialized(NumPixels * 4);

	for (int64 Index = 0; Index < NumPixels; Index++)
	{
		const uint8 Gray = Greys[Index];
		const int64 Offset = Index * 4;
		BGRA[Offset] = Gray;     // B (Blue).
		BGRA[Offset + 1] = Gray; // G (Green).
		BGRA[Offset + 2] = Gray; // R (Red).
		BGRA[Offset + 3] = 255;  // A (Alpha - fully opaque).
	}

	// Step 4: Create or update the asset package.
	const FString PackagePath = TEXT("/DropByDrop/SavedAssets");
	const FString PackageName = FString::Printf(TEXT("%s/%s"), *PackagePath, *AssetName);

	// Check if asset already exists.
	UPackage* ExistingPackage = FindPackage(nullptr, *PackageName);
	UTexture2D* TargetTexture = ExistingPackage ? Cast<UTexture2D>(StaticFindObject(UTexture2D::StaticClass(), ExistingPackage, *AssetName)) : nullptr;

	// Lambda to save package and update asset registry.
	auto SaveAndRescan = [&](UPackage* Package, UObject* Asset)
		{
			FlushRenderingCommands(); // Ensure all rendering is complete.

			// Configure save parameters.
			FSavePackageArgs SaveArgs;
			SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
			SaveArgs.SaveFlags = SAVE_NoError;

			const FString PackageFilename = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

			// Attempt to save package to disk.
			if (!UPackage::SavePackage(Package, Asset, *PackageFilename, SaveArgs))
			{
				UE_LOG(LogDropByDropHeightmap, Error, TEXT("Failed to save package \"%s\""), *PackageName);
				return false;
			}

			// Update asset registry so the asset appears in content browser.
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

	// Lambda to write BGRA8 data to a texture object.
	auto WriteBGRA8ToTexture = [&](UTexture2D* T)
		{
			T->Modify(); // Mark for undo/redo.

			// Initialize texture source with BGRA8 format.
			T->Source.Init(Width, Height, 1, 1, TSF_BGRA8);
			void* Dest = T->Source.LockMip(0);
			const int64 DestSize = NumPixels * 4;
			FMemory::Memcpy(Dest, BGRA.GetData(), DestSize);
			T->Source.UnlockMip(0);

			// Apply preview-optimized settings.
			ApplyPreviewTextureFlags(T);
			T->PostEditChange(); // Notify property changes.
			T->UpdateResource(); // Update GPU resource.
			static_cast<void>(T->MarkPackageDirty()); // Mark package as modified.
		};

	// Update existing asset or create new one.
	if (TargetTexture)
	{
		UE_LOG(LogDropByDropHeightmap, Warning, TEXT("Texture '%s' already exists, updating asset..."), *AssetName);

		WriteBGRA8ToTexture(TargetTexture);
		return SaveAndRescan(TargetTexture->GetOutermost(), TargetTexture);
	}
	else
	{
		// Create new package and texture.
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
		FAssetRegistryModule::AssetCreated(NewTexture); // Notify asset registry of new asset.

		return SaveAndRescan(NewPackage, NewTexture);
	}
}

/**
 * Calculates the starting position for wind direction preview visualization.
 * Uses landscape component bounds to find the center, or falls back to landscape location.
 * Adds vertical offset to ensure visibility above terrain.
 */
FVector UPipelineLibrary::GetWindPreviewStart(const ALandscape* ActiveLandscape, UWorld* World)
{
	// Initialize bounds box for accumulating component bounds.
	FBox Bounds(ForceInitToZero);
	TArray<ULandscapeComponent*> Components;
	ActiveLandscape->GetComponents(Components);

	// Accumulate bounds from all landscape components.
	for (ULandscapeComponent* Component : Components)
	{
		if (!Component)
		{
			continue;
		}

		// Expand bounds to include this component.
		Bounds += Component->Bounds.GetBox();
	}

	// Use bounds center if valid.
	if (Bounds.IsValid)
	{
		FVector Top = Bounds.GetCenter();
		Top.Z += Bounds.Max.Z - 1000.f; // Elevate above terrain for visibility.

		return Top;
	}

	// Fallback: use landscape location.
	FVector Location = ActiveLandscape->GetActorLocation();
	Location.Z += Bounds.Max.Z - 1000.f; // Elevate above terrain for visibility.

	return Location;
}

/**
 * Gets the current wind preview scale factor from global settings.
 */
float UPipelineLibrary::GetWindPreviewScale()
{
	return FDropByDropSettings::Get().GetWindPreviewScale();
}

/**
 * Sets the wind preview scale factor in global settings.
 */
void UPipelineLibrary::SetWindPreviewScale(const float NewScale)
{
	FDropByDropSettings::Get().SetWindPreviewScale(NewScale);
}

/**
 * Draws a comprehensive wind direction visualization in the editor viewport.
 * Creates a 3D debug visualization showing:
 * - Main directional arrow indicating primary wind direction.
 * - Layered cones for visual emphasis.
 * - Sphere at arrow tip.
 * - Radial spokes from sphere.
 * - Crosshair at start point.
 * - Optional wind cone showing directional spread (if wind bias is enabled).
 *
 * All elements are scaled relative to landscape size for appropriate visibility.
 */
void UPipelineLibrary::DrawWindDirectionPreview(const FErosionSettings& ErosionSettings, const ALandscape* ActiveLandscape, float ArrowLength, float ArrowThickness, float ArrowHeadSize, float Duration, bool  bAlsoDrawCone, float ConeHalfAngleDeg)
{
#if WITH_EDITOR
	// Calculate scale factor based on landscape dimensions.
	// 1 km in Unreal ≈ 198 cm scale factor.
	const FVector LandscapeScale = ActiveLandscape->GetActorScale3D();
	const float AverageLandscapeScale = ((LandscapeScale.X / 198.f /* 1 KM ~= 198 CM */) + (LandscapeScale.Y / 198.f /* 1 KM ~= 198 CM */)) * 0.5f;
	const float Scale = FDropByDropSettings::Get().GetWindPreviewScale() * AverageLandscapeScale;

	// Get world context.
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

	// Calculate starting position (center of landscape, elevated).
	const FVector Start = GetWindPreviewStart(ActiveLandscape, World);
	float MeanDeg = 0.f;

	// Get wind direction angle from erosion settings.
	if (!UErosionLibrary::TryGetWindMeanAngleDegrees(ErosionSettings, MeanDeg))
	{
		return;
	}

	// Convert angle to 2D direction vector, then to 3D.
	FVector2D Direction2D = UErosionLibrary::GetWindUnitVectorFromAngle(MeanDeg);
	FVector Direction3D(Direction2D.X, Direction2D.Y, 0.f);

	// Rotate direction to match landscape orientation (only yaw matters for wind).
	const FRotator LandscapeRotation = ActiveLandscape->GetActorRotation();
	const FQuat LandscapeQuat = FQuat(FRotator(0.f, LandscapeRotation.Yaw, 0.f));
	Direction3D = LandscapeQuat.RotateVector(Direction3D);

	// Calculate arrow end point.
	const FVector End = Start + Direction3D * (ArrowLength * Scale);

	// Draw main arrow body (cyan line, 60% of total length).
	const FVector ArrowBodyEnd = Start + Direction3D * (ArrowLength * Scale * 0.6f);
	DrawDebugLine(World, Start, ArrowBodyEnd, FColor::Cyan, false, Duration, 0, ArrowThickness * Scale * 3.f);

	// Draw layered cones at arrow tip for visual emphasis.
	const float HugeHeadSize = ArrowHeadSize * Scale * 8.f;
	const FVector ConeStart = End - (Direction3D * HugeHeadSize);

	// Outer yellow cone (largest).
	DrawDebugCone(World, ConeStart, Direction3D, HugeHeadSize, FMath::DegreesToRadians(30.f), FMath::DegreesToRadians(30.f), 24, FColor::Yellow, false, Duration, 0, 4.f * Scale);

	// Black outline cone (slightly larger and offset back for depth effect).
	DrawDebugCone(World, ConeStart - Direction3D * (5.f * Scale), Direction3D, HugeHeadSize * 1.1f, FMath::DegreesToRadians(32.f), FMath::DegreesToRadians(32.f), 24, FColor::Black, false, Duration, 1, 2.f * Scale);

	// Inner red cone (smallest, for emphasis).
	DrawDebugCone(World, ConeStart, Direction3D, HugeHeadSize * 0.7f, FMath::DegreesToRadians(25.f), FMath::DegreesToRadians(25.f), 16, FColor::Red, false, Duration, 2, 3.f * Scale);

	// Draw target spheres at arrow tip.
	DrawDebugSphere(World, End, 80.f * Scale, 16, FColor::Orange, false, Duration, 0, 5.f * Scale);
	DrawDebugSphere(World, End, 50.f * Scale, 12, FColor::Red, false, Duration, 1, 4.f * Scale);

	// Draw radial spokes from sphere (8 directions, creates star pattern).
	for (int32 Index = 0; Index < 8; Index++)
	{
		const float AngleDeg = (360.f / 8.f) * Index;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);
		const FVector Offset(FMath::Cos(AngleRad) * 100.f * Scale, FMath::Sin(AngleRad) * 100.f * Scale, 0.f);

		DrawDebugLine(World, End + Offset, End, FColor::Yellow, false, Duration, 0, 2.f * Scale);
	}

	// Draw crosshair at start point for reference.
	const float Cross = ArrowLength * 0.25f * Scale;
	FVector CrossX(Cross, 0, 0);
	FVector CrossY(0, Cross, 0);

	// Rotate crosshair to match landscape orientation.
	CrossX = LandscapeQuat.RotateVector(CrossX);
	CrossY = LandscapeQuat.RotateVector(CrossY);

	// Draw horizontal and vertical crosshair lines.
	DrawDebugLine(World, Start - CrossX, Start + CrossX, FColor(128, 128, 128), false, Duration, 0, 1.f * Scale);
	DrawDebugLine(World, Start - CrossY, Start + CrossY, FColor(128, 128, 128), false, Duration, 0, 1.f * Scale);

	// Optionally draw wind cone showing directional spread.
	if (bAlsoDrawCone && ErosionSettings.bWindBias)
	{
		const int32 Spokes = 6;

		// Draw arrows at different angles within the cone.
		for (int32 Index = 0; Index <= Spokes; Index++)
		{
			// Calculate angle within cone range.
			const float NormalizedIndex = static_cast<float>(Index) / static_cast<float>(Spokes);
			const float Deg = MeanDeg - ConeHalfAngleDeg + (2.f * ConeHalfAngleDeg) * NormalizedIndex;

			// Convert angle to direction vector.
			Direction2D = UErosionLibrary::GetWindUnitVectorFromAngle(Deg);
			Direction3D = FVector(Direction2D.X, Direction2D.Y, 0.f);

			// Rotate to match landscape orientation.
			Direction3D = LandscapeQuat.RotateVector(Direction3D);

			// Draw directional arrow for this spoke.
			const FVector SEnd = Start + Direction3D * (ArrowLength * 0.85f * Scale);
			DrawDebugDirectionalArrow(World, Start, SEnd, ArrowHeadSize * 0.75f * Scale, FColor(0, 200, 255), false, Duration, 0, ArrowThickness * 0.75f * Scale);
		}
	}
#endif
}

#pragma endregion