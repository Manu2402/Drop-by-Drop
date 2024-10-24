#include "ErosionLibrary.h"

int32 UErosionLibrary::ErosionCycles = 100000;
float UErosionLibrary::Inertia = 0.3; // pInertia
float UErosionLibrary::Capacity = 8; // pCapacity
float UErosionLibrary::MinimalSlope = 0.01f; // pMinSlope
float UErosionLibrary::DepositionSpeed = 0.2f; // pDeposition
float UErosionLibrary::ErosionSpeed = 0.7f; // pErosion 
float UErosionLibrary::Gravity = 10; // pGravity
float UErosionLibrary::Evaporation = 0.02f; // pEvaporation 
float UErosionLibrary::MaxPath = 64; // pMaxPath 
int32 UErosionLibrary::ErosionRadius = 4; // pRadius 

TArray<float> UErosionLibrary::GridHeights = TArray<float>(); 
TArray<FVector2D> UErosionLibrary::Points = TArray<FVector2D>();
TArray<float> UErosionLibrary::Weights = TArray<float>();

UErosionLibrary::UErosionLibrary()
{
	const int32 SampledPointsInRadius = (2 * ErosionRadius + 1) * (2 * ErosionRadius + 1);
	Weights.Reserve(SampledPointsInRadius);
	Points.Reserve(SampledPointsInRadius);
}

void UErosionLibrary::SetHeights(const TArray<float>& NewHeights)
{
	GridHeights.Reserve(NewHeights.Num());
	GridHeights = NewHeights;
}

TArray<float> UErosionLibrary::GetHeights()
{
	return GridHeights;
}

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

// Useless!

// TArray<float> UErosionComponent::GenerateVirtualGrid(const TArray<float> MapHeightsValues, const int32 MapSize, const int32 NewCellSize = 1)
// {
// 	// NewCellSize = 1 --> 1x1 cells.
// 	// NewCellSize = 2 --> 2x2 cells.
// 	// NewCellSize = 3 --> 3x3 cells...
//
// 	MapHeights = MapHeightsValues;
//
// 	if (NewCellSize < 1)
// 	{
// 		UE_LOG(LogTemp, Error, TEXT("The cell size must be greater than zero!"));
// 		return TArray<float>(); // Error when the TArray size is equal to zero.
// 	}
//
// 	if (NewCellSize == 1)
// 	{
// 		return MapHeights; // Unique valid case at the time.
// 	}
//
// 	TArray<float> NewHeights;
//
// 	// fmod(float x, float y) returns the module about division between two floating numbers.
// 	if (std::fmod(MapSize / (NewCellSize * NewCellSize), 1.0) != 0.f)
// 	{
// 		// Isn't an error (maybe), but this case will be handled in a different way.
// 		return TArray<float>();
// 	}
//
// 	const int32 NewHeightsLength = MapHeights.Num() / (NewCellSize * NewCellSize);
// 	NewHeights.Reserve(NewHeightsLength);
//
// 	int32 Index = 0;
// 	for (int32 Y = 0; Y < MapSize; Y += NewCellSize)
// 	{
// 		for (int32 X = 0; X < MapSize; X += NewCellSize)
// 		{
// 			const int32 CurrentIndex = X + Y * MapSize;
// 			float CurrentValue = 0;
//
// 			for (int32 Z = 0; Z < NewCellSize; Z++)
// 			{
// 				for (int32 W = 0; W < NewCellSize; W++)
// 				{
// 					CurrentValue += MapHeights[CurrentIndex + (MapSize * Z) + W];
// 				}
// 			}
//
// 			CurrentValue /= (NewCellSize * NewCellSize);
//
// 			NewHeights.Add(CurrentValue);
// 			Index++;
// 		}
// 	}
//
// 	return NewHeights;
// }

FDrop UErosionLibrary::GenerateDropInitialParams(const int32& GridSize) // GridSize = MapSize / CellSize
{
	FDrop Drop;

	Drop.Position = FVector2D(FMath::RandRange(0.f, static_cast<float>(GridSize - 1)), FMath::RandRange(0.f, static_cast<float>(GridSize - 1)));
	Drop.Direction = FVector2D(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f));
	
	Drop.Velocity = 1;
	Drop.Water = 1;

	return Drop;
}

FDrop UErosionLibrary::SetDropInitialParams(const int32& GridSize, const FVector2D& Position, const FVector2D& Direction, const float& Velocity, const float& Water)
{
	FDrop Drop;
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

FVector2D UErosionLibrary::GetGradient(const float& P1, const float& P2, const float& P3, const float& P4)
{
	// Normalization with safe check.
	return FVector2D(P1 - P2, P3 - P4).GetSafeNormal();
}

FVector2D UErosionLibrary::GetPairedLinearInterpolation(const FVector2D& OffsetPosition, const float& F1, const float& F2, const float& F3, const float& F4)
{
	// (x, y) = (u, v)
	return FVector2D(
		F1 * (1 - OffsetPosition.Y) + F2 * OffsetPosition.Y,
		F3 * (1 - OffsetPosition.X) + F4 * OffsetPosition.X
	);
}

FPositionHeights UErosionLibrary::GetPositionHeights(const FVector2D& IntegerPosition, const int32& GridSize)
{
	FPositionHeights PositionHeights = FPositionHeights();

	switch (GetOutOfBoundResult(IntegerPosition, GridSize))
	{
		case Error_Right_Down:
			PositionHeights.X_Y = GridHeights[GridSize * GridSize - 1];
			PositionHeights.X1_Y = GridHeights[GridSize * GridSize - 1];
			PositionHeights.X_Y1 = GridHeights[GridSize * GridSize - 1 ];
			PositionHeights.X1_Y1 = GridHeights[GridSize * GridSize - 1];
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
			PositionHeights.X_Y = GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize];					// P(x, y)
			PositionHeights.X1_Y = GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize];				// P(x + 1, y)
			PositionHeights.X_Y1 = GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize];		// P(x, y + 1)
			PositionHeights.X1_Y1 = GridHeights[(IntegerPosition.X + GridSize + 1) + IntegerPosition.Y * GridSize]; // P(x + 1, y + 1)
			break;
	}
	
	return PositionHeights;
}

float UErosionLibrary::GetBilinearInterpolation(const FVector2D& OffsetPosition, const FPositionHeights& PositionHeights)
{
	return PositionHeights.X_Y * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y) +
		PositionHeights.X1_Y * OffsetPosition.X * (1 - OffsetPosition.Y) +
		PositionHeights.X_Y1 * (1 - OffsetPosition.X) * OffsetPosition.Y +
		PositionHeights.X1_Y1 * OffsetPosition.X * OffsetPosition.Y;
}

void UErosionLibrary::SetPointsPositionInRadius(const FVector2D& DropPosition, const int32& GridSize)
{
	// pRadius = 1 --> 9
	// pRadius = 2 --> 25
	// pRadius = 3 --> 49
	// ..................

	for (int32 Y = -ErosionRadius; Y <= ErosionRadius; Y++)
	{
		for (int32 X = -ErosionRadius; X <= ErosionRadius; X++)
		{
			const float PosX = FMath::Floor(DropPosition.X) + X;
			const float PosY = FMath::Floor(DropPosition.Y) + Y;
			
			if (PosX < 0 || PosX >= GridSize || PosY < 0 || PosY >= GridSize)
			{
				continue;
			}

			FVector2D Position(PosX, PosY);
			Points.Add(Position);
		}
	}
}

float UErosionLibrary::GetRelativeWeightOnPoint(const FVector2D& DropPosition, const FVector2D& PointPosition)
{
	return FMath::Max(0, ErosionRadius - (PointPosition - DropPosition).Length());
}

TArray<float> UErosionLibrary::GetErosionOnPoints(const float& ErosionFactor)
{
	TArray<float> ErosionValues;
	ErosionValues.Reserve(Points.Num());

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		ErosionValues.Add(Weights[Index] * ErosionFactor);
	}

	return ErosionValues;
}

void UErosionLibrary::ComputeDepositOnPoints(const FVector2D& IntegerPosition, const FVector2D& OffsetPosition, const float& Deposit, const int32& GridSize)
{
	switch (GetOutOfBoundResult(IntegerPosition, GridSize))
	{
		case Error_Right_Down:
			GridHeights[GridSize * GridSize - 1] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			break;
		case Error_Right:
			GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y;
			break;
		case Error_Down:
			GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y);
			GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y);
			break;
		case No_Error:
			GridHeights[IntegerPosition.X + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * (1 - OffsetPosition.Y); // P(x, y)
			GridHeights[(IntegerPosition.X + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * (1 - OffsetPosition.Y); // P(x + 1, y)
			GridHeights[(IntegerPosition.X + GridSize) + IntegerPosition.Y * GridSize] += Deposit * (1 - OffsetPosition.X) * OffsetPosition.Y; // P(x, y + 1)
			GridHeights[(IntegerPosition.X + GridSize + 1) + IntegerPosition.Y * GridSize] += Deposit * OffsetPosition.X * OffsetPosition.Y; // P(x + 1, y + 1)
			break;
	}
}

EOutOfBoundResult UErosionLibrary::GetOutOfBoundResult(const FVector2D& IntegerPosition, const int32& GridSize)
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

void UErosionLibrary::Erosion(FDrop& Drop, const int32& GridSize)
{
	float Sediment = 0;

	for (int32 Cycle = 0; Cycle < MaxPath; Cycle++)
	{
		// Phase zero: drop position information.
		FVector2D IntegerPosOld = FVector2D(FMath::Floor(Drop.Position.X), FMath::Floor(Drop.Position.Y)); // (x, y)
		FVector2D OffsetPosOld = FVector2D(Drop.Position.X - IntegerPosOld.X, Drop.Position.Y - IntegerPosOld.Y);

		if (Drop.Position.X < 0 || Drop.Position.X >= GridSize || Drop.Position.Y < 0 || Drop.Position.Y >= GridSize)
		{
			return;
		}
		
		//UE_LOG(LogTemp, Warning, TEXT("Drop's integer PosOld: (%f, %f)"), IntegerPosOld.X, IntegerPosOld.Y);
		//UE_LOG(LogTemp, Warning, TEXT("Drop's offset PosOld: (%f, %f)"), OffsetPosOld.X, OffsetPosOld.Y);

		// Phase one: gradients calculus.
		FPositionHeights PosOldHeights = GetPositionHeights(IntegerPosOld, GridSize);

		/*
			FVector2D GradientX_Y = GetGradient(X1_Y, X_Y, X_Y1, X_Y);
			FVector2D GradientX1_Y = GetGradient(X1_Y, X_Y, X1_Y1, X1_Y);
			FVector2D GradientX_Y1 = GetGradient(X1_Y1, X_Y1, X_Y1, X_Y);
			FVector2D GradientX1_Y1 = GetGradient(X1_Y1, X_Y1, X1_Y1, X1_Y);
		*/

		// Phase two: bilinear interpolation between gradients.
		FVector2D DropPositionGradient = GetPairedLinearInterpolation(
			OffsetPosOld,
			PosOldHeights.X1_Y - PosOldHeights.X_Y,
			PosOldHeights.X1_Y1 - PosOldHeights.X_Y1,
			PosOldHeights.X_Y1 - PosOldHeights.X_Y,
			PosOldHeights.X1_Y1 - PosOldHeights.X1_Y
		);

		//UE_LOG(LogTemp, Warning, TEXT("Gradient on Drop's position: (%f, %f)"), DropPositionGradient.X, DropPositionGradient.Y);

		// Phase three: inertia blends.
		Drop.Direction = Drop.Direction * Inertia - DropPositionGradient * (1 - Inertia);

		//UE_LOG(LogTemp, Warning, TEXT("New direction: (%f, %f)"), Drop.Direction.X, Drop.Direction.Y);

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

		//UE_LOG(LogTemp, Warning, TEXT("New position: (%f, %f)"), Drop.Position.X, Drop.Position.Y);

		InitWeights(Drop.Position, GridSize);
		
		// Phase five: heights difference.
		const float HeightPosOld = GetBilinearInterpolation(OffsetPosOld, PosOldHeights);

		FVector2D IntegerPosNew = FVector2D(FMath::Floor(Drop.Position.X), FMath::Floor(Drop.Position.Y));
		FVector2D OffsetPosNew = FVector2D(Drop.Position.X - IntegerPosNew.X, Drop.Position.Y - IntegerPosNew.Y);

		FPositionHeights PosNewHeights = GetPositionHeights(IntegerPosNew, GridSize);

		const float HeightPosNew = GetBilinearInterpolation(OffsetPosNew, PosNewHeights);
		const float HeightsDifference = HeightPosNew - HeightPosOld;

		//UE_LOG(LogTemp, Warning, TEXT("Heights Difference: %f"), HeightsDifference);

		// Phase six: deposit and erosion calculus.

		// Capacity calculus.
		const float C = FMath::Max(-HeightsDifference, MinimalSlope) * Drop.Velocity * Drop.Water * Capacity;

		//UE_LOG(LogTemp, Warning, TEXT("C: %f"), C);

		const bool bDropHasMovingUp = HeightsDifference > 0;
		const bool bDropHasToDeposit = Sediment > C;
		
		if (bDropHasMovingUp || bDropHasToDeposit)
		{
			float Deposit = bDropHasMovingUp ? FMath::Min(HeightsDifference, Sediment) : (Sediment - C) * DepositionSpeed;
			Sediment -= Deposit;
			
			ComputeDepositOnPoints(IntegerPosOld, OffsetPosOld, Deposit, GridSize);
		}
		else
		{
			float Erosion = FMath::Min((C - Sediment) * ErosionSpeed, -HeightsDifference);

			TArray<float> ErosionValues = GetErosionOnPoints(Erosion);

			for (int32 Index = 0; Index < Points.Num(); Index++)
			{
				const int32 MapIndex = Points[Index].X + Points[Index].Y * GridSize;
				
				const float DeltaSediment = GridHeights[MapIndex] < ErosionValues[Index] ? GridHeights[MapIndex] : ErosionValues[Index];

				GridHeights[MapIndex] -= DeltaSediment;
				Sediment += DeltaSediment;
			}
		}
		
		//UE_LOG(LogTemp, Warning, TEXT("Deposit: %f"), Deposit);
		//UE_LOG(LogTemp, Warning, TEXT("Erosion: %f"), Erosion);

		// Phase seven: drop's mutation.
		const float Velocity = ((Drop.Velocity * Drop.Velocity) + (-HeightsDifference) * Gravity);
		Drop.Velocity = Velocity > 0 ? FMath::Sqrt(Velocity) : 0;

		Drop.Water = Drop.Water * (1 - Evaporation);

		//UE_LOG(LogTemp, Warning, TEXT("New Velocity: %f"), Drop.Velocity);
		//UE_LOG(LogTemp, Warning, TEXT("New Water: %f"), Drop.Water);

		//UE_LOG(LogTemp, Warning, TEXT("Iteration number: %d"), (Cycle + 1));

		//UE_LOG(LogTemp, Warning, TEXT("-----------------------------------------------------------"));
	}
}

void UErosionLibrary::InitWeights(const FVector2D& DropPosition, const int32& GridSize)
{
	Weights.Reset();
	Points.Reset();

	float RelativeWeightsSum = 0;
	TArray<float> RelativeWeights;

	SetPointsPositionInRadius(DropPosition, GridSize);
	
	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		RelativeWeights.Add(GetRelativeWeightOnPoint(DropPosition, Points[Index]));
		RelativeWeightsSum += RelativeWeights[Index];
	}

	//UE_LOG(LogTemp, Warning, TEXT("RelativeWeightsSum: %f"), RelativeWeightsSum);

	for (int32 Index = 0; Index < Points.Num(); Index++)
	{
		Weights.Add(GetRelativeWeightOnPoint(DropPosition, Points[Index]) / RelativeWeightsSum);
	}
}

void UErosionLibrary::ErosionHandler(const int32& GridSize)
{
	for (int32 Index = 0; Index < ErosionCycles; Index++)
	{
		FDrop Drop = GenerateDropInitialParams(GridSize);
		Erosion(Drop, GridSize);
		
		//UE_LOG(LogTemp, Warning, TEXT("DROP IS DEAD!"));
		//UE_LOG(LogTemp, Warning, TEXT("-----------------------------------------------------------"));
	}
	
	//UE_LOG(LogTemp, Warning, TEXT("EROSION ENDED!"));
}