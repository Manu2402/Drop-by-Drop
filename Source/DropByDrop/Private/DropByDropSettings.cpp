// © Manuel Solano
// © Roberto Capparelli

#include "DropByDropSettings.h"

/**
 * Initializes the singleton settings manager with default values.
 * Sets "ErosionTemplatesDT" to nullptr (no templates loaded initially)
 * and "WindPreviewScale" to 10.0 for reasonable visualization size.
 */
FDropByDropSettings::FDropByDropSettings() : ErosionTemplatesDT(nullptr), WindPreviewScale(10.0f) { }

FDropByDropSettings& FDropByDropSettings::Get()
{
	static FDropByDropSettings Instance;
	return Instance;
}

float FDropByDropSettings::GetWindPreviewScale() const
{
	return WindPreviewScale;
}

void FDropByDropSettings::SetWindPreviewScale(float NewWindPreviewScale)
{
	WindPreviewScale = NewWindPreviewScale;
}

UDataTable* FDropByDropSettings::GetErosionTemplatesDT() const
{
	return ErosionTemplatesDT;
}

void FDropByDropSettings::SetErosionTemplatesDT(UDataTable* NewErosionTemplatesDT)
{
	ErosionTemplatesDT = NewErosionTemplatesDT;
}