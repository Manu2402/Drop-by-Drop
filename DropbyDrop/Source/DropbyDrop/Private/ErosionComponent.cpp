#include "ErosionComponent.h"

UErosionComponent::UErosionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UErosionComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UErosionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UErosionComponent::SetHeights(const TArray<float> NewHeights)
{
	GridHeights = NewHeights;
}

// Useless!
TArray<float> UErosionComponent::GenerateVirtualGrid(const TArray<float> MapHeightsValues, const int32 MapSize, const int32 NewCellSize = 1)
{
	// NewCellSize = 1 --> 1x1 cells.
	// NewCellSize = 2 --> 2x2 cells.
	// NewCellSize = 3 --> 3x3 cells...

#pragma region Example
/*
		  ----- X
		| 3,5,2,
		| 1,4,6,    (x, y) = z --> (1, 2) = 8
		| 7,8,9
		Y

		X = Columns
		Y = Rows

MapSize = 3; (3x3 grid)

*/
#pragma endregion

	MapHeights = MapHeightsValues;

	if (NewCellSize < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("The cell size must be upper than zero!"));
		return TArray<float>(); // Error when the TArray size is equal to zero.
	}

	if (NewCellSize == 1)
	{
		return MapHeights; // Unique valid case at the time.
	}

	TArray<float> NewHeights;

	// fmod(float x, float y) returns the module about division between two floating numbers.
	if (std::fmod(MapSize / (NewCellSize * NewCellSize), 1.0) != 0.f)
	{
		// Isn't an error (maybe), but this case will be handled in a different way.
		return TArray<float>();
	}

	const int32 NewHeightsLength = MapHeights.Num() / (NewCellSize * NewCellSize);
	NewHeights.Reserve(NewHeightsLength);

	int32 Index = 0;
	for (int32 Y = 0; Y < MapSize; Y += NewCellSize)
	{
		for (int32 X = 0; X < MapSize; X += NewCellSize)
		{
			const int32 CurrentIndex = X + Y * MapSize;
			float CurrentValue = 0;

			for (int32 Z = 0; Z < NewCellSize; Z++)
			{
				for (int32 W = 0; W < NewCellSize; W++)
				{
					CurrentValue += MapHeights[CurrentIndex + (MapSize * Z) + W];
				}
			}

			CurrentValue /= (NewCellSize * NewCellSize);

			NewHeights.Add(CurrentValue);
			Index++;
		}
	}

	return NewHeights;
}

FDrop UErosionComponent::GenerateDropInitialParams(const int32 GridSize) const // GridSize = MapSize / CellSize
{
	FDrop Drop = FDrop();

	Drop.Position = FVector2D(FMath::RandRange(0.f, static_cast<float>(GridSize - 1)), FMath::RandRange(0.f, static_cast<float>(GridSize - 1)));
	Drop.Direction = FVector2D(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f));
	Drop.Velocity = 1;
	Drop.Water = 1;

	return Drop;
}

FDrop UErosionComponent::SetDropInitialParams(const int32 GridSize, const FVector2D Position, const FVector2D Direction, const float Velocity, const float Water) const
{
	FDrop Drop = FDrop();

	if (Position.X < 0 || Position.X >= GridSize || Position.Y < 0 || Position.Y >= GridSize)
	{
		UE_LOG(LogTemp, Error, TEXT("You inserted an invalid position value! The drop's params will be generated automatically!"));
		return GenerateDropInitialParams(GridSize);
	}

	Drop.Position = Position;

	if (Direction.X < -1 || Direction.X > 1 || Direction.Y < -1 || Direction.Y > 1)
	{
		UE_LOG(LogTemp, Error, TEXT("You inserted an invalid direction value! The drop's params will be generated automatically!"));
		return GenerateDropInitialParams(GridSize);
	}

	Drop.Direction = Direction;

	Drop.Velocity = Velocity;
	Drop.Water = Water;

	return Drop;
}

FVector2D UErosionComponent::GetGradient(const float& P1, const float& P2, const float& P3, const float& P4) const
{
	// Normalization with safe check.
	return FVector2D(P1 - P2, P3 - P4).GetSafeNormal();
}

FVector2D UErosionComponent::GetPairedLinearInterpolation(const FVector2D& OffsetPosition, const float& F1, const float& F2, const float& F3, const float& F4) const
{
	// (x, y) = (u, v)
	return FVector2D(
		F1 * (1 - OffsetPosition.Y) + F2 * OffsetPosition.Y,
		F3 * (1 - OffsetPosition.X) + F4 * OffsetPosition.X
	);
}

FPositionHeights UErosionComponent::GetPositionHeights(const FVector2D& IntegerPosition, const int32 GridSize) const
{
	FPositionHeights PositionHeights = FPositionHeights();

	switch (GetOutOfBoundResult(IntegerPosition, GridSize))
	{
		case Error_Right_Down:
			PositionHeights.X_Y = GridHeights[GridSize - 1];
			PositionHeights.X1_Y = GridHeights[GridSize - 1];
			PositionHeights.X_Y1 = GridHeights[GridSize - 1];
			PositionHeights.X1_Y1 = GridHeights[GridSize - 1];
			break;
		case Error_Right:
			PositionHeights.X_Y = GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize];
			PositionHeights.X1_Y = GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize];
			PositionHeights.X_Y1 = GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize];
			PositionHeights.X1_Y1 = GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize];
			break;
		case Error_Down:
			PositionHeights.X_Y = GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize];
			PositionHeights.X1_Y = GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize];
			PositionHeights.X_Y1 = GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize];
			PositionHeights.X1_Y1 = GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize];
			break;
		case No_Error:
			PositionHeights.X_Y = GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize];                     // P(x, y)
			PositionHeights.X1_Y = GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize];				 // P(x + 1, y)
			PositionHeights.X_Y1 = GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize];		 // P(x, y + 1)
			PositionHeights.X1_Y1 = GridHeights[(IntegerPosition.X + GridSize + 1) + IntegerPosition.Y * GridSize];	 // P(x + 1, y + 1)
			break;
	}

	return PositionHeights;
}

float UErosionComponent::GetBilinearInterpolation(const FVector2D& OffsetPosition, const FPositionHeights& PositionHeights) const
{
	return PositionHeights.X_Y * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y) +
		PositionHeights.X1_Y * OffsetPosition.X * (1 - OffsetPosition.Y) +
		PositionHeights.X_Y1 * (1 - OffsetPosition.X) * OffsetPosition.Y +
		PositionHeights.X1_Y1 * OffsetPosition.X * OffsetPosition.Y;
}

TArray<FVector2D> UErosionComponent::GetPointsPositionInRadius(const FVector2D& DropPosition, /* make const */ int32 GridSize) const
{
#pragma region Test
	//DropPosition = FVector2D(1, 1);
	//GridSize = 4;
#pragma endregion

	TArray<FVector2D> PointsPositionInRadius = TArray<FVector2D>();
	PointsPositionInRadius.Reserve((2 * ErosionRadius + 1) * (2 * ErosionRadius + 1));

	// pRadius = 1 --> 9
	// pRadius = 2 --> 25
	// pRadius = 3 --> 49
	// ..................

	for (int32 Y = -ErosionRadius; Y <= ErosionRadius; Y++)
	{
		for (int32 X = -ErosionRadius; X <= ErosionRadius; X++)
		{
			const float PosX = DropPosition.X + X;
			const float PosY = DropPosition.Y + Y;

			if (PosX < 0 || PosX >= GridSize || PosY < 0 || PosY >= GridSize)
			{
				continue;
			}

			PointsPositionInRadius.Add(FVector2D(PosX, PosY));
		}
	}

	return PointsPositionInRadius;
}

float UErosionComponent::GetRelativeWeightOnPoint(const FVector2D& DropPosition, const FVector2D& PointPosition) const
{
	return FMath::Max(0, ErosionRadius - (PointPosition - DropPosition).Length());
}

TArray<float> UErosionComponent::GetErosionOnPoints(const float& ErosionFactor)
{
	TArray<float> ErosionValues;
	ErosionValues.Reserve(Points.Num());

#pragma region Test
	//ErosionFactor = 0.7f;
#pragma endregion

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		ErosionValues.Add(Weights[Index] * ErosionFactor);
	}

	return ErosionValues;
}

void UErosionComponent::ComputeDepositOnPoints(const FVector2D& IntegerPosition, const FVector2D& OffsetPosition, const float& Deposit, const int32 GridSize)
{
	switch (GetOutOfBoundResult(IntegerPosition, GridSize))
	{
		case Error_Right_Down:
			GridHeights[GridSize - 1] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			GridHeights[GridSize - 1] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);
			GridHeights[GridSize - 1] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y;
			GridHeights[GridSize - 1] += Deposit * OffsetPosition.X * OffsetPosition.Y;
			break;
		case Error_Right:
			GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);
			GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y;
			GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * OffsetPosition.Y;
			break;
		case Error_Down:
			GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);
			GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y;
			GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * OffsetPosition.Y;
			break;
		case No_Error:
			GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);		    // P(x, y)
			GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);			// P(x + 1, y)
			GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y;	// P(x, y + 1)
			GridHeights[(IntegerPosition.X + GridSize + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * OffsetPosition.Y;	// P(x + 1, y + 1)
			break;
	}
}

EOutOfBoundResult UErosionComponent::GetOutOfBoundResult(const FVector2D& IntegerPosition, const int32 GridSize) const
{
	const bool bOutOfBoundsOnX = IntegerPosition.X >= GridSize - 1;
	const bool bOutOfBoundsOnY = IntegerPosition.Y >= GridSize - 1;

#pragma region OutOfBoundsHandler
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
#pragma endregion

	return EOutOfBoundResult::No_Error;
}

void UErosionComponent::Erosion(FDrop Drop, const int32 GridSize, const int32 MapSize)
{
#pragma region Test
	/*
	
	GridHeights = TArray<float>({ 3, 5, 2, 1, 4, 6, 7, 8, 9 });
	GridSize = 3;
	Drop.Position = FVector2D(1.7f, 1.1f);
	Drop.Direction = FVector2D(1, 2);
	Drop.Velocity = 2;
	Drop.Water = 2;

	*/
#pragma endregion
	float Sediment = 0;

	for (int32 Cycle = 0; Cycle < MaxPath; Cycle++)
	{
		// Phase zero: drop position information.
		FVector2D IntegerPosOld = FVector2D(FMath::Floor(Drop.Position.X), FMath::Floor(Drop.Position.Y)); // (x, y)
		FVector2D OffsetPosOld = FVector2D(Drop.Position.X - IntegerPosOld.X, Drop.Position.Y - IntegerPosOld.Y);

		UE_LOG(LogTemp, Warning, TEXT("Drop's integer PosOld: (%f, %f)"), IntegerPosOld.X, IntegerPosOld.Y);
		UE_LOG(LogTemp, Warning, TEXT("Drop's offset PosOld: (%f, %f)"), OffsetPosOld.X, OffsetPosOld.Y);

		// Phase one: gradients calculus.
		FPositionHeights PosOldHeights = GetPositionHeights(IntegerPosOld, GridSize);

		/*
			FVector2D GradientX_Y = GetGradient(X1_Y, X_Y, X_Y1, X_Y);
			FVector2D GradientX1_Y = GetGradient(X1_Y, X_Y, X1_Y1, X1_Y);
			FVector2D GradientX_Y1 = GetGradient(X1_Y1, X_Y1, X_Y1, X_Y);
			FVector2D GradientX1_Y1 = GetGradient(X1_Y1, X_Y1, X1_Y1, X1_Y);
		*/

#pragma region Test
		/*
		
		PosOldHeights.X_Y = 4;
		PosOldHeights.X1_Y = 6;
		PosOldHeights.X_Y1 = 8;
		PosOldHeights.X1_Y1 = 9;
		
		*/
#pragma endregion

		// Phase two: bilinear interpolation between gradients.
		FVector2D DropPositionGradient = GetPairedLinearInterpolation(
			OffsetPosOld,
			PosOldHeights.X1_Y - PosOldHeights.X_Y,
			PosOldHeights.X1_Y1 - PosOldHeights.X_Y1,
			PosOldHeights.X_Y1 - PosOldHeights.X_Y,
			PosOldHeights.X1_Y1 - PosOldHeights.X1_Y
		);

		UE_LOG(LogTemp, Warning, TEXT("Gradient on Drop's position: (%f, %f)"), DropPositionGradient.X, DropPositionGradient.Y);

		// Phase three: inertia blends.
		Drop.Direction = Drop.Direction * Inertia - DropPositionGradient * (1 - Inertia);

		UE_LOG(LogTemp, Warning, TEXT("New direction: (%f, %f)"), Drop.Direction.X, Drop.Direction.Y);

		// Phase three.five: inertia blends.
		if (Drop.Direction.SquaredLength() > 0)
		{
			Drop.Direction.Normalize();
		}
		else if (Drop.Direction.SquaredLength() < 0)
		{
			Drop.Direction = FVector2D(FMath::RandRange(-1, 1), FMath::RandRange(-1, 1)).GetSafeNormal();
		}
		else
		{
			return;
		}

		// Phase four: new position calculus.
		Drop.Position = Drop.Position + Drop.Direction;

		if (Drop.Position.X < 0 || Drop.Position.X >= GridSize || Drop.Position.Y < 0 || Drop.Position.Y >= GridSize)
		{
			return;
		}

		UE_LOG(LogTemp, Warning, TEXT("New position: (%f, %f)"), Drop.Position.X, Drop.Position.Y);

		InitWeights(Drop.Position, GridSize);

#pragma region Test
		//Drop.Position = FVector2D(1.168711f, 0.252809f);
#pragma endregion

		// Phase five: heights difference.
		const float HeightPosOld = GetBilinearInterpolation(OffsetPosOld, PosOldHeights);

		FVector2D IntegerPosNew = FVector2D(FMath::Floor(Drop.Position.X), FMath::Floor(Drop.Position.Y));
		FVector2D OffsetPosNew = FVector2D(Drop.Position.X - IntegerPosNew.X, Drop.Position.Y - IntegerPosNew.Y);

		FPositionHeights PosNewHeights = GetPositionHeights(IntegerPosNew, GridSize);

		float HeightPosNew = GetBilinearInterpolation(OffsetPosNew, PosNewHeights);
		float HeightsDifference = HeightPosNew - HeightPosOld;

		UE_LOG(LogTemp, Warning, TEXT("Heights Difference: %f"), HeightsDifference);

		// Phase six: deposit and erosion calculus.
		float Deposit = 0, Erosion = 0;

#pragma region Test
		//Drop.Velocity = 1;
#pragma endregion

		// Capacity calculus.
		const float C = FMath::Max(-HeightsDifference, MinimalSlope) * Drop.Velocity * Drop.Water * Capacity;

		UE_LOG(LogTemp, Warning, TEXT("C: %f"), C);

		const bool bDropHasMovingUp = HeightsDifference > 0;

		if (const bool bDropHasToDeposit = Sediment > C; bDropHasMovingUp || bDropHasToDeposit)
		{
			Deposit = bDropHasMovingUp ? Deposit = FMath::Min(HeightsDifference, Sediment) : (Sediment - C) * DepositionSpeed;
			Sediment -= Deposit;

			ComputeDepositOnPoints(IntegerPosOld, OffsetPosOld, Deposit, GridSize);
		}
		else
		{
			Erosion = FMath::Min((C - Sediment) * ErosionSpeed, -HeightsDifference);

			TArray<float> ErosionValues = GetErosionOnPoints(Erosion);

#pragma region Test
			/*
			
			ErosionValues.Empty();
			ErosionValues.Add(0.07f);
			ErosionValues.Add(0.27f);
			ErosionValues.Add(0.39f);
			
			*/
#pragma endregion

			for (int32 Index = 0; Index < Points.Num(); Index++)
			{
				const int32 MapIndex = Points[Index].X + Points[Index].Y * GridSize;
				Sediment += MapHeights[MapIndex] < ErosionValues[Index] ? MapHeights[MapIndex] : ErosionValues[Index];

				GridHeights[MapIndex] -= ErosionValues[Index];
			}
		}

		// Apply modifies on the original heightmap.

		UE_LOG(LogTemp, Warning, TEXT("Deposit: %f"), Deposit);
		UE_LOG(LogTemp, Warning, TEXT("Erosion: %f"), Erosion);

		// Phase seven: drop's mutation. The original formula take "HeightsDifference" without minus.
		const float VelocityValue = ((Drop.Velocity * Drop.Velocity) + (-HeightsDifference * Gravity));
		const float Velocity = VelocityValue > 0 ? VelocityValue : 0;
		Drop.Velocity = FMath::Sqrt(Velocity);

#pragma region Test
		//Drop.Water = 3;
#pragma endregion

		Drop.Water = Drop.Water * (1 - Evaporation);

		UE_LOG(LogTemp, Warning, TEXT("New Velocity: %f"), Drop.Velocity);
		UE_LOG(LogTemp, Warning, TEXT("New Water: %f"), Drop.Water);

		UE_LOG(LogTemp, Warning, TEXT("Iteration number: %d"), (Cycle + 1));

		UE_LOG(LogTemp, Warning, TEXT("-----------------------------------------------------------"));
	}

}

void UErosionComponent::InitWeights(const FVector2D& DropPosition, const int32 GridSize)
{
	Weights.Empty();
	TArray<float> RelativeWeights;

#pragma region Test
	/*
	
	Points = TArray<FVector2D>({
		FVector2D(1, 1),
		FVector2D(1, 2),
		FVector2D(2, 2)
	});
		
	*/

	//DropPosition = FVector2D(2, 2);
#pragma endregion

	float RelativeWeightsSum = 0;

	Points = GetPointsPositionInRadius(DropPosition, GridSize);

	Weights.Reserve(Points.Num());
	RelativeWeights.Reserve(Points.Num());

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		RelativeWeights.Add(GetRelativeWeightOnPoint(DropPosition, Points[Index]));
		RelativeWeightsSum += RelativeWeights[Index];
	}

	UE_LOG(LogTemp, Warning, TEXT("RelativeWeightsSum: %f"), RelativeWeightsSum);

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		Weights.Add(GetRelativeWeightOnPoint(DropPosition, Points[Index]) / RelativeWeightsSum);
	}
}

void UErosionComponent::ErosionHandler(const int32 GridSize, const int32 MapSize)
{
	for (int32 Index = 0; Index < ErosionCycles; Index++)
	{
		const FDrop Drop = GenerateDropInitialParams(GridSize);

		Erosion(Drop, GridSize, MapSize);
		UE_LOG(LogTemp, Warning, TEXT("DROP IS DEAD!"));
		UE_LOG(LogTemp, Warning, TEXT("-----------------------------------------------------------"));
	}

	UE_LOG(LogTemp, Warning, TEXT("EROSION ENDED!"));
}