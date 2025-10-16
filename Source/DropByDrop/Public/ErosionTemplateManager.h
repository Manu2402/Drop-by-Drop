// © Roberto Capparelli

#pragma once
#include "CoreMinimal.h"
#include "ErosionTemplateManager.generated.h"

#pragma region ForwardDeclarations

struct FErosionSettings;

#pragma endregion

/**
 * UErosionTemplateManager
 *
 * Manager class responsible for handling erosion preset templates.
 * Provides functionality to save, load, delete, and query erosion configuration
 * presets stored in a data table. This allows users to create and reuse
 * erosion settings across different terrain generation sessions.
 *
 * Inherits from UObject to integrate with Unreal's reflection system and
 * garbage collection.
 */
UCLASS()
class UErosionTemplateManager : public UObject
{
	GENERATED_BODY()

private:
	/**
	 * Shared pointer to the current erosion settings being managed.
	 * This reference is used when saving the current configuration as a template
	 * or when loading a template to populate the active settings.
	 */
	TSharedPtr<FErosionSettings> ErosionSettings;

public:
	/**
	 * Retrieves all available erosion template names from the data table.
	 * Excludes the "None" entry which is typically used as a null/default row.
	 *
	 * @return Array of strings containing all template names.
	 */
	TArray<FString> GetAllTemplateNames() const;

	/**
	 * Searches for template names that match a given query filter.
	 * Uses a "starts with" matching strategy for filtering results.
	 * If the query is empty, returns all templates.
	 *
	 * @param QueryFilter - String prefix to filter template names by.
	 * @return Array of template names matching the filter criteria.
	 */
	TArray<FString> FindTemplateNames(const FString& QueryFilter) const;

	/**
	 * Saves the current erosion settings as a new named template in the data table.
	 * Allows users to preserve their custom erosion configurations for future use.
	 *
	 * @param Name - The name to assign to the new template.
	 */
	void SaveCurrentAsTemplate(const FString& Name);

	/**
	 * Loads a saved erosion template by name and applies it to the current.
	 * erosion settings. Overwrites the active configuration with the template values.
	 *
	 * @param Name - The name of the template to load.
	 */
	void LoadTemplate(const FString& Name);

	/**
	 * Removes a template from the data table permanently.
	 * Used for cleaning up unwanted or obsolete presets.
	 *
	 * @param Name - The name of the template to delete.
	 */
	void DeleteTemplate(const FString& Name);

	/**
	 * Sets the erosion settings instance that this manager will operate on.
	 * Must be called before saving or loading templates to establish the
	 * connection between the manager and the active settings.
	 *
	 * @param ErosionSettingsValue - Shared pointer to the erosion settings to manage.
	 */
	void SetErosionSettings(const TSharedPtr<FErosionSettings>& ErosionSettingsValue);

};