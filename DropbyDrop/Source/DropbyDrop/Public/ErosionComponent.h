#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ErosionComponent.generated.h"


USTRUCT(BlueprintType)
struct FDrop
{
	GENERATED_BODY()

	FVector2D Position;
	FVector2D Direction;
	float Velocity;
	float Water;
	float Sediment;
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

UCLASS(Blueprintable)
class DROPBYDROP_API UErosionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UErosionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellSize;

#pragma region Parameters
	UPROPERTY(EditAnywhere)
	int32 ErosionCycles;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1")) // [0, 1]
	float Inertia; // pInertia

	UPROPERTY(EditAnywhere)
	float Capacity; // pCapacity

	UPROPERTY(EditAnywhere)
	float MinimalSlope; // pMinSlope

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1"))
	float DepositionSpeed; // pDeposition

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1"))
	float ErosionSpeed; // pErosion

	UPROPERTY(EditAnywhere)
	float Gravity; // pGravity

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "1"))
	float Evaporation; // pEvaporation

	UPROPERTY(EditAnywhere)
	float MaxPath; // pMaxPath

#pragma endregion

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetHeights(const TArray<float> NewHeights);

	UFUNCTION(BlueprintCallable)
	TArray<float> GenerateVirtualGrid(TArray<float> MapHeights, const int32 MapSize, const int32 NewCellSize);

	UFUNCTION(BlueprintCallable)
	void ErosionHandler(const int32 GridSize);

#pragma region InitDrop
	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	FDrop GenerateDropInitialParams(const int32 GridSize) const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	FDrop SetDropInitialParams(const int32 GridSize, const FVector2D Position, const FVector2D Direction, const float Velocity, const float Water) const;
#pragma endregion

	UFUNCTION(BlueprintCallable, BlueprintPure = false)
	void Erosion(FDrop Drop, /* make const */ int32 GridSize);

private:
	TArray<float> GridHeights;

#pragma region ErosionSubFunctions
	FVector2D GetGradient(const float& P1, const float& P2, const float& P3, const float& P4) const;
	FVector2D GetPairedLinearInterpolation(const FVector2D& OffsetPosition, const float& F1, const float& F2, const float& F3, const float& F4) const;
	FPositionHeights GetPositionHeights(const FVector2D& Position, const int32 GridSize) const;
	float GetBilinearInterpolation(const FVector2D& OffsetPosition, const FPositionHeights& PositionHeights) const;
#pragma endregion

};
