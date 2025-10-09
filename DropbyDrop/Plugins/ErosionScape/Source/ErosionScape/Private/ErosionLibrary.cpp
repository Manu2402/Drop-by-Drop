#include "ErosionLibrary.h"
#include "ErosionScapeSettings.h"
#include "Math/UnrealMathUtility.h"
#include "DropByDropLogger.h"

void UErosionLibrary::SetHeights(FErosionContext& ErosionContext, const TArray<float>& InHeights)
{
	ErosionContext.GridHeights.Reserve(InHeights.Num());
	ErosionContext.GridHeights = InHeights;
}

TArray<float> UErosionLibrary::GetHeights(const FErosionContext& ErosionContext)
{
	return ErosionContext.GridHeights;
}

FDrop& UErosionLibrary::InitDrop(const FErosionSettings& ErosionSettings, FDrop& OutDrop, const int32 GridSize /* GridSize = MapSize / CellSize */)
{
	const float Limit = static_cast<float>(GridSize - 1);

	OutDrop.Position = FVector2D(FMath::RandRange(0.f, Limit), FMath::RandRange(0.f, Limit));
	OutDrop.Direction = GetWindDirection(ErosionSettings);

	OutDrop.Velocity = 1;
	OutDrop.Water = 1;

	return OutDrop;
}

FDrop& UErosionLibrary::SetDrop(const FErosionSettings& ErosionSettings, FDrop& Drop, const int32 GridSize, const FVector2D& Position, const FVector2D& Direction, const float Velocity, const float Water)
{
	if (IsOutOfBound(Drop.Position, GridSize))
	{
		UE_LOG(LogDropByDropErosion, Error, TEXT("You inserted an invalid position value! The drop's params will be generated automatically!"));
		return InitDrop(ErosionSettings, Drop, GridSize);
	}

	Drop.Position = Position;

	if (Direction.X < -1 || Direction.X > 1 || Direction.Y < -1 || Direction.Y > 1)
	{
		UE_LOG(LogDropByDropErosion, Error, TEXT("You inserted an invalid direction value! The drop's params will be generated automatically!"));
		return InitDrop(ErosionSettings, Drop, GridSize);
	}

	Drop.Direction = Direction;

	Drop.Velocity = Velocity;
	Drop.Water = Water;

	return Drop;
}

FVector2D UErosionLibrary::ComputeGradient(const float P1, const float P2, const float P3, const float P4)
{
	// Normalization with safe check.
	return FVector2D(P1 - P2, P3 - P4).GetSafeNormal();
}

FVector2D UErosionLibrary::ComputeGradientByInterpolate(const FVector2D& OffsetPosition, const float F1, const float F2, const float F3, const float F4)
{
	// (x, y) = (u, v)
	return FVector2D(
		F1 * (1 - OffsetPosition.Y) + F2 * OffsetPosition.Y,
		F3 * (1 - OffsetPosition.X) + F4 * OffsetPosition.X
	);
}

FCornersHeights& UErosionLibrary::SetCornersHeights(const FErosionContext& ErosionContext, FCornersHeights& InCornersHeights, const FVector2D& TruncatedPosition, const int32 GridSize)
{
	const EOutOfBoundResult OutOfBoundResult = GetOutOfBoundAsResult(TruncatedPosition, GridSize);
	
	switch (OutOfBoundResult)
	{
	case Error_Right_Down:
		InCornersHeights.X_Y   = ErosionContext.GridHeights[GridSize * GridSize - 1];
		InCornersHeights.X1_Y  = ErosionContext.GridHeights[GridSize * GridSize - 1];
		InCornersHeights.X_Y1  = ErosionContext.GridHeights[GridSize * GridSize - 1];
		InCornersHeights.X1_Y1 = ErosionContext.GridHeights[GridSize * GridSize - 1];
		break;
	case Error_Right:
		InCornersHeights.X_Y   = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];
		InCornersHeights.X1_Y  = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];
		InCornersHeights.X_Y1  = ErosionContext.GridHeights[(TruncatedPosition.X + GridSize) + TruncatedPosition.Y * GridSize];
		InCornersHeights.X1_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + GridSize) + TruncatedPosition.Y * GridSize];
		break;
	case Error_Down:
		InCornersHeights.X_Y   = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];
		InCornersHeights.X1_Y  = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];
		InCornersHeights.X_Y1  = ErosionContext.GridHeights[(TruncatedPosition.X + 1) + TruncatedPosition.Y * GridSize];
		InCornersHeights.X1_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + 1) + TruncatedPosition.Y * GridSize];
		break;
	case No_Error:
		InCornersHeights.X_Y   = ErosionContext.GridHeights[TruncatedPosition.X + TruncatedPosition.Y * GridSize];					  // P(x, y)
		InCornersHeights.X1_Y  = ErosionContext.GridHeights[(TruncatedPosition.X + 1) + TruncatedPosition.Y * GridSize];			  // P(x + 1, y)
		InCornersHeights.X_Y1  = ErosionContext.GridHeights[(TruncatedPosition.X + GridSize) + TruncatedPosition.Y * GridSize];		  // P(x, y + 1)
		InCornersHeights.X1_Y1 = ErosionContext.GridHeights[(TruncatedPosition.X + GridSize + 1) + TruncatedPosition.Y * GridSize];   // P(x + 1, y + 1)
		break;
	}

	return InCornersHeights;
}

float UErosionLibrary::GetBilinearInterpolation(const FVector2D& OffsetPosition, const FCornersHeights& CornersHeights)
{
	return CornersHeights.X_Y * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y) +
		CornersHeights.X1_Y * OffsetPosition.X * (1 - OffsetPosition.Y) +
		CornersHeights.X_Y1 * (1 - OffsetPosition.X) * OffsetPosition.Y +
		CornersHeights.X1_Y1 * OffsetPosition.X * OffsetPosition.Y;
}

void UErosionLibrary::SetInRadiusPoints(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const int32 GridSize)
{
	// pRadius = 1 --> 9
	// pRadius = 2 --> 25
	// pRadius = 3 --> 49
	// ..................

	for (int32 Y = -ErosionSettings.ErosionRadius; Y <= ErosionSettings.ErosionRadius; Y++)
	{
		for (int32 X = -ErosionSettings.ErosionRadius; X <= ErosionSettings.ErosionRadius; X++)
		{
			const float PosX = FMath::Floor(DropPosition.X) + X;
			const float PosY = FMath::Floor(DropPosition.Y) + Y;

			FVector2D Position(PosX, PosY);
			if (IsOutOfBound(Position, GridSize))
			{
				continue;
			}

			ErosionContext.Points.Add(Position);
		}
	}
}

float UErosionLibrary::GetRelativeWeightOnPoint(const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const FVector2D& PointPosition)
{
	const int32 SquaredErosionRadius =  ErosionSettings.ErosionRadius * ErosionSettings.ErosionRadius;
	return FMath::Max(0, SquaredErosionRadius - (PointPosition - DropPosition).SquaredLength());
}

TArray<float> UErosionLibrary::GetErosionOnPoints(const FErosionContext& ErosionContext, const float ErosionFactor)
{
	TArray<float> ErosionValues;
	ErosionValues.Reserve(ErosionContext.Points.Num());

	for (int32 Index = 0; Index < ErosionContext.Points.Num(); Index++)
	{
		ErosionValues.Add(ErosionContext.SquaredWeights[Index] * ErosionFactor);
	}

	return ErosionValues;
}

void UErosionLibrary::ComputeDepositOnPoints(FErosionContext& ErosionContext, const FVector2D& IntegerPosition, const FVector2D& OffsetPosition, const float Deposit, const int32 GridSize)
{
	switch (GetOutOfBoundAsResult(IntegerPosition, GridSize))
	{
	case Error_Right_Down:
		ErosionContext.GridHeights[GridSize * GridSize - 1] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
		break;
	case Error_Right:
		ErosionContext.GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
		ErosionContext.GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y;
		break;
	case Error_Down:
		ErosionContext.GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
		ErosionContext.GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);
		break;
	case No_Error:
		ErosionContext.GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);        // P(x, y)
		ErosionContext.GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);        // P(x + 1, y)
		ErosionContext.GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y; // P(x, y + 1)
		ErosionContext.GridHeights[(IntegerPosition.X + GridSize + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * OffsetPosition.Y;   // P(x + 1, y + 1)
		break;
	}
}

bool UErosionLibrary::IsOutOfBound(const FVector2D& DropPosition, const int32 GridSize)
{
	return DropPosition.X < 0 || DropPosition.X >= GridSize || DropPosition.Y < 0 || DropPosition.Y >= GridSize;
}

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

FVector2D UErosionLibrary::GetWindDirection(const FErosionSettings& ErosionSettings)
{
	const float MinAngle = 0.f;
	const float MaxAngle = 360.f;
	float Mu = 0.f;
	const float Sigma = 15.f;

	// Top-Left Pivot.
	switch (ErosionSettings.WindDirection)
	{
	case EWindDirection::Est:        Mu = 0.f;   break;
	case EWindDirection::Sud_Est:    Mu = 45.f;  break;
	case EWindDirection::Sud:        Mu = 90.f;  break;
	case EWindDirection::Sud_Ovest:  Mu = 135.f; break;
	case EWindDirection::Ovest:      Mu = 180.f; break;
	case EWindDirection::Nord_Ovest: Mu = 225.f; break;
	case EWindDirection::Nord:       Mu = 270.f; break;
	case EWindDirection::Nord_Est:   Mu = 315.f; break;
	case EWindDirection::Random:
	default:
		Mu = FMath::RandRange(MinAngle, MaxAngle);
		break;
	}

	float FinalAngle = Mu;

	if (ErosionSettings.bWindBias)
	{
		// Box-Muller algorithm
		const float First = FMath::FRandRange(0.f + KINDA_SMALL_NUMBER, 1.f);
		const float Second = FMath::FRandRange(0.f + KINDA_SMALL_NUMBER, 1.f);
		const float Z = FMath::Sqrt(-2.f * FMath::Loge(First)) * FMath::Cos(2.f * PI * Second);

		// Gaussian distribution.
		FinalAngle = Mu + (Sigma * Z);

		while (FinalAngle < 0.f) FinalAngle += 360.f;
		while (FinalAngle >= 360.f) FinalAngle -= 360.f;
	}

	const float Rad = FMath::DegreesToRadians(FinalAngle);
	const float Strength = FMath::RandRange(0.f, 1.f);

	return FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)) * Strength;
}

bool UErosionLibrary::TryGetWindMeanAngleDegrees(const FErosionSettings& ErosionSettings, float& MeanAngle)
{
	float Mu = 0.f;

	switch (ErosionSettings.WindDirection)
	{
	case EWindDirection::Est:        Mu = 0.f;   break;
	case EWindDirection::Sud_Est:    Mu = 45.f;  break;
	case EWindDirection::Sud:        Mu = 90.f;  break;
	case EWindDirection::Sud_Ovest:  Mu = 135.f; break;
	case EWindDirection::Ovest:      Mu = 180.f; break;
	case EWindDirection::Nord_Ovest: Mu = 225.f; break;
	case EWindDirection::Nord:       Mu = 270.f; break;
	case EWindDirection::Nord_Est:   Mu = 315.f; break;
	case EWindDirection::Random:
	default: return false;
	}

	float R = FMath::Fmod(Mu, 360.f);
	MeanAngle = (R < 0.f) ? (R + 360.f) : R;
	return true;
}

FVector2D UErosionLibrary::GetWindUnitVectorFromAngle(float Degrees)
{
	const float Rad = FMath::DegreesToRadians(Degrees);

	const float X = FMath::Cos(Rad);
	const float Y = FMath::Sin(Rad);

	return FVector2D(X, Y).GetSafeNormal();
}

void UErosionLibrary::ApplyErosion(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, FDrop& Drop, const int32 GridSize)
{
	float Sediment = 0;

	for (uint64 Cycle = 0; Cycle < ErosionSettings.MaxPath; Cycle++)
	{
		// 0) Drop's position computation on grid.
		if (IsOutOfBound(Drop.Position, GridSize))
		{
			return;
		}
		
		const FVector2D TruncatedPosOld = FVector2D(FMath::Floor(Drop.Position.X), FMath::Floor(Drop.Position.Y)); // (x, y)
		const FVector2D OffsetPosOld = FVector2D(Drop.Position.X - TruncatedPosOld.X, Drop.Position.Y - TruncatedPosOld.Y);

		// 1) Compute heights on all cell's corners.
		FCornersHeights PosOldHeights;
		SetCornersHeights(ErosionContext, PosOldHeights, TruncatedPosOld, GridSize);
		
		// 2) Compute gradient.
		FVector2D DropGradient = ComputeGradientByInterpolate(
			OffsetPosOld,
			PosOldHeights.X1_Y  - PosOldHeights.X_Y,
			PosOldHeights.X1_Y1 - PosOldHeights.X_Y1,
			PosOldHeights.X_Y1  - PosOldHeights.X_Y,
			PosOldHeights.X1_Y1 - PosOldHeights.X1_Y
		);
		
		// Phase three: inertia blends.
		Drop.Direction = Drop.Direction * ErosionSettings.Inertia - DropGradient * (1 - ErosionSettings.Inertia);

		if (Drop.Direction.SquaredLength() <= 0)
		{
			return;
		}

		Drop.Direction.Normalize();

		// Phase four: new position calculus.
		Drop.Position = Drop.Position + Drop.Direction;

		if (IsOutOfBound(Drop.Position, GridSize))
		{
			return;
		}

		InitWeights(ErosionContext, ErosionSettings, Drop.Position, GridSize);

		// Phase five: heights differences.
		const float HeightPosOld = GetBilinearInterpolation(OffsetPosOld, PosOldHeights);

		const FVector2D TruncatedPosNew = FVector2D(FMath::Floor(Drop.Position.X), FMath::Floor(Drop.Position.Y));
		const FVector2D OffsetPosNew = FVector2D(Drop.Position.X - TruncatedPosNew.X, Drop.Position.Y - TruncatedPosNew.Y);

		FCornersHeights PosNewHeights;
		SetCornersHeights(ErosionContext, PosNewHeights, TruncatedPosNew, GridSize);

		const float HeightPosNew = GetBilinearInterpolation(OffsetPosNew, PosNewHeights);
		const float HeightsDifference = HeightPosNew - HeightPosOld;

		// Phase six: deposit and erosion calculus.

		// Capacity calculus.
		const float C = FMath::Max(-HeightsDifference, ErosionSettings.MinimalSlope) * Drop.Velocity * Drop.Water * ErosionSettings.Capacity;

		const bool bDropHasMovingUp = HeightsDifference > 0;
		const bool bDropHasToDeposit = Sediment > C;

		if (bDropHasMovingUp || bDropHasToDeposit)
		{
			float Deposit = bDropHasMovingUp ? FMath::Min(HeightsDifference, Sediment) : (Sediment - C) * ErosionSettings.DepositionSpeed;
			Sediment -= Deposit;

			ComputeDepositOnPoints(ErosionContext, TruncatedPosOld, OffsetPosOld, Deposit, GridSize);
		}
		else
		{
			float Erosion = FMath::Min((C - Sediment) * ErosionSettings.ErosionSpeed, -HeightsDifference);

			TArray<float> ErosionValues = GetErosionOnPoints(ErosionContext, Erosion);
			for (int32 Index = 0; Index < ErosionValues.Num(); Index++)
			{
				const int32 MapIndex = ErosionContext.Points[Index].X + ErosionContext.Points[Index].Y * GridSize;
				const float DeltaSediment = ErosionContext.GridHeights[MapIndex] < ErosionValues[Index] ? ErosionContext.GridHeights[MapIndex] : ErosionValues[Index];

				ErosionContext.GridHeights[MapIndex] -= DeltaSediment;
				Sediment += DeltaSediment;
			}
		}

		// Phase seven: drop's mutation.
		const float Velocity = ((Drop.Velocity * Drop.Velocity) + (-HeightsDifference) * ErosionSettings.Gravity);
		Drop.Velocity = Velocity > 0 ? FMath::Sqrt(Velocity) : 0;

		Drop.Water = Drop.Water * (1 - ErosionSettings.Evaporation);
	}
}

void UErosionLibrary::InitWeights(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const FVector2D& DropPosition, const int32 GridSize)
{
	ErosionContext.SquaredWeights.Reset();
	ErosionContext.Points.Reset();

	float RelativeSquaredWeightsSum = 0;
	TArray<float> RelativeSquaredWeights;

	SetInRadiusPoints(ErosionContext, ErosionSettings, DropPosition, GridSize);

	for (int32 Index = 0; Index < ErosionContext.Points.Num(); Index++)
	{
		RelativeSquaredWeights.Add(GetRelativeWeightOnPoint(ErosionSettings, DropPosition, ErosionContext.Points[Index]));
		RelativeSquaredWeightsSum += RelativeSquaredWeights[Index];
	}

	for (int32 Index = 0; Index < ErosionContext.Points.Num(); Index++)
	{
		ErosionContext.SquaredWeights.Add(GetRelativeWeightOnPoint(ErosionSettings, DropPosition, ErosionContext.Points[Index]) / RelativeSquaredWeightsSum);
	}
}

// Erosion started.
void UErosionLibrary::Erosion(FErosionContext& ErosionContext, const FErosionSettings& ErosionSettings, const int32 GridSize)
{
	const int32 SampledPointsInRadius = (2 * ErosionSettings.ErosionRadius + 1) * (2 * ErosionSettings.ErosionRadius + 1);
	ErosionContext.SquaredWeights.Reserve(SampledPointsInRadius);
	ErosionContext.Points.Reserve(SampledPointsInRadius);
	
	for (uint64 Index = 0; Index < ErosionSettings.ErosionCycles; Index++)
	{
		FDrop Drop;
		InitDrop(ErosionSettings, Drop, GridSize);
		ApplyErosion(ErosionContext, ErosionSettings, Drop, GridSize);

		// Drop death.
	}

	// Erosion ended.
}