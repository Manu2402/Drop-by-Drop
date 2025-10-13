// © Manuel Solano

#pragma once

#include "CoreMinimal.h"
#include "DropByDropSettings.h"
#include "LandscapeInfoComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DROPBYDROP_API ULandscapeInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public: // Methods.
	ULandscapeInfoComponent() = default;

private: // Members.
	FHeightMapGenerationSettings HeightMapSettings;
	FExternalHeightMapSettings ExternalSettings;
	FLandscapeGenerationSettings LandscapeSettings;

	bool bIsEroded;
	bool bIsSplittedIntoProxies;

public: // Methods.
	FHeightMapGenerationSettings& GetHeightMapSettings();
	void SetHeightMapSettings(const FHeightMapGenerationSettings& NewHeightMapSettings);

	FExternalHeightMapSettings& GetExternalSettings();
	void SetExternalSettings(const FExternalHeightMapSettings& NewExternalSettings);

	FLandscapeGenerationSettings& GetLandscapeSettings();
	void SetLandscapeSettings(const FLandscapeGenerationSettings& NewLandscapeSettings);

	bool GetIsEroded() const;
	void SetIsEroded(const bool bNewIsEroded);

	bool GetIsSplittedIntoProxies() const;
	void SetIsSplittedIntoProxies(const bool bNewIsSplittedIntoProxies);

};
