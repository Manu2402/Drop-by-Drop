// • Manuel Solano

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ErosionLibrary.generated.h"

#pragma region DataStructures

struct FDrop
{
	// Position in the world.
	FVector2D Position;
	
	// Direction in the world.
	FVector2D Direction;

	// Amount of speed.
	float Velocity;

	// Amount of "water" that compose the drop.
	float Water;
};

// All corners of a grid cell on the landscapes.
struct FCornersHeights
{
	float X_Y;   // Top-Left Corner
	float X1_Y;  // Top-Right Corner
	float X_Y1;  // Bottom-Left Corner
	float X1_Y1; // Bottom-Right Corner
};

// Cell corners evaluation based on grid borders for erosion algorithm.
enum EOutOfBoundResult : uint8
{
	No_Error,
	Error_Right,
	Error_Down,
	Error_Right_Down
};

UENUM(BlueprintType)
enum EWindDirection : uint8
{
	Random,
	Est,
	Nord_Est,
	Nord,
	Nord_Ovest,
	Ovest,
	Sud_Ovest,
	Sud,
	Sud_Est
};

#pragma endregion // DataStructures

struct FErosionContext
{
	// Landscape's heights for each cell.
	TArray<float> GridHeights;

	// Cells involved into the erosion when the drop moves.
	TArray<FVector2D> Points; // pI
	
	// Weights (squared) of involved cells, based on their distance from the drop's position.
	TArray<float> SquaredWeights; // wI
};

UCLASS()
class UErosionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

// TOGLIERE
#pragma region Parameters

public:	
	static int32 ErosionCycles; // ErosionCycles
	static void SetErosion(const int32 NewErosionCycles)
	{
		ErosionCycles = NewErosionCycles;
	}
	static int32 GetErosionCycles()
	{
		return ErosionCycles;
	}
	
	static float Inertia; // pInertia
	static void SetInertia(const float NewInertia)
	{
		Inertia = NewInertia;
	}
	static float GetInertia()
	{
		return Inertia;
	}
	
	static float Capacity; // pCapacity
	static void SetCapacity(const float NewCapacity)
	{
		Capacity = NewCapacity;
	}
	static float GetCapacity()
	{
		return Capacity;
	}
	
	static float MinimalSlope; // pMinSlope
	static void SetMinimalSlope(const float NewMinimalSlope)
	{
		MinimalSlope = NewMinimalSlope;
	}
	static float GetMinimalSlope()
	{
		return MinimalSlope;
	}
	
	static float DepositionSpeed; // pDeposition
	static void SetDepositionSpeed(const float NewDepositionSpeed)
	{
		DepositionSpeed = NewDepositionSpeed;
	}
	static float GetDepositionSpeed()
	{
		return DepositionSpeed;
	}
	
	static float ErosionSpeed; // pErosion
	static void SetErosionSpeed(const float NewErosionSpeed)
	{
		ErosionSpeed = NewErosionSpeed;
	}
	static float GetErosionSpeed()
	{
		return ErosionSpeed;
	}
	
	static float Gravity; // pGravity
	static void SetGravity(const float NewGravity)
	{
		Gravity = NewGravity;
	}
	static float GetGravity()
	{
		return Gravity;
	}
	
	static float Evaporation; // pEvaporation
	static void SetEvaporation(const float NewEvaporation)
	{
		Evaporation = NewEvaporation;
	}
	static float GetEvaporation()
	{
		return Evaporation;
	}
	
	static float MaxPath; // pMaxPath
	static void SetMaxPath(const float NewMaxPath)
	{
		MaxPath = NewMaxPath;
	}
	static float GetMaxPath()
	{
		return MaxPath;
	}
	
	static int32 ErosionRadius; // pRadius
	static void SetErosionRadius(const int32 NewErosionRadius)
	{
		ErosionRadius = NewErosionRadius;
	}
	static int32 GetErosionRadius()
	{
		return ErosionRadius;
	}

	static EWindDirection WindDirection;
	static void SetWindDirection(const EWindDirection NewWindDirection)
	{
		WindDirection = NewWindDirection;
	}
	static EWindDirection GetWindDirectionTemp()
	{
		return WindDirection;
	}

	static bool WindBias;
	static void SetWindBias(const bool NewWindBias)
	{
		WindBias = NewWindBias;
	}
	static bool GetWindBias()
	{
		return WindBias;
	}

#pragma endregion

public:	
	static void SetHeights(FErosionContext& ErosionContext, const TArray<float>& InHeights); // ✔
	static TArray<float> GetHeights(const FErosionContext& ErosionContext); // ✔

	static void Erosion(FErosionContext& ErosionContext, const int32 GridSize); // ✔
	
	UFUNCTION(BlueprintCallable, Category="Erosion|Debug")
	static float GetWindMeanAngleDegrees();
	UFUNCTION(BlueprintCallable, Category="Erosion|Debug")
	static FVector2D GetWindUnitVectorFromAngle(float Degrees);
	
private:
	static void ApplyErosion(FErosionContext& ErosionContext, FDrop& Drop, const int32 GridSize);
	
	static FDrop& InitDrop(FDrop& Drop, const int32 GridSize); // ✔
	static FDrop& SetDrop(FDrop& Drop, const int32 GridSize, const FVector2D& Position, const FVector2D& Direction, const float Velocity, const float Water); // ✔
	
	static void InitWeights(FErosionContext& ErosionContext, const FVector2D& DropPosition, const int32 GridSize); // ✔

	static FVector2D ComputeGradient(const float P1, const float P2, const float P3, const float P4); // ✔
	static FVector2D ComputeGradientByInterpolate(const FVector2D& OffsetPosition, const float F1, const float F2, const float F3, const float F4); // ✔
	static FCornersHeights& SetCornersHeights(const FErosionContext& ErosionContext, FCornersHeights& InCornersHeights, const FVector2D& TruncatedPosition, const int32 GridSize); // ✔
	static float GetBilinearInterpolation(const FVector2D& OffsetPosition, const FCornersHeights& CornersHeights); // ✔

	static void SetInRadiusPoints(FErosionContext& ErosionContext, const FVector2D& DropPosition, const int32 GridSize); // ✔
	static float GetRelativeWeightOnPoint(const FVector2D& DropPosition, const FVector2D& PointPosition);

	static TArray<float> GetErosionOnPoints(const FErosionContext& ErosionContext, const float ErosionFactor);
	static void ComputeDepositOnPoints(FErosionContext& ErosionContext, const FVector2D& IntegerPosition, const FVector2D& OffsetPosition, const float Deposit, const int32 GridSize); // ✔

	static bool IsOutOfBound(const FVector2D& DropPosition, const int32 GridSize); // ✔
	static EOutOfBoundResult GetOutOfBoundAsResult(const FVector2D& IntegerPosition, const int32 GridSize); // ✔

	static FVector2D GetWindDirection(); // ✔ ~
};