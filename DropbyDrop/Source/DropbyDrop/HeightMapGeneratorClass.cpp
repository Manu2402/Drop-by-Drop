#include "HeightMapGeneratorClass.h"
#include "ImageUtils.h"
#include "Engine/TextureRenderTarget2D.h"

TArray<float> AHeightMapGeneratorClass::GenerateHeightMapCPU(int32 MapSize)
{
	TArray<float> HeightMap;
	HeightMap.SetNum(MapSize * MapSize);

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
				FVector2D P = Offsets[i] + FVector2D(x, y) / static_cast<float>(MapSize) * Scale; //Create a vector(x,y) contains the current position of the pixel in the map end normalized the vector {0,1}
				NoiseValue += FMath::PerlinNoise2D(P) * Weight; //Value of Perlin Noise in position P
				Weight *= Persistence; //Muliply the weight with the current with of octave
				Scale *= Lacunarity; //Scale the position P with the frequence of the octave
			}

			HeightMap[y * MapSize + x] = NoiseValue;
			MinValue = FMath::Min(MinValue, NoiseValue);
			MaxValue = FMath::Max(MaxValue, NoiseValue);
		}
	}
	UE_LOG(LogTemp, Log, TEXT("Min noise value: %f, Max noise value: %f"), MinValue, MaxValue);

	// Normalize heightmap values (if necessary)
	if (MinValue != MaxValue)
	{
		for (int32 i = 0; i < HeightMap.Num(); i++)
		{
			HeightMap[i] = (HeightMap[i] - MinValue) / (MaxValue - MinValue);
		}
	}
	// Log some sample heightmap values for debugging
	for (int32 i = 0; i < 10; ++i)
	{
		UE_LOG(LogTemp, Log, TEXT("HeightMap[%d]: %f"), i, HeightMap[i]);
	}

	return HeightMap;
}

UTexture2D* AHeightMapGeneratorClass::CreateHeightMapTexture(const TArray<float>& HeightMapData, int32 Width, int32 Height)
{
	//Create Empty Texture2D          CreateTransient -> Used to create texture in execute, the texture create in Transient can be modificated in execute.
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_R8G8B8A8/* */);
	Texture->SRGB = false; //Standard Red Green Blue, is false becouse the heightmap interpreted the grey color for height not like color.
	Texture->MipGenSettings = TMGS_NoMipmaps; //No multiply mipMaps

	//Ref to the first MipMap
	FTexture2DMipMap& MipMap = Texture->GetPlatformData()->Mips[0];
	MipMap.SizeX = Width;
	MipMap.SizeY = Height;

	// Locks the mipmap data for read and write access, returning a raw byte pointer.
	uint8* MipData = static_cast<uint8*>(MipMap.BulkData.Lock(LOCK_READ_WRITE));
	// Reinterprets the raw byte data as an array of colors, allowing direct pixel manipulation.
	FColor* DestPtr = reinterpret_cast<FColor*>(MipData);

	// Copia i dati elemento per elemento
	for (int32 i = 0; i < HeightMapData.Num(); ++i) // Iterate over each height value in the heightmap
	{
		float NormalizedValue = HeightMapData[i]; // Get the normalized height value
		uint8 GrayValue = FMath::Clamp(static_cast<uint8>(NormalizedValue * 255.0f), 0, 255); // Convert the normalized value to a grayscale value between 0 and 255
		DestPtr[i] = FColor(GrayValue, GrayValue, GrayValue, 255); // Assign the grayscale color to the corresponding pixel in the texture

		// Log the values for debugging
		UE_LOG(LogTemp, Log, TEXT("HeightMapData[%d]: %f, Normalized: %f, GrayValue: %d"), i, HeightMapData[i], NormalizedValue, GrayValue);
	}
	//ESSENTIAL! Unlock the mipMap Data
	MipMap.BulkData.Unlock();
	Texture->UpdateResource();

	return Texture;
}

void AHeightMapGeneratorClass::SaveTextureToFile(UTexture2D* Texture, const FString& FilePath)
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

	// Crea un render target
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

	// Copia i dati dalla texture al render target
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
			RHICmdList.CopyTexture(Texture->GetResource()->TextureRHI, RenderTarget->GetResource()->TextureRHI, CopyInfo);
		}
		);

	// Assicura che tutte le operazioni di rendering siano complete
	FlushRenderingCommands(); //Forza la chiusura di comandi di rendering pendenti

	// Esporta la texture come PNG
	TArray<FColor> Bitmap;
	FRenderTarget* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
	if (!RenderTargetResource)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get render target resource"));
		return;
	}
	RenderTargetResource->ReadPixels(Bitmap);

	/*
		// Stampa i valori dei pixel per il debug
		for (int32 y = 0; y < Texture->GetSizeY(); y++)
		{
			for (int32 x = 0; x < Texture->GetSizeX(); x++)
			{
				FColor PixelColor = Bitmap[y * Texture->GetSizeX() + x];
				UE_LOG(LogTemp, Log, TEXT("Pixel[%d][%d]: R=%d, G=%d, B=%d, A=%d"), x, y, PixelColor.R, PixelColor.G, PixelColor.B, PixelColor.A);
			}
		}

		// Salva il bitmap in un file PNG
		TArray64<uint8> CompressedBitmap;
		// ReSharper disable once CppDeprecatedEntity
		FImageUtils::PNGCompressImageArray(Texture->GetSizeX(), Texture->GetSizeY(), Bitmap, CompressedBitmap);
		*/

		// Check if Bitmap has the expected number of pixels
	int32 ExpectedPixelCount = Texture->GetSizeX() * Texture->GetSizeY();
	if (Bitmap.Num() != ExpectedPixelCount)
	{
		UE_LOG(LogTemp, Error, TEXT("Bitmap size mismatch. Expected: %d, Actual: %d"), ExpectedPixelCount, Bitmap.Num());
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

void AHeightMapGeneratorClass::CreateHeightMap(int32 MapSize)
{
	TArray<float> HeightMap = GenerateHeightMapCPU(MapSize);
	UTexture2D* Texture = CreateHeightMapTexture(HeightMap, MapSize, MapSize);
	FString FilePath = FPaths::ProjectDir() / TEXT("Saved/HeightMap/Heightmap.png");
	SaveTextureToFile(Texture, FilePath);
}


