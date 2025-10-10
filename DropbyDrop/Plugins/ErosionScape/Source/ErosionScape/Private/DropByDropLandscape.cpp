#include "DropByDropLandscape.h"

ULandscapeInfoComponent::ULandscapeInfoComponent()
{
	HeightMapSettings = FHeightMapGenerationSettings();
	ExternalSettings = FExternalHeightMapSettings();
	LandscapeSettings = FLandscapeGenerationSettings();

	bIsEroded = false;
	bIsSplittedIntoProxies = false;
}
