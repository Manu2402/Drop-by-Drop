#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"  
#include "HeightMapMeshActor.generated.h"

UCLASS()
class DROPBYDROP_API AHeightMapMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AHeightMapMeshActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	UProceduralMeshComponent* Mesh;

	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	void GenerateMeshFromHeightMap(const TArray<float>& HeightMap, int32 MapSize, float MaxHeight, float ScaleXY);

	UFUNCTION(BlueprintCallable, Category = "HeightMap")
	void GenerateMeshFromPng(const FString& FilePath, int32 MapSize, float MaxHeight, float ScaleXY);

};
