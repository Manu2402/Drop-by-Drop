#include "ErosionTemplateManager.h"
#include "GeneratorHeightMapLibrary.h"
#include "ErosionLibrary.h"

TArray<FString> UErosionTemplateManager::GetAllTemplateNames() const
{
	TArray<FString> Out;
	if (UDataTable* DataTable = UGeneratorHeightMapLibrary::GetErosionTemplates())
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
	UGeneratorHeightMapLibrary::SaveErosionTemplate(
		Name,
		UErosionLibrary::GetErosionCycles(),
		UErosionLibrary::GetInertia(),
		UErosionLibrary::GetCapacity(),
		UErosionLibrary::GetMinimalSlope(),
		UErosionLibrary::GetDepositionSpeed(),
		UErosionLibrary::GetErosionSpeed(),
		UErosionLibrary::GetGravity(),
		UErosionLibrary::GetEvaporation(),
		UErosionLibrary::GetMaxPath(),
		UErosionLibrary::GetErosionRadius()
	);
}

void UErosionTemplateManager::LoadTemplate(const FString& Name)
{
	if (auto* Row = UGeneratorHeightMapLibrary::LoadErosionTemplate(Name))
	{
		UGeneratorHeightMapLibrary::LoadRowIntoErosionFields(Row);
	}
}

void UErosionTemplateManager::DeleteTemplate(const FString& Name)
{
	UGeneratorHeightMapLibrary::DeleteErosionTemplate(Name);
}
