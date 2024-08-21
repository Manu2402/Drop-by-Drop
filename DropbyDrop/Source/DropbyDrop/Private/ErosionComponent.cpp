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