#include "ErosionTemplateManager.h"
#include "GeneratorHeightMapLibrary.h"
#include "ErosionScapeSettings.h"
#include "ErosionLibrary.h"

TArray<FString> UErosionTemplateManager::GetAllTemplateNames() const
{
	TArray<FString> Out;
	if (UDataTable* DataTable = FDropByDropSettings::Get().GetErosionTemplatesDT())
	{
		for (const FName& Name : DataTable->GetRowNames())
		{
			if (!Name.ToString().Equals(TEXT("None")))
				Out.Add(Name.ToString());
		}
	}
	return Out;
}

TArray<FString> UErosionTemplateManager::FindTemplateNames(const FString& Query) const
{
	if (Query.IsEmpty()) return GetAllTemplateNames();
	TArray<FString> All = GetAllTemplateNames();
	All.RemoveAll([&](const FString& N){ return !N.StartsWith(Query); });
	return All;
}

void UErosionTemplateManager::SaveCurrentAsTemplate(const FString& Name)
{
	if (!ErosionSettingsReference.IsValid())
	{
		return;
	}

	UGeneratorHeightMapLibrary::SaveErosionTemplate(
		Name,
		ErosionSettingsReference->ErosionCycles,
		ErosionSettingsReference->Inertia,
		ErosionSettingsReference->Capacity,
		ErosionSettingsReference->MinimalSlope,
		ErosionSettingsReference->DepositionSpeed,
		ErosionSettingsReference->ErosionSpeed,
		ErosionSettingsReference->Gravity,
		ErosionSettingsReference->Evaporation,
		ErosionSettingsReference->MaxPath,
		ErosionSettingsReference->ErosionRadius
	);
}

void UErosionTemplateManager::LoadTemplate(const FString& Name)
{
	if (!ErosionSettingsReference.IsValid())
	{
		return;
	}

	if (auto* Row = UGeneratorHeightMapLibrary::LoadErosionTemplate(Name))
	{
		UGeneratorHeightMapLibrary::LoadRowIntoErosionFields(ErosionSettingsReference, Row);
	}
}

void UErosionTemplateManager::DeleteTemplate(const FString& Name)
{
	UGeneratorHeightMapLibrary::DeleteErosionTemplate(Name);
}

void UErosionTemplateManager::SetErosionSettingsReference(const TSharedPtr<FErosionSettings>& ErosionSettings)
{
	this->ErosionSettingsReference = ErosionSettings;
}
