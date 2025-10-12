// © Roberto Capparelli

#include "ErosionTemplateManager.h"

#include "GeneratorHeightMapLibrary.h"
#include "ErosionScapeSettings.h"
#include "DropByDropLogger.h"

TArray<FString> UErosionTemplateManager::GetAllTemplateNames() const
{
	TArray<FString> OutNames;

	if (UDataTable* DataTable = FDropByDropSettings::Get().GetErosionTemplatesDT())
	{
		for (const FName& Name : DataTable->GetRowNames())
		{
			if (!Name.ToString().Equals(TEXT("None")))
			{
				OutNames.Add(Name.ToString());
			}
		}
	}

	return OutNames;
}

TArray<FString> UErosionTemplateManager::FindTemplateNames(const FString& QueryFilter) const
{
	if (QueryFilter.IsEmpty())
	{
		return GetAllTemplateNames();
	}

	TArray<FString> AllNames = GetAllTemplateNames();
	AllNames.RemoveAll([&QueryFilter](const FString& Name){ return !Name.StartsWith(QueryFilter); });

	return AllNames;
}

void UErosionTemplateManager::SaveCurrentAsTemplate(const FString& Name)
{
	if (!ErosionSettings.IsValid())
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"ErosionSettings\" resource is invalid!"));
		return;
	}

	UGeneratorHeightMapLibrary::SaveErosionTemplate(
		Name,
		ErosionSettings->ErosionCycles,
		ErosionSettings->Inertia,
		ErosionSettings->Capacity,
		ErosionSettings->MinimalSlope,
		ErosionSettings->DepositionSpeed,
		ErosionSettings->ErosionSpeed,
		ErosionSettings->Gravity,
		ErosionSettings->Evaporation,
		ErosionSettings->MaxPath,
		ErosionSettings->ErosionRadius
	);
}

void UErosionTemplateManager::LoadTemplate(const FString& Name)
{
	if (!ErosionSettings.IsValid())
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"ErosionSettings\" resource is invalid!"));
		return;
	}

	if (auto* Row = UGeneratorHeightMapLibrary::LoadErosionTemplate(Name))
	{
		UGeneratorHeightMapLibrary::LoadRowIntoErosionFields(ErosionSettings, Row);
	}
}

void UErosionTemplateManager::DeleteTemplate(const FString& Name)
{
	UGeneratorHeightMapLibrary::DeleteErosionTemplate(Name);
}

void UErosionTemplateManager::SetErosionSettings(const TSharedPtr<FErosionSettings>& ErosionSettingsValue)
{
	ErosionSettings = ErosionSettingsValue;
}
