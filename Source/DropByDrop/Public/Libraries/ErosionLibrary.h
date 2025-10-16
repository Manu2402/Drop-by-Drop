// © Manuel Solano

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ErosionLibrary.generated.h"

#pragma region ForwardDeclarations

struct FErosionSettings;

#pragma endregion

#pragma region DataStructures

/**
 * Represents a water drop used in the hydraulic erosion simulation.
 * Each drop moves across the landscape, eroding and depositing sediment.
 */
struct FDrop
{
	/** Current position of the drop in 2D world space. */
	FVector2D Position;

	/** Movement direction of the drop in 2D world space. */
	FVector2D Direction;

	/** Current speed of the drop. */
	float Velocity;

	/** Amount of water volume that composes this drop. */
	float Water;
};

/**
 * Stores the height values of all four corners of a grid cell.
 * Used for bilinear interpolation and gradient calculations.
 */
struct FCornersHeights
{
	float X_Y;   // Top-Left Corner height.
	float X1_Y;  // Top-Right Corner height.
	float X_Y1;  // Bottom-Left Corner height.
	float X1_Y1; // Bottom-Right Corner height.
};

/**
 * Enumeration indicating whether a position is within bounds or which boundary it exceeds.
 * Used to handle edge cases during erosion simulation.
 */
enum EOutOfBoundResult : uint8
{
	No_Error,         // Position is within valid bounds.
	Error_Right,      // Position exceeds right boundary.
	Error_Down,       // Position exceeds bottom boundary.
	Error_Right_Down  // Position exceeds both right and bottom boundaries.
};

/**
 * Context structure containing all data needed for erosion simulation.
 * Maintains the heightmap state and temporary calculation data.
 */
struct FErosionContext
{
	/** Height values for each cell in the landscape grid. */
	TArray<float> GridHeights;

	/** Grid positions of cells affected by the current drop's movement. */
	TArray<FVector2D> Points; // pI

	/** Squared weight values for affected cells based on distance from drop position. */
	TArray<float> SquaredWeights; // wI
};

#pragma endregion

/**
 * Blueprint function library providing hydraulic erosion simulation utilities.
 * Implements particle-based erosion algorithm for landscape heightmap modification.
 */
UCLASS()
class UErosionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Sets the height values in the erosion context.
	 * @param ErosionContext - The erosion context to modify.
	 * @param InHeights - Array of height values to assign.
	 */
	static void SetHeights(FErosionContext& ErosionContext, const TArray<float>& InHeights);

	/**
	 * Gets the current height values from the erosion context.
	 * @param ErosionContext - The erosion context to read from.
	 * @return Array of height values.
	 */
	static TArray<float> GetHeights(const FErosionContext& ErosionContext);

	/**
	 * Performs hydraulic erosion simulation on the heightmap.
	 * @param ErosionContext - Context containing heightmap and working data.
	 * @param ErosionSettings - Settings controlling erosion behavior.
	 * @param GridSize - Size of the square grid of heights (width and height).
	 */
	static void Erosion(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const int32 GridSize);

	/**
	 * Get the normalized mean wind angle from erosion settings.
	 * @param ErosionSettings - Settings containing wind parameters.
	 * @param NormalizedAngle - Output parameter for the normalized angle in degrees.
	 * @return True if wind angle is randomless, false otherwise.
	 */
	static bool TryGetWindMeanAngleDegrees(const FErosionSettings& ErosionSettings, float& NormalizedAngle);

	/**
	 * Converts a wind angle in degrees to a unit direction vector.
	 * @param Degrees - Angle in degrees.
	 * @return Normalized 2D direction vector.
	 */
	static FVector2D GetWindUnitVectorFromAngle(const float Degrees);

private:
	/**
	 * Applies erosion effects for a single water drop simulation.
	 * @param ErosionContext - Context containing heightmap and working data.
	 * @param ErosionSettings - Settings controlling erosion behavior.
	 * @param Drop - The water drop being simulated.
	 * @param GridSize - Size of the square grid of heights.
	 */
	static void ApplyErosion(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, FDrop& Drop, const int32 GridSize);

	/**
	 * Initializes a water drop with starting position, direction, and properties.
	 * @param ErosionSettings - Settings containing initialization parameters.
	 * @param Drop - The drop to initialize.
	 * @param GridSize - Size of the square grid of heights.
	 * @return Reference to the initialized drop.
	 */
	static FDrop& InitDrop(const FErosionSettings& ErosionSettings, FDrop& Drop, const int32 GridSize);

	/**
	 * Initializes weight values for cells within the erosion radius of the drop.
	 * @param ErosionContext - Context to store calculated weights.
	 * @param ErosionSettings - Settings containing erosion radius.
	 * @param DropPosition - Current position of the drop.
	 * @param GridSize - Size of the square grid of heights.
	 */
	static void InitWeights(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const int32 GridSize);

	/**
	 * Computes the gradient vector from four corner height values.
	 * @param P1 - Height at first corner.
	 * @param P2 - Height at second corner.
	 * @param P3 - Height at third corner.
	 * @param P4 - Height at fourth corner.
	 * @return Gradient vector in 2D.
	 */
	static FVector2D ComputeGradient(const float P1, const float P2, const float P3, const float P4);

	/**
	 * Computes gradient using bilinear interpolation of four values.
	 * @param OffsetPosition - Position offset within the cell ([0, 1) range).
	 * @param F1 - First interpolation value.
	 * @param F2 - Second interpolation value.
	 * @param F3 - Third interpolation value.
	 * @param F4 - Fourth interpolation value.
	 * @return Interpolated gradient vector.
	 */
	static FVector2D ComputeGradientByInterpolate(const FVector2D& OffsetPosition, const float F1, const float F2, const float F3, const float F4);

	/**
	 * Retrieves and sets the height values for the four corners of a grid cell.
	 * @param ErosionContext - Context containing height data.
	 * @param InCornersHeights - Structure to populate with corner heights.
	 * @param TruncatedPosition - Integer grid position.
	 * @param GridSize - Size of the square grid of heights.
	 * @return Reference to the populated corner heights structure.
	 */
	static FCornersHeights& SetCornersHeights(const FErosionContext& ErosionContext, FCornersHeights& InCornersHeights, const FVector2D& TruncatedPosition, const int32 GridSize);

	/**
	 * Performs bilinear interpolation using corner heights and offset position.
	 * @param OffsetPosition - Position offset within the cell ([0, 1) range).
	 * @param CornersHeights - Heights at the four corners of the cell.
	 * @return Interpolated height value at the given position.
	 */
	static float GetBilinearInterpolation(const FVector2D& OffsetPosition, const FCornersHeights& CornersHeights);

	/**
	 * Identifies and stores all grid points within the erosion radius of the drop.
	 * @param ErosionContext - Context to store affected points.
	 * @param ErosionSettings - Settings containing erosion radius.
	 * @param DropPosition - Current position of the drop.
	 * @param GridSize - Size of the square grid of heights.
	 */
	static void SetInRadiusPoints(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const int32 GridSize);

	/**
	 * Calculates the relative weight of a point based on its distance from the drop.
	 * @param ErosionSettings - Settings containing erosion radius.
	 * @param DropPosition - Current position of the drop.
	 * @param PointPosition - Position of the point to evaluate.
	 * @return Weight value indicating influence strength.
	 */
	static float GetRelativeWeightOnPoint(const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const FVector2D& PointPosition);

	/**
	 * Calculates erosion amounts for all affected points based on weights.
	 * @param ErosionContext - Context containing points and weights.
	 * @param ErosionFactor - Multiplier for erosion strength.
	 * @return Array of erosion amounts for each affected point.
	 */
	static TArray<float> GetErosionOnPoints(const FErosionContext& ErosionContext, const float ErosionFactor);

	/**
	 * Distributes sediment deposit across nearby grid points using interpolation.
	 * @param ErosionContext - Context to modify with deposited sediment.
	 * @param IntegerPosition - Integer grid position.
	 * @param OffsetPosition - Fractional position within the cell.
	 * @param Deposit - Amount of sediment to deposit.
	 * @param GridSize - Size of the square grid of heights.
	 */
	static void ComputeDepositOnPoints(FErosionContext& ErosionContext, const FVector2D& IntegerPosition, const FVector2D& OffsetPosition, const float Deposit, const int32 GridSize);

	/**
	 * Checks if a position is outside the valid grid boundaries.
	 * @param DropPosition - Position to check.
	 * @param GridSize - Size of the square grid of heights.
	 * @return True if position is out of bounds, false otherwise.
	 */
	static bool IsOutOfBound(const FVector2D& DropPosition, const int32 GridSize);

	/**
	 * Determines which boundary (if any) a position exceeds.
	 * @param IntegerPosition - Integer grid position to check.
	 * @param GridSize - Size of the square grid of heights.
	 * @return Enumeration indicating which boundaries are exceeded.
	 */
	static EOutOfBoundResult GetOutOfBoundAsResult(const FVector2D& IntegerPosition, const int32 GridSize);

	/**
	 * Calculates wind direction vector from erosion settings.
	 * @param ErosionSettings - Settings containing wind parameters.
	 * @return 2D direction vector representing wind direction.
	 */
	static FVector2D GetWindDirection(const FErosionSettings& ErosionSettings);

};