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
	Heights = NewHeights;
}

/*

TO DO:
- Handle "not squared" terrains (num_rows != num_columns);
- Find a way to take excluded values (in a 20x20 map with 7x7 pixels size there is an out of bounds
  or in cases as 6x6 pixels size there are some excluded values). 

*/

TArray<float> UErosionComponent::GenerateVirtualGrid(TArray<float> MapHeights, const int32 MapSize, const int32 NewCellSize = 1)
{
	// NewCellSize = 1 --> 1x1 cells.
	// NewCellSize = 2 --> 2x2 cells.
	// NewCellSize = 3 --> 3x3 cells...

#pragma region Test
/*

MapHeights = TArray<float>({    3,5,2,1,1,7,
								4,6,7,8,7,9,
								9,3,6,9,8,11,
								2,4,6,1,4,3,
								7,9,0,3,2,1,
								1,2,4,5,3,6
						  });

MapSize = MapHeights.Num() / 6;

*/
#pragma endregion

	if (NewCellSize < 1)
	{
		UE_LOG(LogTemp, Error, TEXT("The cell size must be upper than zero!"));
		return TArray<float>(); // Error when the TArray size is equal to zero.
	}

	if (NewCellSize == 1)
	{
		return MapHeights;
	}

	TArray<float> NewHeights;

	// fmod(float x, float y) returns the module about division between two floating numbers.
	if (std::fmod(MapSize / (NewCellSize * NewCellSize), 1.0) != 0.f)
	{
		// Isn't an error (maybe), but this case will be handle in a different way.
		return TArray<float>();
	}

	int32 NewHeightsLength = MapHeights.Num() / (NewCellSize * NewCellSize);
	NewHeights.Reserve(NewHeightsLength);

	int32 Index = 0;
	for (int32 Y = 0; Y < MapSize; Y += NewCellSize)
	{
		for (int32 X = 0; X < MapSize; X += NewCellSize)
		{
			int32 CurrentIndex = X + Y * MapSize;
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