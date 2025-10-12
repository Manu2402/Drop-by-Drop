// © Roberto Capparelli

#pragma once

#include "CoreMinimal.h"
#include "ErosionTemplateManager.generated.h"

#pragma region ForwardDeclarations

struct FErosionSettings;

#pragma endregion

UCLASS()
class UErosionTemplateManager : public UObject
{
	GENERATED_BODY()

private:
	TSharedPtr<FErosionSettings> ErosionSettings;

public:
	TArray<FString> GetAllTemplateNames() const;
	TArray<FString> FindTemplateNames(const FString& QueryFilter) const;

	void SaveCurrentAsTemplate(const FString& Name);
	void LoadTemplate(const FString& Name);
	void DeleteTemplate(const FString& Name);

	void SetErosionSettings(const TSharedPtr<FErosionSettings>& ErosionSettingsValue);

};
