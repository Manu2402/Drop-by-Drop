#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ErosionLibrary.generated.h"

USTRUCT(BlueprintType)
struct FDrop
{
	GENERATED_BODY()

	FVector2D Position;
	FVector2D Direction;
	float Velocity;
	float Water;
};

USTRUCT()
struct FPositionHeights
{
	GENERATED_BODY()

	float X_Y;
	float X1_Y;
	float X_Y1;
	float X1_Y1;
};

UENUM()
enum EOutOfBoundResult : uint8
{
	No_Error,
	Error_Right,
	Error_Down,
	Error_Right_Down
};

UCLASS(Blueprintable)
class UErosionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:	
	UErosionLibrary();
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// int32 CellSize;

#pragma region Parameters
	//UPROPERTY(EditAnywhere)
	static int32 ErosionCycles; // erosionCycles
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetErosion(const int32& NewErosionCycles)
	{
		ErosionCycles = NewErosionCycles;
	}
	static int32 GetErosionCycles()
	{
		return ErosionCycles;
	}
	
	//UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1")) // [0, 1]
	static float Inertia; // pInertia
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetInertia(const float& NewInertia)
	{
		Inertia = NewInertia;
	}
	static float GetInertia()
	{
		return Inertia;
	}
	
	//UPROPERTY(EditAnywhere)
	static float Capacity; // pCapacity
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetCapacity(const float& NewCapacity)
	{
		Capacity = NewCapacity;
	}
	static float GetCapacity()
	{
		return Capacity;
	}
	
	//UPROPERTY(EditAnywhere)
	static float MinimalSlope; // pMinSlope
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetMinimalSlope(const float& NewMinimalSlope)
	{
		MinimalSlope = NewMinimalSlope;
	}
	static float GetMinimalSlope()
	{
		return MinimalSlope;
	}
	
	//UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1"))
	static float DepositionSpeed; // pDeposition
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetDepositionSpeed(const float& NewDepositionSpeed)
	{
		DepositionSpeed = NewDepositionSpeed;
	}
	static float GetDepositionSpeed()
	{
		return DepositionSpeed;
	}
	
	//UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1"))
	static float ErosionSpeed; // pErosion
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetErosionSpeed(const float& NewErosionSpeed)
	{
		ErosionSpeed = NewErosionSpeed;
	}
	static float GetErosionSpeed()
	{
		return ErosionSpeed;
	}
	
	//UPROPERTY(EditAnywhere)
	static float Gravity; // pGravity
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetGravity(const float& NewGravity)
	{
		Gravity = NewGravity;
	}
	static float GetGravity()
	{
		return Gravity;
	}
	
	//UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1"))
	static float Evaporation; // pEvaporation
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetEvaporation(const float& NewEvaporation)
	{
		Evaporation = NewEvaporation;
	}
	static float GetEvaporation()
	{
		return Evaporation;
	}
	
	//UPROPERTY(EditAnywhere)
	static float MaxPath; // pMaxPath
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetMaxPath(const float& NewMaxPath)
	{
		MaxPath = NewMaxPath;
	}
	static float GetMaxPath()
	{
		return MaxPath;
	}
	
	//UPROPERTY(EditAnywhere)
	static int32 ErosionRadius;
	UFUNCTION(BlueprintCallable, Category = "Erosion")
	static void SetErosionRadius(const int32& NewErosionRadius)
	{
		ErosionRadius = NewErosionRadius;
	}// pRadius
	static int32 GetErosionRadius()
	{
		return ErosionRadius;
	}
#pragma endregion

public:	
	static void SetHeights(const TArray<float>& NewHeights);
	static TArray<float> GetHeights();

	//UFUNCTION(BlueprintCallable) Useless!
	//TArray<float> GenerateVirtualGrid(const TArray<float> MapHeightsValues, const int32 MapSize, const int32 NewCellSize);

	static void ErosionHandler(const int32& GridSize);

private:
	// TArray<float> MapHeights;
	static TArray<float> GridHeights;
	
	static TArray<FVector2D> Points; // pI
	static TArray<float> Weights; // wI

#pragma region InitDrop
	static FDrop GenerateDropInitialParams(const int32& GridSize);
	static FDrop SetDropInitialParams(const int32& GridSize, const FVector2D& Position, const FVector2D& Direction, const float& Velocity, const float& Water);
#pragma endregion

	static void Erosion(FDrop& Drop, const int32& GridSize);

	static void InitWeights(const FVector2D& DropPosition, const int32& GridSize);

#pragma region ErosionSubFunctions
	static FVector2D GetGradient(const float& P1, const float& P2, const float& P3, const float& P4);
	static FVector2D GetPairedLinearInterpolation(const FVector2D& OffsetPosition, const float& F1, const float& F2, const float& F3, const float& F4);
	static FPositionHeights GetPositionHeights(const FVector2D& IntegerPosition, const int32& GridSize);
	static float GetBilinearInterpolation(const FVector2D& OffsetPosition, const FPositionHeights& PositionHeights);
#pragma endregion

	static void SetPointsPositionInRadius(const FVector2D& DropPosition, const int32& GridSize);

#pragma region WeightsSubFunctions
	static float GetRelativeWeightOnPoint(const FVector2D& DropPosition, const FVector2D& PointPosition);
#pragma endregion

	static TArray<float> GetErosionOnPoints(const float& ErosionFactor);
	static void ComputeDepositOnPoints(const FVector2D& IntegerPosition, const FVector2D& OffsetPosition, const float& Deposit, const int32& GridSize);

	static EOutOfBoundResult GetOutOfBoundResult(const FVector2D& IntegerPosition, const int32& GridSize);
};
