// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "DropbyDrop/HeightMapGeneratorClass.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeHeightMapGeneratorClass() {}
// Cross Module References
	DROPBYDROP_API UClass* Z_Construct_UClass_AHeightMapGeneratorClass();
	DROPBYDROP_API UClass* Z_Construct_UClass_AHeightMapGeneratorClass_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_AActor();
	ENGINE_API UClass* Z_Construct_UClass_UTexture2D_NoRegister();
	UPackage* Z_Construct_UPackage__Script_DropbyDrop();
// End Cross Module References
	DEFINE_FUNCTION(AHeightMapGeneratorClass::execCreateHeightMap)
	{
		P_GET_PROPERTY(FIntProperty,Z_Param_MapSize);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->CreateHeightMap(Z_Param_MapSize);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AHeightMapGeneratorClass::execSaveTextureToFile)
	{
		P_GET_OBJECT(UTexture2D,Z_Param_Texture);
		P_GET_PROPERTY(FStrProperty,Z_Param_FilePath);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->SaveTextureToFile(Z_Param_Texture,Z_Param_FilePath);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AHeightMapGeneratorClass::execCreateHeightMapTexture)
	{
		P_GET_TARRAY_REF(float,Z_Param_Out_HeightMapData);
		P_GET_PROPERTY(FIntProperty,Z_Param_Width);
		P_GET_PROPERTY(FIntProperty,Z_Param_Height);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(UTexture2D**)Z_Param__Result=P_THIS->CreateHeightMapTexture(Z_Param_Out_HeightMapData,Z_Param_Width,Z_Param_Height);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AHeightMapGeneratorClass::execGenerateHeightMapCPU)
	{
		P_GET_PROPERTY(FIntProperty,Z_Param_MapSize);
		P_FINISH;
		P_NATIVE_BEGIN;
		*(TArray<float>*)Z_Param__Result=P_THIS->GenerateHeightMapCPU(Z_Param_MapSize);
		P_NATIVE_END;
	}
	void AHeightMapGeneratorClass::StaticRegisterNativesAHeightMapGeneratorClass()
	{
		UClass* Class = AHeightMapGeneratorClass::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "CreateHeightMap", &AHeightMapGeneratorClass::execCreateHeightMap },
			{ "CreateHeightMapTexture", &AHeightMapGeneratorClass::execCreateHeightMapTexture },
			{ "GenerateHeightMapCPU", &AHeightMapGeneratorClass::execGenerateHeightMapCPU },
			{ "SaveTextureToFile", &AHeightMapGeneratorClass::execSaveTextureToFile },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics
	{
		struct HeightMapGeneratorClass_eventCreateHeightMap_Parms
		{
			int32 MapSize;
		};
		static const UECodeGen_Private::FIntPropertyParams NewProp_MapSize;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::NewProp_MapSize = { "MapSize", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventCreateHeightMap_Parms, MapSize), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::NewProp_MapSize,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::Function_MetaDataParams[] = {
		{ "Category", "HeightMap" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "//Main Function\n" },
#endif
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Main Function" },
#endif
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AHeightMapGeneratorClass, nullptr, "CreateHeightMap", nullptr, nullptr, Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::PropPointers), sizeof(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::HeightMapGeneratorClass_eventCreateHeightMap_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::Function_MetaDataParams), Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::Function_MetaDataParams) };
	static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::PropPointers) < 2048);
	static_assert(sizeof(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::HeightMapGeneratorClass_eventCreateHeightMap_Parms) < MAX_uint16);
	UFunction* Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics
	{
		struct HeightMapGeneratorClass_eventCreateHeightMapTexture_Parms
		{
			TArray<float> HeightMapData;
			int32 Width;
			int32 Height;
			UTexture2D* ReturnValue;
		};
		static const UECodeGen_Private::FFloatPropertyParams NewProp_HeightMapData_Inner;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_HeightMapData_MetaData[];
#endif
		static const UECodeGen_Private::FArrayPropertyParams NewProp_HeightMapData;
		static const UECodeGen_Private::FIntPropertyParams NewProp_Width;
		static const UECodeGen_Private::FIntPropertyParams NewProp_Height;
		static const UECodeGen_Private::FObjectPropertyParams NewProp_ReturnValue;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_HeightMapData_Inner = { "HeightMapData", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_HeightMapData_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_HeightMapData = { "HeightMapData", nullptr, (EPropertyFlags)0x0010000008000182, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventCreateHeightMapTexture_Parms, HeightMapData), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_HeightMapData_MetaData), Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_HeightMapData_MetaData) };
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_Width = { "Width", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventCreateHeightMapTexture_Parms, Width), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_Height = { "Height", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventCreateHeightMapTexture_Parms, Height), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventCreateHeightMapTexture_Parms, ReturnValue), Z_Construct_UClass_UTexture2D_NoRegister, METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_HeightMapData_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_HeightMapData,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_Width,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_Height,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::Function_MetaDataParams[] = {
		{ "Category", "HeightMap" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "//Function to generate texture2D, using an array<float>\n" },
#endif
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Function to generate texture2D, using an array<float>" },
#endif
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AHeightMapGeneratorClass, nullptr, "CreateHeightMapTexture", nullptr, nullptr, Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::PropPointers), sizeof(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::HeightMapGeneratorClass_eventCreateHeightMapTexture_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::Function_MetaDataParams), Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::Function_MetaDataParams) };
	static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::PropPointers) < 2048);
	static_assert(sizeof(Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::HeightMapGeneratorClass_eventCreateHeightMapTexture_Parms) < MAX_uint16);
	UFunction* Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics
	{
		struct HeightMapGeneratorClass_eventGenerateHeightMapCPU_Parms
		{
			int32 MapSize;
			TArray<float> ReturnValue;
		};
		static const UECodeGen_Private::FIntPropertyParams NewProp_MapSize;
		static const UECodeGen_Private::FFloatPropertyParams NewProp_ReturnValue_Inner;
		static const UECodeGen_Private::FArrayPropertyParams NewProp_ReturnValue;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::NewProp_MapSize = { "MapSize", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventGenerateHeightMapCPU_Parms, MapSize), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::NewProp_ReturnValue_Inner = { "ReturnValue", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::NewProp_ReturnValue = { "ReturnValue", nullptr, (EPropertyFlags)0x0010000000000580, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventGenerateHeightMapCPU_Parms, ReturnValue), EArrayPropertyFlags::None, METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::NewProp_MapSize,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::NewProp_ReturnValue_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::NewProp_ReturnValue,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::Function_MetaDataParams[] = {
		{ "Category", "Heightmap" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "// Function to generate heightmap on CPU, generates a heightmap as a float array with the specified map size using Perlin noise.\n" },
#endif
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Function to generate heightmap on CPU, generates a heightmap as a float array with the specified map size using Perlin noise." },
#endif
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AHeightMapGeneratorClass, nullptr, "GenerateHeightMapCPU", nullptr, nullptr, Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::PropPointers), sizeof(Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::HeightMapGeneratorClass_eventGenerateHeightMapCPU_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::Function_MetaDataParams), Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::Function_MetaDataParams) };
	static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::PropPointers) < 2048);
	static_assert(sizeof(Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::HeightMapGeneratorClass_eventGenerateHeightMapCPU_Parms) < MAX_uint16);
	UFunction* Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics
	{
		struct HeightMapGeneratorClass_eventSaveTextureToFile_Parms
		{
			UTexture2D* Texture;
			FString FilePath;
		};
		static const UECodeGen_Private::FObjectPropertyParams NewProp_Texture;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_FilePath_MetaData[];
#endif
		static const UECodeGen_Private::FStrPropertyParams NewProp_FilePath;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::NewProp_Texture = { "Texture", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventSaveTextureToFile_Parms, Texture), Z_Construct_UClass_UTexture2D_NoRegister, METADATA_PARAMS(0, nullptr) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::NewProp_FilePath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::NewProp_FilePath = { "FilePath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapGeneratorClass_eventSaveTextureToFile_Parms, FilePath), METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::NewProp_FilePath_MetaData), Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::NewProp_FilePath_MetaData) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::NewProp_Texture,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::NewProp_FilePath,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::Function_MetaDataParams[] = {
		{ "Category", "HeightMap" },
#if !UE_BUILD_SHIPPING
		{ "Comment", "//Function to generate PNG grayscale from texture2D\n" },
#endif
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "ToolTip", "Function to generate PNG grayscale from texture2D" },
#endif
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AHeightMapGeneratorClass, nullptr, "SaveTextureToFile", nullptr, nullptr, Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::PropPointers), sizeof(Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::HeightMapGeneratorClass_eventSaveTextureToFile_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::Function_MetaDataParams), Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::Function_MetaDataParams) };
	static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::PropPointers) < 2048);
	static_assert(sizeof(Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::HeightMapGeneratorClass_eventSaveTextureToFile_Parms) < MAX_uint16);
	UFunction* Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AHeightMapGeneratorClass);
	UClass* Z_Construct_UClass_AHeightMapGeneratorClass_NoRegister()
	{
		return AHeightMapGeneratorClass::StaticClass();
	}
	struct Z_Construct_UClass_AHeightMapGeneratorClass_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_Seed_MetaData[];
#endif
		static const UECodeGen_Private::FIntPropertyParams NewProp_Seed;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_bRandomizeSeed_MetaData[];
#endif
		static void NewProp_bRandomizeSeed_SetBit(void* Obj);
		static const UECodeGen_Private::FBoolPropertyParams NewProp_bRandomizeSeed;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_NumOctaves_MetaData[];
#endif
		static const UECodeGen_Private::FIntPropertyParams NewProp_NumOctaves;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_Persistence_MetaData[];
#endif
		static const UECodeGen_Private::FFloatPropertyParams NewProp_Persistence;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_Lacunarity_MetaData[];
#endif
		static const UECodeGen_Private::FFloatPropertyParams NewProp_Lacunarity;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_InitialScale_MetaData[];
#endif
		static const UECodeGen_Private::FFloatPropertyParams NewProp_InitialScale;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_AHeightMapGeneratorClass_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_AActor,
		(UObject* (*)())Z_Construct_UPackage__Script_DropbyDrop,
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::DependentSingletons) < 16);
	const FClassFunctionLinkInfo Z_Construct_UClass_AHeightMapGeneratorClass_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMap, "CreateHeightMap" }, // 2361869822
		{ &Z_Construct_UFunction_AHeightMapGeneratorClass_CreateHeightMapTexture, "CreateHeightMapTexture" }, // 1265222271
		{ &Z_Construct_UFunction_AHeightMapGeneratorClass_GenerateHeightMapCPU, "GenerateHeightMapCPU" }, // 1645195802
		{ &Z_Construct_UFunction_AHeightMapGeneratorClass_SaveTextureToFile, "SaveTextureToFile" }, // 2730270610
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::FuncInfo) < 2048);
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapGeneratorClass_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "HeightMapGeneratorClass.h" },
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
	};
#endif
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Seed_MetaData[] = {
		{ "Category", "Heightmap Generation" },
		{ "DisplayName", "Random Seed" },
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "Tooltip", "Seed used to initialize the random number generator." },
#endif
	};
#endif
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Seed = { "Seed", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AHeightMapGeneratorClass, Seed), METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Seed_MetaData), Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Seed_MetaData) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_bRandomizeSeed_MetaData[] = {
		{ "Category", "Heightmap Generation" },
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "Tooltip", "Determines whether a random seed is generated for each heightmap. If false, the specified Seed value will be used." },
#endif
	};
#endif
	void Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_bRandomizeSeed_SetBit(void* Obj)
	{
		((AHeightMapGeneratorClass*)Obj)->bRandomizeSeed = 1;
	}
	const UECodeGen_Private::FBoolPropertyParams Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_bRandomizeSeed = { "bRandomizeSeed", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Bool | UECodeGen_Private::EPropertyGenFlags::NativeBool, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, sizeof(bool), sizeof(AHeightMapGeneratorClass), &Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_bRandomizeSeed_SetBit, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_bRandomizeSeed_MetaData), Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_bRandomizeSeed_MetaData) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_NumOctaves_MetaData[] = {
		{ "Category", "Heightmap Generation" },
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "Tooltip", "The number of octaves used in Perlin noise generation. More octaves result in more detailed heightmaps." },
#endif
	};
#endif
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_NumOctaves = { "NumOctaves", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AHeightMapGeneratorClass, NumOctaves), METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_NumOctaves_MetaData), Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_NumOctaves_MetaData) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Persistence_MetaData[] = {
		{ "Category", "Heightmap Generation" },
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "Tooltip", "Controls the amplitude of successive octaves. Lower values create smoother heightmaps." },
#endif
	};
#endif
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Persistence = { "Persistence", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AHeightMapGeneratorClass, Persistence), METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Persistence_MetaData), Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Persistence_MetaData) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Lacunarity_MetaData[] = {
		{ "Category", "Heightmap Generation" },
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "Tooltip", "Controls the frequency of successive octaves. Higher values create more detailed, higher frequency features." },
#endif
	};
#endif
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Lacunarity = { "Lacunarity", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AHeightMapGeneratorClass, Lacunarity), METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Lacunarity_MetaData), Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Lacunarity_MetaData) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_InitialScale_MetaData[] = {
		{ "Category", "Heightmap Generation" },
		{ "ModuleRelativePath", "HeightMapGeneratorClass.h" },
#if !UE_BUILD_SHIPPING
		{ "Tooltip", "The initial scale applied to the Perlin noise calculation. Controls the overall size of features in the heightmap." },
#endif
	};
#endif
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_InitialScale = { "InitialScale", nullptr, (EPropertyFlags)0x0010000000000005, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AHeightMapGeneratorClass, InitialScale), METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_InitialScale_MetaData), Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_InitialScale_MetaData) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AHeightMapGeneratorClass_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Seed,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_bRandomizeSeed,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_NumOctaves,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Persistence,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_Lacunarity,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AHeightMapGeneratorClass_Statics::NewProp_InitialScale,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_AHeightMapGeneratorClass_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AHeightMapGeneratorClass>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_AHeightMapGeneratorClass_Statics::ClassParams = {
		&AHeightMapGeneratorClass::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		Z_Construct_UClass_AHeightMapGeneratorClass_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::PropPointers),
		0,
		0x009000A4u,
		METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::Class_MetaDataParams), Z_Construct_UClass_AHeightMapGeneratorClass_Statics::Class_MetaDataParams)
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapGeneratorClass_Statics::PropPointers) < 2048);
	UClass* Z_Construct_UClass_AHeightMapGeneratorClass()
	{
		if (!Z_Registration_Info_UClass_AHeightMapGeneratorClass.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AHeightMapGeneratorClass.OuterSingleton, Z_Construct_UClass_AHeightMapGeneratorClass_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_AHeightMapGeneratorClass.OuterSingleton;
	}
	template<> DROPBYDROP_API UClass* StaticClass<AHeightMapGeneratorClass>()
	{
		return AHeightMapGeneratorClass::StaticClass();
	}
	AHeightMapGeneratorClass::AHeightMapGeneratorClass(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
	DEFINE_VTABLE_PTR_HELPER_CTOR(AHeightMapGeneratorClass);
	AHeightMapGeneratorClass::~AHeightMapGeneratorClass() {}
	struct Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapGeneratorClass_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapGeneratorClass_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_AHeightMapGeneratorClass, AHeightMapGeneratorClass::StaticClass, TEXT("AHeightMapGeneratorClass"), &Z_Registration_Info_UClass_AHeightMapGeneratorClass, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AHeightMapGeneratorClass), 4000139727U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapGeneratorClass_h_2994453756(TEXT("/Script/DropbyDrop"),
		Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapGeneratorClass_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapGeneratorClass_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
