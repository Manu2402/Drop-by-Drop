#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class UErosionTemplateManager;

class STemplateBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STemplateBrowser) {}
	SLATE_ARGUMENT(TObjectPtr<UErosionTemplateManager>, TemplateManager)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	TObjectPtr<UErosionTemplateManager> TemplateManager;

	TArray<TSharedPtr<FString>> Items;
	TSharedPtr<class SListView<TSharedPtr<FString>>> ListView;
	TSharedPtr<class SEditableTextBox> NameTextBox;
	TSharedPtr<class SSearchBox> SearchBox;

	void Reload();
	void FilterAndRefresh(const FString& Query);

	FReply OnSaveClicked();
	FReply OnLoadClicked();
	FReply OnDeleteClicked();

	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);
};
