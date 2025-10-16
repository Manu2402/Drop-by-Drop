// © Manuel Solano

#pragma once
#include "CoreMinimal.h"
#include "DropByDropSettings.h"
#include "LandscapeInfoComponent.generated.h"

/**
 * Component that stores and manages information related to landscape generation
 * and configuration in the "DropByDrop" system.
 *
 * This component maintains settings for heightmap generation (internal and external),
 * landscape configurations, and tracks the erosion and proxy splitting state.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DROPBYDROP_API ULandscapeInfoComponent : public UActorComponent
{
	GENERATED_BODY()

public: // Methods.
	/** Default constructor for the component. */
	ULandscapeInfoComponent() = default;

private: // Members.
	/** Settings for procedural heightmap generation. */
	FHeightMapGenerationSettings HeightMapSettings;

	/** Settings for importing heightmaps from file system. */
	FExternalHeightMapSettings ExternalSettings;

	/** General configurations for landscape creation. */
	FLandscapeGenerationSettings LandscapeSettings;

	/** Flag indicating whether the landscape has been eroded. */
	bool bIsEroded;

	/** Flag indicating whether the landscape has been split into landscape proxies. */
	bool bIsSplittedIntoProxies;

public: // Methods.
	/**
	 * Gets a reference to the heightmap generation settings.
	 * @return Reference to the heightmap generation settings.
	 */
	FHeightMapGenerationSettings& GetHeightMapSettings();

	/**
	 * Sets new settings for heightmap generation.
	 * @param NewHeightMapSettings - The new settings to apply.
	 */
	void SetHeightMapSettings(const FHeightMapGenerationSettings& NewHeightMapSettings);

	/**
	 * Gets a reference to the external heightmap settings.
	 * @return Reference to the external heightmap settings.
	 */
	FExternalHeightMapSettings& GetExternalSettings();

	/**
	 * Sets new external heightmap settings.
	 * @param NewExternalSettings - The new external settings to apply.
	 */
	void SetExternalSettings(const FExternalHeightMapSettings& NewExternalSettings);

	/**
	 * Gets a reference to the landscape generation settings.
	 * @return Reference to the landscape generation settings.
	 */
	FLandscapeGenerationSettings& GetLandscapeSettings();

	/**
	 * Sets new landscape generation settings.
	 * @param NewLandscapeSettings - The new landscape settings to apply.
	 */
	void SetLandscapeSettings(const FLandscapeGenerationSettings& NewLandscapeSettings);

	/**
	 * Gets the erosion state of the landscape.
	 * @return True if the landscape has been eroded, false otherwise.
	 */
	bool GetIsEroded() const;

	/**
	 * Sets the erosion state of the landscape.
	 * @param bNewIsEroded - The new erosion state.
	 */
	void SetIsEroded(const bool bNewIsEroded);

	/**
	 * Gets the proxy splitting state of the landscape.
	 * @return True if the landscape has been split into proxies, false otherwise.
	 */
	bool GetIsSplittedIntoProxies() const;

	/**
	 * Sets the proxy splitting state of the landscape.
	 * @param bNewIsSplittedIntoProxies - The new proxy splitting state.
	 */
	void SetIsSplittedIntoProxies(const bool bNewIsSplittedIntoProxies);

};