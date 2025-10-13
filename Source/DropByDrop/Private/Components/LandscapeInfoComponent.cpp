// © Manuel Solano

#include "Components/LandscapeInfoComponent.h"

FHeightMapGenerationSettings& ULandscapeInfoComponent::GetHeightMapSettings()
{
	return HeightMapSettings;
}

void ULandscapeInfoComponent::SetHeightMapSettings(const FHeightMapGenerationSettings& NewHeightMapSettings)
{
	HeightMapSettings = NewHeightMapSettings;
}

FExternalHeightMapSettings& ULandscapeInfoComponent::GetExternalSettings()
{
	return ExternalSettings;
}

void ULandscapeInfoComponent::SetExternalSettings(const FExternalHeightMapSettings& NewExternalSettings)
{
	ExternalSettings = NewExternalSettings;
}

FLandscapeGenerationSettings& ULandscapeInfoComponent::GetLandscapeSettings()
{
	return LandscapeSettings;
}

void ULandscapeInfoComponent::SetLandscapeSettings(const FLandscapeGenerationSettings& NewLandscapeSettings)
{
	LandscapeSettings = NewLandscapeSettings;
}

bool ULandscapeInfoComponent::GetIsEroded() const
{
	return bIsEroded;
}

void ULandscapeInfoComponent::SetIsEroded(const bool bNewIsEroded)
{
	bIsEroded = bNewIsEroded;
}

bool ULandscapeInfoComponent::GetIsSplittedIntoProxies() const
{
	return bIsSplittedIntoProxies;
}

void ULandscapeInfoComponent::SetIsSplittedIntoProxies(const bool bNewIsSplittedIntoProxies)
{
	bIsSplittedIntoProxies = bNewIsSplittedIntoProxies;
}
