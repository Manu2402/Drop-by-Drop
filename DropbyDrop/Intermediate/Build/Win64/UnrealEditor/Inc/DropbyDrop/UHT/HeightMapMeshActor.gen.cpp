// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "DropbyDrop/HeightMapMeshActor.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeHeightMapMeshActor() {}
// Cross Module References
	DROPBYDROP_API UClass* Z_Construct_UClass_AHeightMapMeshActor();
	DROPBYDROP_API UClass* Z_Construct_UClass_AHeightMapMeshActor_NoRegister();
	ENGINE_API UClass* Z_Construct_UClass_AActor();
	PROCEDURALMESHCOMPONENT_API UClass* Z_Construct_UClass_UProceduralMeshComponent_NoRegister();
	UPackage* Z_Construct_UPackage__Script_DropbyDrop();
// End Cross Module References
	DEFINE_FUNCTION(AHeightMapMeshActor::execGenerateMeshFromPng)
	{
		P_GET_PROPERTY(FStrProperty,Z_Param_FilePath);
		P_GET_PROPERTY(FIntProperty,Z_Param_MapSize);
		P_GET_PROPERTY(FFloatProperty,Z_Param_MaxHeight);
		P_GET_PROPERTY(FFloatProperty,Z_Param_ScaleXY);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->GenerateMeshFromPng(Z_Param_FilePath,Z_Param_MapSize,Z_Param_MaxHeight,Z_Param_ScaleXY);
		P_NATIVE_END;
	}
	DEFINE_FUNCTION(AHeightMapMeshActor::execGenerateMeshFromHeightMap)
	{
		P_GET_TARRAY_REF(float,Z_Param_Out_HeightMap);
		P_GET_PROPERTY(FIntProperty,Z_Param_MapSize);
		P_GET_PROPERTY(FFloatProperty,Z_Param_MaxHeight);
		P_GET_PROPERTY(FFloatProperty,Z_Param_ScaleXY);
		P_FINISH;
		P_NATIVE_BEGIN;
		P_THIS->GenerateMeshFromHeightMap(Z_Param_Out_HeightMap,Z_Param_MapSize,Z_Param_MaxHeight,Z_Param_ScaleXY);
		P_NATIVE_END;
	}
	void AHeightMapMeshActor::StaticRegisterNativesAHeightMapMeshActor()
	{
		UClass* Class = AHeightMapMeshActor::StaticClass();
		static const FNameNativePtrPair Funcs[] = {
			{ "GenerateMeshFromHeightMap", &AHeightMapMeshActor::execGenerateMeshFromHeightMap },
			{ "GenerateMeshFromPng", &AHeightMapMeshActor::execGenerateMeshFromPng },
		};
		FNativeFunctionRegistrar::RegisterFunctions(Class, Funcs, UE_ARRAY_COUNT(Funcs));
	}
	struct Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics
	{
		struct HeightMapMeshActor_eventGenerateMeshFromHeightMap_Parms
		{
			TArray<float> HeightMap;
			int32 MapSize;
			float MaxHeight;
			float ScaleXY;
		};
		static const UECodeGen_Private::FFloatPropertyParams NewProp_HeightMap_Inner;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_HeightMap_MetaData[];
#endif
		static const UECodeGen_Private::FArrayPropertyParams NewProp_HeightMap;
		static const UECodeGen_Private::FIntPropertyParams NewProp_MapSize;
		static const UECodeGen_Private::FFloatPropertyParams NewProp_MaxHeight;
		static const UECodeGen_Private::FFloatPropertyParams NewProp_ScaleXY;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_HeightMap_Inner = { "HeightMap", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, 0, METADATA_PARAMS(0, nullptr) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_HeightMap_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_HeightMap = { "HeightMap", nullptr, (EPropertyFlags)0x0010000008000182, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapMeshActor_eventGenerateMeshFromHeightMap_Parms, HeightMap), EArrayPropertyFlags::None, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_HeightMap_MetaData), Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_HeightMap_MetaData) };
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_MapSize = { "MapSize", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapMeshActor_eventGenerateMeshFromHeightMap_Parms, MapSize), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_MaxHeight = { "MaxHeight", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapMeshActor_eventGenerateMeshFromHeightMap_Parms, MaxHeight), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_ScaleXY = { "ScaleXY", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapMeshActor_eventGenerateMeshFromHeightMap_Parms, ScaleXY), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_HeightMap_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_HeightMap,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_MapSize,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_MaxHeight,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::NewProp_ScaleXY,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::Function_MetaDataParams[] = {
		{ "Category", "HeightMap" },
		{ "ModuleRelativePath", "HeightMapMeshActor.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AHeightMapMeshActor, nullptr, "GenerateMeshFromHeightMap", nullptr, nullptr, Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::PropPointers), sizeof(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::HeightMapMeshActor_eventGenerateMeshFromHeightMap_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04420401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::Function_MetaDataParams), Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::Function_MetaDataParams) };
	static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::PropPointers) < 2048);
	static_assert(sizeof(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::HeightMapMeshActor_eventGenerateMeshFromHeightMap_Parms) < MAX_uint16);
	UFunction* Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	struct Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics
	{
		struct HeightMapMeshActor_eventGenerateMeshFromPng_Parms
		{
			FString FilePath;
			int32 MapSize;
			float MaxHeight;
			float ScaleXY;
		};
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_FilePath_MetaData[];
#endif
		static const UECodeGen_Private::FStrPropertyParams NewProp_FilePath;
		static const UECodeGen_Private::FIntPropertyParams NewProp_MapSize;
		static const UECodeGen_Private::FFloatPropertyParams NewProp_MaxHeight;
		static const UECodeGen_Private::FFloatPropertyParams NewProp_ScaleXY;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Function_MetaDataParams[];
#endif
		static const UECodeGen_Private::FFunctionParams FuncParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_FilePath_MetaData[] = {
		{ "NativeConst", "" },
	};
#endif
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_FilePath = { "FilePath", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapMeshActor_eventGenerateMeshFromPng_Parms, FilePath), METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_FilePath_MetaData), Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_FilePath_MetaData) };
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_MapSize = { "MapSize", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapMeshActor_eventGenerateMeshFromPng_Parms, MapSize), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_MaxHeight = { "MaxHeight", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapMeshActor_eventGenerateMeshFromPng_Parms, MaxHeight), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FFloatPropertyParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_ScaleXY = { "ScaleXY", nullptr, (EPropertyFlags)0x0010000000000080, UECodeGen_Private::EPropertyGenFlags::Float, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(HeightMapMeshActor_eventGenerateMeshFromPng_Parms, ScaleXY), METADATA_PARAMS(0, nullptr) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_FilePath,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_MapSize,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_MaxHeight,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::NewProp_ScaleXY,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::Function_MetaDataParams[] = {
		{ "Category", "HeightMap" },
		{ "ModuleRelativePath", "HeightMapMeshActor.h" },
	};
#endif
	const UECodeGen_Private::FFunctionParams Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::FuncParams = { (UObject*(*)())Z_Construct_UClass_AHeightMapMeshActor, nullptr, "GenerateMeshFromPng", nullptr, nullptr, Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::PropPointers, UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::PropPointers), sizeof(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::HeightMapMeshActor_eventGenerateMeshFromPng_Parms), RF_Public|RF_Transient|RF_MarkAsNative, (EFunctionFlags)0x04020401, 0, 0, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::Function_MetaDataParams), Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::Function_MetaDataParams) };
	static_assert(UE_ARRAY_COUNT(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::PropPointers) < 2048);
	static_assert(sizeof(Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::HeightMapMeshActor_eventGenerateMeshFromPng_Parms) < MAX_uint16);
	UFunction* Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng()
	{
		static UFunction* ReturnFunction = nullptr;
		if (!ReturnFunction)
		{
			UECodeGen_Private::ConstructUFunction(&ReturnFunction, Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng_Statics::FuncParams);
		}
		return ReturnFunction;
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AHeightMapMeshActor);
	UClass* Z_Construct_UClass_AHeightMapMeshActor_NoRegister()
	{
		return AHeightMapMeshActor::StaticClass();
	}
	struct Z_Construct_UClass_AHeightMapMeshActor_Statics
	{
		static UObject* (*const DependentSingletons[])();
		static const FClassFunctionLinkInfo FuncInfo[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_Mesh_MetaData[];
#endif
		static const UECodeGen_Private::FObjectPropertyParams NewProp_Mesh;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_AHeightMapMeshActor_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_AActor,
		(UObject* (*)())Z_Construct_UPackage__Script_DropbyDrop,
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapMeshActor_Statics::DependentSingletons) < 16);
	const FClassFunctionLinkInfo Z_Construct_UClass_AHeightMapMeshActor_Statics::FuncInfo[] = {
		{ &Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromHeightMap, "GenerateMeshFromHeightMap" }, // 2168520334
		{ &Z_Construct_UFunction_AHeightMapMeshActor_GenerateMeshFromPng, "GenerateMeshFromPng" }, // 3167461072
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapMeshActor_Statics::FuncInfo) < 2048);
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapMeshActor_Statics::Class_MetaDataParams[] = {
		{ "IncludePath", "HeightMapMeshActor.h" },
		{ "ModuleRelativePath", "HeightMapMeshActor.h" },
	};
#endif
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AHeightMapMeshActor_Statics::NewProp_Mesh_MetaData[] = {
		{ "Category", "Mesh" },
		{ "EditInline", "true" },
		{ "ModuleRelativePath", "HeightMapMeshActor.h" },
	};
#endif
	const UECodeGen_Private::FObjectPropertyParams Z_Construct_UClass_AHeightMapMeshActor_Statics::NewProp_Mesh = { "Mesh", nullptr, (EPropertyFlags)0x001000000008000d, UECodeGen_Private::EPropertyGenFlags::Object, RF_Public|RF_Transient|RF_MarkAsNative, nullptr, nullptr, 1, STRUCT_OFFSET(AHeightMapMeshActor, Mesh), Z_Construct_UClass_UProceduralMeshComponent_NoRegister, METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapMeshActor_Statics::NewProp_Mesh_MetaData), Z_Construct_UClass_AHeightMapMeshActor_Statics::NewProp_Mesh_MetaData) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UClass_AHeightMapMeshActor_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UClass_AHeightMapMeshActor_Statics::NewProp_Mesh,
	};
	const FCppClassTypeInfoStatic Z_Construct_UClass_AHeightMapMeshActor_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AHeightMapMeshActor>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_AHeightMapMeshActor_Statics::ClassParams = {
		&AHeightMapMeshActor::StaticClass,
		"Engine",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		FuncInfo,
		Z_Construct_UClass_AHeightMapMeshActor_Statics::PropPointers,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		UE_ARRAY_COUNT(FuncInfo),
		UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapMeshActor_Statics::PropPointers),
		0,
		0x009000A4u,
		METADATA_PARAMS(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapMeshActor_Statics::Class_MetaDataParams), Z_Construct_UClass_AHeightMapMeshActor_Statics::Class_MetaDataParams)
	};
	static_assert(UE_ARRAY_COUNT(Z_Construct_UClass_AHeightMapMeshActor_Statics::PropPointers) < 2048);
	UClass* Z_Construct_UClass_AHeightMapMeshActor()
	{
		if (!Z_Registration_Info_UClass_AHeightMapMeshActor.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AHeightMapMeshActor.OuterSingleton, Z_Construct_UClass_AHeightMapMeshActor_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_AHeightMapMeshActor.OuterSingleton;
	}
	template<> DROPBYDROP_API UClass* StaticClass<AHeightMapMeshActor>()
	{
		return AHeightMapMeshActor::StaticClass();
	}
	DEFINE_VTABLE_PTR_HELPER_CTOR(AHeightMapMeshActor);
	AHeightMapMeshActor::~AHeightMapMeshActor() {}
	struct Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapMeshActor_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapMeshActor_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_AHeightMapMeshActor, AHeightMapMeshActor::StaticClass, TEXT("AHeightMapMeshActor"), &Z_Registration_Info_UClass_AHeightMapMeshActor, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AHeightMapMeshActor), 2261158221U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapMeshActor_h_4107471816(TEXT("/Script/DropbyDrop"),
		Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapMeshActor_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_39345_Desktop_FORK_Drop_by_Drop_DropbyDrop_Source_DropbyDrop_HeightMapMeshActor_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
