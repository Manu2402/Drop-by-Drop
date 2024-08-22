#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ErosionComponent.generated.h"


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

private:
	TArray<float> Heights;

};
