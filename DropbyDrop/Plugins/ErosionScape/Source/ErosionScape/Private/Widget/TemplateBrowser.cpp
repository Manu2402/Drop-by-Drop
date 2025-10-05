#include "Widget/TemplateBrowser.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Text/STextBlock.h"
#include "ErosionTemplateManager.h"

void STemplateBrowser::Construct(const FArguments& Args)
{
	TemplateManager = Args._TemplateManager;
	Reload();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SAssignNew(SearchBox, SSearchBox)
			.OnTextChanged_Lambda([this](const FText& T) { FilterAndRefresh(T.ToString()); })
			.HintText(FText::FromString("Search a template..."))
		]

		+ SVerticalBox::Slot().FillHeight(1.f).Padding(4)
		[
			SAssignNew(ListView, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&Items)
			.OnGenerateRow(this, &STemplateBrowser::OnGenerateRow)
			.SelectionMode(ESelectionMode::Single)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SAssignNew(NameTextBox, SEditableTextBox).HintText(FText::FromString("Template name"))
		]

		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton).Text(FText::FromString("Save")).OnClicked(this, &STemplateBrowser::OnSaveClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton).Text(FText::FromString("Load")).OnClicked(this, &STemplateBrowser::OnLoadClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton).Text(FText::FromString("Delete")).OnClicked(this, &STemplateBrowser::OnDeleteClicked)
			]
		]
	];
}

void STemplateBrowser::Reload()
{
	Items.Empty();
	if (TemplateManager)
	{
		for (const FString& N : TemplateManager->GetAllTemplateNames())
			Items.Add(MakeShared<FString>(N));
	}
	if (ListView) ListView->RequestListRefresh();
}

void STemplateBrowser::FilterAndRefresh(const FString& Query)
{
	Items.Empty();
	if (TemplateManager)
	{
		for (const FString& N : TemplateManager->FindTemplateNames(Query))
			Items.Add(MakeShared<FString>(N));
	}
	if (ListView) ListView->RequestListRefresh();
}

FReply STemplateBrowser::OnSaveClicked()
{
	if (TemplateManager && NameTextBox) TemplateManager->SaveCurrentAsTemplate(NameTextBox->GetText().ToString());
	Reload();
	return FReply::Handled();
}

FReply STemplateBrowser::OnLoadClicked()
{
	if (!TemplateManager || !ListView) return FReply::Handled();
	TArray<TSharedPtr<FString>> Sel = ListView->GetSelectedItems();
	if (Sel.Num() > 0) TemplateManager->LoadTemplate(*Sel[0].Get());
	return FReply::Handled();
}

FReply STemplateBrowser::OnDeleteClicked()
{
	if (!TemplateManager || !ListView) return FReply::Handled();
	TArray<TSharedPtr<FString>> Sel = ListView->GetSelectedItems();
	if (Sel.Num() > 0) TemplateManager->DeleteTemplate(*Sel[0].Get());
	Reload();
	return FReply::Handled();
}

TSharedRef<ITableRow> STemplateBrowser::OnGenerateRow(TSharedPtr<FString> Item,
                                                      const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item.Get() : TEXT("")))
		];
}
