#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ErosionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DROPBYDROP_API UErosionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UErosionComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> Heights;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

};
