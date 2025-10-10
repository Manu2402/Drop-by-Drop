// © Manuel Solano
// © Roberto Capparelli

#pragma once
#include "CoreMinimal.h"

#pragma region ForwardDeclarations

class UErosionTemplateManager;
class SEditableTextBox;
class SSearchBox;

#pragma endregion

class STemplateBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STemplateBrowser) {}
		SLATE_ARGUMENT(TObjectPtr<UErosionTemplateManager>, TemplateManager)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private: // Members.
	TObjectPtr<UErosionTemplateManager> TemplateManager;

	TArray<TSharedPtr<FString>> Items;
	TSharedPtr<class SListView<TSharedPtr<FString>>> ListView;
	TSharedPtr<SEditableTextBox> NameTextBox;
	TSharedPtr<SSearchBox> SearchBox;

private: // Methods.
	void RefreshTemplates(const bool bFilter, const FString QueryFilter = TEXT("Default"));

	FReply OnSaveClicked();
	FReply OnLoadClicked();
	FReply OnDeleteClicked();

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
};
