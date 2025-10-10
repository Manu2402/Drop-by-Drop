#pragma once

#include "CoreMinimal.h"
#include "Landscape.h"
#include "ErosionScapeSettings.h"
#include "DropByDropLandscape.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class EROSIONSCAPE_API ULandscapeInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULandscapeInfoComponent();

	FHeightMapGenerationSettings HeightMapSettings;
	FExternalHeightMapSettings ExternalSettings;
	FLandscapeGenerationSettings LandscapeSettings;

	bool bIsEroded;
	bool bIsSplittedIntoProxies;

private:

};
