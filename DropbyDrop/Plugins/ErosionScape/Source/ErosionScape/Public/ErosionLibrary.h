// • Manuel Solano

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ErosionLibrary.generated.h"

#pragma region DataStructures

struct FErosionSettings;

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

public:	
	static void SetHeights(FErosionContext& ErosionContext, const TArray<float>& InHeights);
	static TArray<float> GetHeights(const FErosionContext& ErosionContext);

	static void Erosion(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const int32 GridSize);
	
	static bool TryGetWindMeanAngleDegrees(const FErosionSettings& ErosionSettings, float& NormalizedAngle);
	static FVector2D GetWindUnitVectorFromAngle(float Degrees);
	
private:
	static void ApplyErosion(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, FDrop& Drop, const int32 GridSize);
	
	static FDrop& InitDrop(const FErosionSettings& ErosionSettings, FDrop& Drop, const int32 GridSize);
	static FDrop& SetDrop(const FErosionSettings& ErosionSettings, FDrop& Drop, const int32 GridSize, const FVector2D& Position, const FVector2D& Direction, const float Velocity, const float Water);
	
	static void InitWeights(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const int32 GridSize);

	static FVector2D ComputeGradient(const float P1, const float P2, const float P3, const float P4);
	static FVector2D ComputeGradientByInterpolate(const FVector2D& OffsetPosition, const float F1, const float F2, const float F3, const float F4);
	static FCornersHeights& SetCornersHeights(const FErosionContext& ErosionContext, FCornersHeights& InCornersHeights, const FVector2D& TruncatedPosition, const int32 GridSize);
	static float GetBilinearInterpolation(const FVector2D& OffsetPosition, const FCornersHeights& CornersHeights);

	static void SetInRadiusPoints(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const int32 GridSize);
	static float GetRelativeWeightOnPoint(const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const FVector2D& PointPosition);

	static TArray<float> GetErosionOnPoints(const FErosionContext& ErosionContext, const float ErosionFactor);
	static void ComputeDepositOnPoints(FErosionContext& ErosionContext, const FVector2D& IntegerPosition, const FVector2D& OffsetPosition, const float Deposit, const int32 GridSize);

	static bool IsOutOfBound(const FVector2D& DropPosition, const int32 GridSize);
	static EOutOfBoundResult GetOutOfBoundAsResult(const FVector2D& IntegerPosition, const int32 GridSize);

	static FVector2D GetWindDirection(const FErosionSettings& ErosionSettings);
};