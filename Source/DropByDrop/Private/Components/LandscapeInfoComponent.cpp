// © Manuel Solano

#include "Components/LandscapeInfoComponent.h"

/**
 * Gets a reference to the heightmap generation settings.
 */
FHeightMapGenerationSettings& ULandscapeInfoComponent::GetHeightMapSettings()
{
	return HeightMapSettings;
}

/**
 * Sets new heightmap generation settings by copying the provided settings.
 */
void ULandscapeInfoComponent::SetHeightMapSettings(const FHeightMapGenerationSettings& NewHeightMapSettings)
{
	HeightMapSettings = NewHeightMapSettings;
}

/**
 * Gets a reference to the external heightmap settings.
 */
FExternalHeightMapSettings& ULandscapeInfoComponent::GetExternalSettings()
{
	return ExternalSettings;
}

/**
 * Sets new external heightmap settings by copying the provided settings.
 */
void ULandscapeInfoComponent::SetExternalSettings(const FExternalHeightMapSettings& NewExternalSettings)
{
	ExternalSettings = NewExternalSettings;
}

/**
 * Gets a reference to the landscape generation settings.
 */
FLandscapeGenerationSettings& ULandscapeInfoComponent::GetLandscapeSettings()
{
	return LandscapeSettings;
}

/**
 * Sets new landscape generation settings by copying the provided settings.
 */
void ULandscapeInfoComponent::SetLandscapeSettings(const FLandscapeGenerationSettings& NewLandscapeSettings)
{
	LandscapeSettings = NewLandscapeSettings;
}

/**
 * Gets the current erosion state of the landscape.
 */
bool ULandscapeInfoComponent::GetIsEroded() const
{
	return bIsEroded;
}

/**
 * Sets the erosion state of the landscape.
 */
void ULandscapeInfoComponent::SetIsEroded(const bool bNewIsEroded)
{
	bIsEroded = bNewIsEroded;
}

/**
 * Gets the current proxy splitting state of the landscape.
 */
bool ULandscapeInfoComponent::GetIsSplittedIntoProxies() const
{
	return bIsSplittedIntoProxies;
}

/**
 * Sets the proxy splitting state of the landscape.
 */
void ULandscapeInfoComponent::SetIsSplittedIntoProxies(const bool bNewIsSplittedIntoProxies)
{
	bIsSplittedIntoProxies = bNewIsSplittedIntoProxies;
}