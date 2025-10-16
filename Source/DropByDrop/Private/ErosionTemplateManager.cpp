// © Roberto Capparelli

#include "ErosionTemplateManager.h"
#include "Libraries/PipelineLibrary.h"
#include "DropByDropNotifications.h"
#include "DropByDropSettings.h"
#include "DropByDropLogger.h"

/**
 * Extracts all row names from the erosion templates data table.
 * Filters out the "None" entry which is conventionally used as a placeholder
 * or default row in Unreal data tables.
 */
TArray<FString> UErosionTemplateManager::GetAllTemplateNames() const
{
	TArray<FString> OutNames;

	// Retrieve the data table from the singleton settings manager.
	if (UDataTable* DataTable = FDropByDropSettings::Get().GetErosionTemplatesDT())
	{
		// Iterate through all row names in the data table.
		for (const FName& Name : DataTable->GetRowNames())
		{
			OutNames.Add(Name.ToString());
		}
	}

	return OutNames;
}

/**
 * Performs a filtered search for template names based on a query string.
 * Uses prefix matching to find relevant templates.
 * If no filter is provided, returns all available templates.
 */
TArray<FString> UErosionTemplateManager::FindTemplateNames(const FString& QueryFilter) const
{
	// If no filter specified, return everything.
	if (QueryFilter.IsEmpty())
	{
		return GetAllTemplateNames();
	}

	// Get all template names.
	TArray<FString> AllNames = GetAllTemplateNames();

	// Remove names that don't start with the query filter.
	// Lambda captures QueryFilter by reference for efficient filtering.
	AllNames.RemoveAll([&QueryFilter](const FString& Name){ return !Name.StartsWith(QueryFilter); });

	return AllNames;
}

/**
 * Persists the current erosion settings to the data table as a named template.
 * Validates that erosion settings are properly initialized before attempting to save.
 * Delegates the actual save operation to "UPipelineLibrary" which handles
 * data table serialization.
 */
void UErosionTemplateManager::SaveCurrentAsTemplate(const FString& Name)
{
	// Validate that we have valid erosion settings to save.
	if (!ErosionSettings.IsValid())
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"ErosionSettings\" resource is invalid!"));
		return;
	}

	if (Name.IsEmpty())
	{
		UDropByDropNotifications::ShowWarningNotification(TEXT("Cannot save an erosion template with an empty name!"));
		UE_LOG(LogDropByDropTemplate, Warning, TEXT("Cannot save an erosion template with an empty name!"));
		return;
	}

	// Delegate to PipelineLibrary to handle data table row creation/update.
	// Passes all individual erosion parameter values.
	if (!UPipelineLibrary::SaveErosionTemplate(
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
	))
	{
		UDropByDropNotifications::ShowErrorNotification(FString::Printf(TEXT("Failed to save the \"%s\" erosion template!"), *Name));
		return;
	}

	UDropByDropNotifications::ShowSuccessNotification(FString::Printf(TEXT("Erosion template \"%s\" saved successfully!"), *Name));
}

/**
 * Retrieves a saved template from the data table and applies its values
 * to the current erosion settings instance. This overwrites any existing
 * configuration with the template's stored parameters.
 *
 * Validates erosion settings existence and checks if the requested template
 * exists before attempting to load.
 */
void UErosionTemplateManager::LoadTemplate(const FString& Name)
{
	// Ensure we have a valid erosion settings object to populate.
	if (!ErosionSettings.IsValid())
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"ErosionSettings\" resource is invalid!"));
		return;
	}

	if (Name.IsEmpty())
	{
		UDropByDropNotifications::ShowWarningNotification(TEXT("Cannot load an erosion template with an empty name!"));
		UE_LOG(LogDropByDropTemplate, Warning, TEXT("Cannot load an erosion template with an empty name!"));
		return;
	}

	// Attempt to load the template row from the data table.
	if (auto* Row = UPipelineLibrary::LoadErosionTemplate(Name))
	{
		// Populate the erosion settings with values from the loaded row.
		if (UPipelineLibrary::LoadRowIntoErosionFields(ErosionSettings, Row))
		{
			UDropByDropNotifications::ShowSuccessNotification(FString::Printf(TEXT("Erosion template \"%s\" loaded successfully!"), *Name));
			return;
		}
	}

	UDropByDropNotifications::ShowErrorNotification(FString::Printf(TEXT("Failed to load the \"%s\" erosion template!"), *Name));
	UE_LOG(LogDropByDropTemplate, Warning, TEXT("Failed to load the \"%s\" erosion template!"), *Name);
}

/**
 * Permanently removes a template from the erosion templates data table.
 * Delegates the deletion operation to "UPipelineLibrary" which handles
 * data table row removal.
 */
void UErosionTemplateManager::DeleteTemplate(const FString& Name)
{
	if (Name.IsEmpty())
	{
		UDropByDropNotifications::ShowWarningNotification(TEXT("Cannot delete an erosion template with an empty name!"));
		UE_LOG(LogDropByDropTemplate, Warning, TEXT("Cannot delete an erosion template with an empty name!"));
		return;
	}

	// Delegate deletion to the PipelineLibrary.
	if (!UPipelineLibrary::DeleteErosionTemplate(Name)) 
	{
		UDropByDropNotifications::ShowErrorNotification(FString::Printf(TEXT("Failed to delete the \"%s\" erosion template!"), *Name));
		return;
	}

	UDropByDropNotifications::ShowSuccessNotification(FString::Printf(TEXT("Erosion template \"%s\" deleted successfully!"), *Name));
}

/**
 * Establishes the connection between this manager and the erosion settings
 * it will operate on. This must be called before any save/load/delete operations
 * to ensure the manager has a valid target for its operations.
 */
void UErosionTemplateManager::SetErosionSettings(const TSharedPtr<FErosionSettings>& ErosionSettingsValue)
{
	ErosionSettings = ErosionSettingsValue;
}