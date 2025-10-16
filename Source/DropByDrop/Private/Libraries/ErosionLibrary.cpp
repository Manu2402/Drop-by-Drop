// © Manuel Solano

#include "Libraries/ErosionLibrary.h"

#include "DropByDropSettings.h"
#include "DropByDropLogger.h"

/**
 * Sets the height values in the erosion context.
 * Reserves memory and copies the provided height array.
 */
void UErosionLibrary::SetHeights(FErosionContext& ErosionContext, const TArray<float>& InHeights)
{
	ErosionContext.GridHeights.Reserve(InHeights.Num());
	ErosionContext.GridHeights = InHeights;
}

/**
 * Gets the current height values from the erosion context.
 * Returns a copy of the internal height array.
 */
TArray<float> UErosionLibrary::GetHeights(const FErosionContext& ErosionContext)
{
	return ErosionContext.GridHeights;
}

/**
 * Initializes a water drop with starting position, direction, and properties.
 * Position is randomly placed within grid bounds, direction is determined by wind settings.
 */
FDrop& UErosionLibrary::InitDrop(const FErosionSettings& ErosionSettings, FDrop& OutDrop, const int32 GridSize /* GridSize = MapSize / CellSize */)
{
	const float Limit = static_cast<float>(GridSize - 1);

	// Random starting position within valid grid bounds.
	OutDrop.Position = FVector2D(FMath::RandRange(0.f, Limit), FMath::RandRange(0.f, Limit));

	// Direction based on wind settings.
	OutDrop.Direction = GetWindDirection(ErosionSettings);

	// Initial velocity and water amount.
	OutDrop.Velocity = 1;
	OutDrop.Water = 1;

	return OutDrop;
}

/**
 * Computes the gradient vector from four corner height values.
 * Calculates the normalized directional derivative.
 */
FVector2D UErosionLibrary::ComputeGradient(const float P1, const float P2, const float P3, const float P4)
{
	// Normalization with safe check to avoid division by zero.
	return FVector2D(P1 - P2, P3 - P4).GetSafeNormal();
}

/**
 * Computes gradient using bilinear interpolation of four values.
 * Uses offset position within cell (u, v coordinates in [0-1) range).
 */
FVector2D UErosionLibrary::ComputeGradientByInterpolate(const FVector2D& OffsetPosition, const float F1, const float F2, const float F3, const float F4)
{
	// (x, y) = (u, v)
	// Linear interpolation along both axes.
	return FVector2D(
		F1 * (1 - OffsetPosition.Y) + F2 * OffsetPosition.Y,
		F3 * (1 - OffsetPosition.X) + F4 * OffsetPosition.X
	);
}

/**
 * Retrieves and sets the height values for the four corners of a grid cell.
 * Handles boundary cases where position is at or beyond grid edges.
 */
FCornersHeights& UErosionLibrary::SetCornersHeights(const FErosionContext& ErosionContext, FCornersHeights& InCornersHeights, const FVector2D& TruncatedPosition, const int32 GridSize)
{
	const EOutOfBoundResult OutOfBoundResult = GetOutOfBoundAsResult(TruncatedPosition, GridSize);

	switch (OutOfBoundResult)
	{
	case Error_Right_Down:
		// Both X and Y are out of bounds: use last grid cell height for all corners.
		InCornersHeights.X_Y = ErosionContext.GridHeights[GridSize * GridSize - 1];
		InCornersHeights.X1_Y = ErosionContext.GridHeights[GridSize * GridSize - 1];
		InCornersHeights.X_Y1 = ErosionContext.GridHeights[GridSize * GridSize - 1];
		InCornersHeights.X1_Y1 = ErosionContext.GridHeights[GridSize * GridSize - 1];
		break;
	case Error_Right:
		// X is out of bounds: clamp X coordinate, use valid Y coordinates.
		InCornersHeights.X_Y = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];
		InCornersHeights.X1_Y = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];
		InCornersHeights.X_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + GridSize) + TruncatedPosition.Y * GridSize];
		InCornersHeights.X1_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + GridSize) + TruncatedPosition.Y * GridSize];
		break;
	case Error_Down:
		// Y is out of bounds: use valid X coordinates, clamp Y coordinate.
		InCornersHeights.X_Y = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];
		InCornersHeights.X1_Y = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];
		InCornersHeights.X_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + 1) + TruncatedPosition.Y * GridSize];
		InCornersHeights.X1_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + 1) + TruncatedPosition.Y * GridSize];
		break;
	case No_Error:
		// All corners are within valid bounds: retrieve normal corner heights.
		InCornersHeights.X_Y = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];					  // P(x, y)
		InCornersHeights.X1_Y = ErosionContext.GridHeights[(TruncatedPosition.X + 1) + TruncatedPosition.Y * GridSize];			  // P(x + 1, y)
		InCornersHeights.X_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + GridSize) + TruncatedPosition.Y * GridSize];		  // P(x, y + 1)
		InCornersHeights.X1_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + GridSize + 1) + TruncatedPosition.Y * GridSize];   // P(x + 1, y + 1)
		break;
	}

	return InCornersHeights;
}

/**
 * Performs bilinear interpolation using corner heights and offset position.
 * Standard bilinear interpolation formula for smooth height calculation within a cell.
 */
float UErosionLibrary::GetBilinearInterpolation(const FVector2D& OffsetPosition, const FCornersHeights& CornersHeights)
{
	return CornersHeights.X_Y * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y) +
		CornersHeights.X1_Y * OffsetPosition.X * (1 - OffsetPosition.Y) +
		CornersHeights.X_Y1 * (1 - OffsetPosition.X) * OffsetPosition.Y +
		CornersHeights.X1_Y1 * OffsetPosition.X * OffsetPosition.Y;
}

/**
 * Identifies and stores all grid points within the erosion radius of the drop.
 * Creates a square area centered on the drop position with side length = 2 * radius + 1.
 * Example: radius = 1 -> 9 points, radius = 2 -> 25 points, radius = 3 -> 49 points...
 */
void UErosionLibrary::SetInRadiusPoints(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const int32 GridSize)
{
	// Iterate through square area around drop position.
	for (int32 Y = -ErosionSettings.ErosionRadius; Y <= ErosionSettings.ErosionRadius; Y++)
	{
		for (int32 X = -ErosionSettings.ErosionRadius; X <= ErosionSettings.ErosionRadius; X++)
		{
			const float PosX = FMath::Floor(DropPosition.X) + X;
			const float PosY = FMath::Floor(DropPosition.Y) + Y;

			FVector2D Position(PosX, PosY);

			// Skip points outside valid grid bounds.
			if (IsOutOfBound(Position, GridSize))
			{
				continue;
			}

			ErosionContext.Points.Add(Position);
		}
	}
}

/**
 * Calculates the relative weight of a point based on its distance from the drop.
 * Uses squared distance for efficiency, with weight decreasing as distance increases.
 */
float UErosionLibrary::GetRelativeWeightOnPoint(const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const FVector2D& PointPosition)
{
	const int32 SquaredErosionRadius = ErosionSettings.ErosionRadius * ErosionSettings.ErosionRadius;

	// Weight = max(0, radius^2 - distance^2)
	return FMath::Max(0, SquaredErosionRadius - (PointPosition - DropPosition).SquaredLength());
}

/**
 * Calculates erosion amounts for all affected points based on weights.
 * Multiplies each point's weight by the erosion factor to get final erosion value.
 */
TArray<float> UErosionLibrary::GetErosionOnPoints(const FErosionContext& ErosionContext, const float ErosionFactor)
{
	TArray<float> ErosionValues;
	ErosionValues.Reserve(ErosionContext.Points.Num());

	// Calculate erosion for each point proportional to its weight.
	for (int32 Index = 0; Index < ErosionContext.Points.Num(); Index++)
	{
		ErosionValues.Add(ErosionContext.SquaredWeights[Index] * ErosionFactor);
	}

	return ErosionValues;
}

/**
 * Distributes sediment deposit across nearby grid points using bilinear interpolation.
 * Handles boundary cases where deposit position is at or beyond grid edges.
 */
void UErosionLibrary::ComputeDepositOnPoints(FErosionContext& ErosionContext, const FVector2D& IntegerPosition, const FVector2D& OffsetPosition, const float Deposit, const int32 GridSize)
{
	switch (GetOutOfBoundAsResult(IntegerPosition, GridSize))
	{
		case Error_Right_Down:
			// Both coordinates out of bounds: deposit all to last cell.
			ErosionContext.GridHeights[GridSize * GridSize - 1] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			break;
		case Error_Right:
			// X out of bounds: distribute between two valid Y positions.
			ErosionContext.GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			ErosionContext.GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y;
			break;
		case Error_Down:
			// Y out of bounds: distribute between two valid X positions.
			ErosionContext.GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			ErosionContext.GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);
			break;
		case No_Error:
			// All positions valid: distribute using bilinear interpolation weights.
			ErosionContext.GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);        // P(x, y)
			ErosionContext.GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);        // P(x + 1, y)
			ErosionContext.GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y; // P(x, y + 1)
			ErosionContext.GridHeights[(IntegerPosition.X + GridSize + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * OffsetPosition.Y;   // P(x + 1, y + 1)
			break;
	}
}

/**
 * Checks if a position is outside the valid grid boundaries.
 * Returns true if any coordinate is negative or exceeds grid size.
 */
bool UErosionLibrary::IsOutOfBound(const FVector2D& DropPosition, const int32 GridSize)
{
	return DropPosition.X < 0 || DropPosition.X >= GridSize || DropPosition.Y < 0 || DropPosition.Y >= GridSize;
}

/**
 * Determines which boundary (if any) a position exceeds.
 * Checks if position is at the last valid cell (GridSize - 1) which needs special handling.
 */
EOutOfBoundResult UErosionLibrary::GetOutOfBoundAsResult(const FVector2D& IntegerPosition, const int32 GridSize)
{
	const bool bOutOfBoundsOnX = IntegerPosition.X >= GridSize - 1;
	const bool bOutOfBoundsOnY = IntegerPosition.Y >= GridSize - 1;

	if (bOutOfBoundsOnX && bOutOfBoundsOnY)
	{
		return EOutOfBoundResult::Error_Right_Down;
	}

	if (bOutOfBoundsOnX)
	{
		return EOutOfBoundResult::Error_Right;
	}

	if (bOutOfBoundsOnY)
	{
		return EOutOfBoundResult::Error_Down;
	}

	return EOutOfBoundResult::No_Error;
}

/**
 * Calculates wind direction vector from erosion settings.
 * Uses "Box-Muller" algorithm for "Gaussian distribution" if wind bias is enabled.
 * Direction can be cardinal, diagonal, or random with optional bias variation.
 */
FVector2D UErosionLibrary::GetWindDirection(const FErosionSettings& ErosionSettings)
{
	const float MinAngle = 0.f;
	const float MaxAngle = 360.f;
	const float Sigma = 35.f;  // Standard deviation for "Gaussian distribution".
	float Mu = 0.f;  // Mean angle.

	// Top-Left Pivot coordinate system.
	// Determine base wind angle from cardinal/diagonal direction.
	switch (ErosionSettings.WindDirection)
	{
		case EWindDirection::East:         Mu = 0.f;   break;
		case EWindDirection::South_East:   Mu = 45.f;  break;
		case EWindDirection::South:        Mu = 90.f;  break;
		case EWindDirection::South_West:   Mu = 135.f; break;
		case EWindDirection::West:         Mu = 180.f; break;
		case EWindDirection::North_West:   Mu = 225.f; break;
		case EWindDirection::North:        Mu = 270.f; break;
		case EWindDirection::North_East:   Mu = 315.f; break;
		case EWindDirection::Random:
		default:
			Mu = FMath::RandRange(MinAngle, MaxAngle);
			break;
	}

	float FinalAngle = Mu;

	// Apply Gaussian bias to wind direction if enabled.
	if (ErosionSettings.bWindBias)
	{
		// "Box-Muller" algorithm for Gaussian random number generation.
		const float First = FMath::FRandRange(0.f + KINDA_SMALL_NUMBER, 1.f);
		const float Second = FMath::FRandRange(0.f + KINDA_SMALL_NUMBER, 1.f);
		const float Z = FMath::Sqrt(-2.f * FMath::Loge(First)) * FMath::Cos(2.f * PI * Second);

		// Apply "Gaussian distribution" with mean Mu and standard deviation Sigma.
		FinalAngle = Mu + (Sigma * Z);

		// Wrap angle to [0, 360) range.
		while (FinalAngle < MinAngle)
		{
			FinalAngle += MaxAngle;
		}

		while (FinalAngle >= MaxAngle)
		{
			FinalAngle -= MaxAngle;
		}
	}

	// Convert angle to radians and create direction vector.
	const float Rad = FMath::DegreesToRadians(FinalAngle);
	const float Strength = FMath::RandRange(0.f, 1.f);  // Random strength multiplier.

	return FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)) * Strength;
}

/**
 * Attempts to get the normalized mean wind angle from erosion settings.
 * Returns false for random wind direction, true otherwise with normalized angle in [0, 360) range.
 */
bool UErosionLibrary::TryGetWindMeanAngleDegrees(const FErosionSettings& ErosionSettings, float& MeanAngle)
{
	float Mu = 0.f;

	// Map wind direction enum to angle in degrees.
	switch (ErosionSettings.WindDirection)
	{
		case EWindDirection::East:         Mu = 0.f;   break;
		case EWindDirection::South_East:   Mu = 45.f;  break;
		case EWindDirection::South:        Mu = 90.f;  break;
		case EWindDirection::South_West:   Mu = 135.f; break;
		case EWindDirection::West:         Mu = 180.f; break;
		case EWindDirection::North_West:   Mu = 225.f; break;
		case EWindDirection::North:        Mu = 270.f; break;
		case EWindDirection::North_East:   Mu = 315.f; break;
		case EWindDirection::Random:
		default: return false;  // Cannot provide mean for random direction.
	}

	// Normalize angle to [0, 360) range.
	float Angle = FMath::Fmod(Mu, 360.f);
	MeanAngle = (Angle < 0.f) ? (Angle + 360.f) : Angle;

	return true;
}

/**
 * Converts a wind angle in degrees to a normalized unit direction vector.
 * Uses standard trigonometric conversion with safe normalization.
 */
FVector2D UErosionLibrary::GetWindUnitVectorFromAngle(const float Degrees)
{
	const float Rad = FMath::DegreesToRadians(Degrees);

	const float X = FMath::Cos(Rad);
	const float Y = FMath::Sin(Rad);

	return FVector2D(X, Y).GetSafeNormal();
}

/**
 * Applies erosion effects for a single water drop simulation.
 * Simulates drop movement, sediment transport, erosion and deposition over multiple cycles.
 * Uses particle-based hydraulic erosion algorithm.
 */
void UErosionLibrary::ApplyErosion(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, FDrop& Drop, const int32 GridSize)
{
	float Sediment = 0;  // Amount of sediment currently carried by the drop.

	// Simulate drop movement for specified number of cycles.
	for (int64 Cycle = 0; Cycle < ErosionSettings.MaxPath; Cycle++)
	{
		// 0) Check if drop has left the valid grid area.
		if (IsOutOfBound(Drop.Position, GridSize))
		{
			return;
		}

		// Calculate integer and fractional parts of drop position.
		const FVector2D TruncatedPosOld = FVector2D(FMath::Floor(Drop.Position.X), FMath::Floor(Drop.Position.Y)); // (x, y)
		const FVector2D OffsetPosOld = FVector2D(Drop.Position.X - TruncatedPosOld.X, Drop.Position.Y - TruncatedPosOld.Y);

		// 1) Get heights at all four corners of the current cell.
		FCornersHeights PosOldHeights;
		SetCornersHeights(ErosionContext, PosOldHeights, TruncatedPosOld, GridSize);

		// 2) Compute gradient at current position using bilinear interpolation.
		FVector2D DropGradient = ComputeGradientByInterpolate(
			OffsetPosOld,
			PosOldHeights.X1_Y - PosOldHeights.X_Y,
			PosOldHeights.X1_Y1 - PosOldHeights.X_Y1,
			PosOldHeights.X_Y1 - PosOldHeights.X_Y,
			PosOldHeights.X1_Y1 - PosOldHeights.X1_Y
		);

		// 3) Update direction using inertia blending.
		// Mix previous direction (inertia) with downhill gradient.
		Drop.Direction = Drop.Direction * ErosionSettings.Inertia - DropGradient * (1 - ErosionSettings.Inertia);

		// Stop simulation if direction becomes invalid.
		if (Drop.Direction.SquaredLength() <= 0)
		{
			return;
		}

		Drop.Direction.Normalize();

		// 4) Calculate new position by moving in current direction.
		Drop.Position = Drop.Position + Drop.Direction;

		// Check if new position is still valid.
		if (IsOutOfBound(Drop.Position, GridSize))
		{
			return;
		}

		// Initialize weights for points within erosion radius.
		InitWeights(ErosionContext, ErosionSettings, Drop.Position, GridSize);

		// 5) Calculate height difference between old and new positions.
		const float HeightPosOld = GetBilinearInterpolation(OffsetPosOld, PosOldHeights);

		const FVector2D TruncatedPosNew = FVector2D(FMath::Floor(Drop.Position.X), FMath::Floor(Drop.Position.Y));
		const FVector2D OffsetPosNew = FVector2D(Drop.Position.X - TruncatedPosNew.X, Drop.Position.Y - TruncatedPosNew.Y);

		FCornersHeights PosNewHeights;
		SetCornersHeights(ErosionContext, PosNewHeights, TruncatedPosNew, GridSize);

		const float HeightPosNew = GetBilinearInterpolation(OffsetPosNew, PosNewHeights);
		const float HeightsDifference = HeightPosNew - HeightPosOld;

		// 6) Determine whether to deposit or erode sediment.

		// Calculate sediment carrying capacity based on slope, velocity, and water.
		const float C = FMath::Max(-HeightsDifference, ErosionSettings.MinimalSlope) * Drop.Velocity * Drop.Water * ErosionSettings.Capacity;

		const bool bDropHasMovingUp = HeightsDifference > 0;
		const bool bDropHasToDeposit = Sediment > C;

		// Deposit sediment if moving uphill or carrying too much.
		if (bDropHasMovingUp || bDropHasToDeposit)
		{
			// Calculate deposit amount.
			float Deposit = bDropHasMovingUp ? FMath::Min(HeightsDifference, Sediment) : (Sediment - C) * ErosionSettings.DepositionSpeed;
			Sediment -= Deposit;

			// Distribute deposit across nearby cells.
			ComputeDepositOnPoints(ErosionContext, TruncatedPosOld, OffsetPosOld, Deposit, GridSize);
		}
		else
		{
			// Erode terrain and pick up sediment.
			float Erosion = FMath::Min((C - Sediment) * ErosionSettings.ErosionSpeed, -HeightsDifference);

			// Apply erosion to all points within radius.
			TArray<float> ErosionValues = GetErosionOnPoints(ErosionContext, Erosion);
			for (int32 Index = 0; Index < ErosionValues.Num(); Index++)
			{
				const int32 MapIndex = ErosionContext.Points[Index].X + ErosionContext.Points[Index].Y * GridSize;

				// Ensure we don't erode below zero height.
				const float DeltaSediment = ErosionContext.GridHeights[MapIndex] < ErosionValues[Index] ? ErosionContext.GridHeights[MapIndex] : ErosionValues[Index];

				ErosionContext.GridHeights[MapIndex] -= DeltaSediment;
				Sediment += DeltaSediment;
			}
		}

		// 7) Update drop's physical properties.
		// Calculate new velocity.
		const float Velocity = ((Drop.Velocity * Drop.Velocity) + (-HeightsDifference) * ErosionSettings.Gravity);
		Drop.Velocity = Velocity > 0 ? FMath::Sqrt(Velocity) : 0;

		// Simulate water evaporation.
		Drop.Water = Drop.Water * (1 - ErosionSettings.Evaporation);
	}
}

/**
 * Initializes weight values for cells within the erosion radius of the drop.
 * Calculates normalized weights based on distance from drop position.
 */
void UErosionLibrary::InitWeights(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const int32 GridSize)
{
	// Clear previous weights and points.
	ErosionContext.SquaredWeights.Reset();
	ErosionContext.Points.Reset();

	float RelativeSquaredWeightsSum = 0;
	TArray<float> RelativeSquaredWeights;

	// Identify all points within erosion radius.
	SetInRadiusPoints(ErosionContext, ErosionSettings, DropPosition, GridSize);

	// Calculate raw weights and their sum.
	for (int32 Index = 0; Index < ErosionContext.Points.Num(); Index++)
	{
		RelativeSquaredWeights.Add(GetRelativeWeightOnPoint(ErosionSettings, DropPosition, ErosionContext.Points[Index]));
		RelativeSquaredWeightsSum += RelativeSquaredWeights[Index];
	}

	// Normalize weights so they sum to "1.0".
	for (int32 Index = 0; Index < ErosionContext.Points.Num(); Index++)
	{
		ErosionContext.SquaredWeights.Add(GetRelativeWeightOnPoint(ErosionSettings, DropPosition, ErosionContext.Points[Index]) / RelativeSquaredWeightsSum);
	}
}

/**
 * Main erosion simulation entry point.
 * Simulates multiple water drops to erode the landscape over many iterations.
 */
void UErosionLibrary::Erosion(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const int32 GridSize)
{
	// Pre-allocate memory for points and weights based on erosion radius.
	const int32 SampledPointsInRadius = (2 * ErosionSettings.ErosionRadius + 1) * (2 * ErosionSettings.ErosionRadius + 1);
	ErosionContext.SquaredWeights.Reserve(SampledPointsInRadius);
	ErosionContext.Points.Reserve(SampledPointsInRadius);

	// Simulate multiple drops for specified number of erosion cycles.
	for (int64 Index = 0; Index < ErosionSettings.ErosionCycles; Index++)
	{
		FDrop Drop;
		InitDrop(ErosionSettings, Drop, GridSize);
		ApplyErosion(ErosionContext, ErosionSettings, Drop, GridSize);

		// Drop completes its lifecycle.
	}

	// Erosion simulation complete.
}