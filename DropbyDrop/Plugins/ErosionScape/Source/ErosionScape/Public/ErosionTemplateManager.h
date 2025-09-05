#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ErosionTemplateManager.generated.h"

UCLASS()
class UErosionTemplateManager : public UObject
{
	GENERATED_BODY()
public:
	TArray<FString> GetAllTemplateNames() const;
	TArray<FString> FindTemplateNames(const FString& Query) const;

	void SaveCurrentAsTemplate(const FString& Name);
	void LoadTemplate(const FString& Name);
	void DeleteTemplate(const FString& Name);
};
