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

UCLASS(Blueprintable)
class DROPBYDROP_API UErosionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UErosionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CellSize;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetHeights(const TArray<float> NewHeights);

	UFUNCTION(BlueprintCallable)
	TArray<float> GenerateVirtualGrid(TArray<float> MapHeights, const int32 MapSize, const int32 NewCellSize);

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
	FVector2D GetBilinearInterpolation(const FVector2D& OffsetPosition, const float& F1, const float& F2, const float& F3, const float& F4) const;
#pragma endregion

};
