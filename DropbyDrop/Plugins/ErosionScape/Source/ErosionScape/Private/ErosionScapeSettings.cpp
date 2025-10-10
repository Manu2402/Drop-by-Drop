#include "ErosionScapeSettings.h"

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
