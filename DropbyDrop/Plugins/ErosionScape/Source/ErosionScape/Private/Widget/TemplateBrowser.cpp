#include "Widget/TemplateBrowser.h"

#include "Widgets/Input/SSearchBox.h"
#include "ErosionTemplateManager.h"

#define EMPTY_STRING ""

void STemplateBrowser::Construct(const FArguments& Args)
{
	TemplateManager = Args._TemplateManager;

	RefreshTemplates(false);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			SAssignNew(SearchBox, SSearchBox)
				.OnTextChanged_Lambda([this](const FText& Text){ RefreshTemplates(true, Text.ToString()); })
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
			SAssignNew(NameTextBox, SEditableTextBox)
				.HintText(FText::FromString("Template name"))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton)
					.Text(FText::FromString("Save"))
					.OnClicked(this, &STemplateBrowser::OnSaveClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton)
					.Text(FText::FromString("Load"))
					.OnClicked(this, &STemplateBrowser::OnLoadClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton)
					.Text(FText::FromString("Delete"))
					.OnClicked(this, &STemplateBrowser::OnDeleteClicked)
			]
		]
	];
}

void STemplateBrowser::RefreshTemplates(const bool bFilter, const FString Query)
{
	if (!IsValid(TemplateManager))
	{
		return;
	}

	Items.Empty();

	TArray<FString> TemplateNames;

	if (bFilter)
	{
		TemplateNames = TemplateManager->FindTemplateNames(Query);
	}
	else
	{
		TemplateNames = TemplateManager->GetAllTemplateNames();
	}

	for (const FString& TemplateName : TemplateNames)
	{
		Items.Add(MakeShared<FString>(TemplateName));
	}

	if (!ListView.IsValid())
	{
		return;
	}
	
	ListView->RequestListRefresh();
}

FReply STemplateBrowser::OnSaveClicked()
{
	if (IsValid(TemplateManager) && NameTextBox.IsValid())
	{
		TemplateManager->SaveCurrentAsTemplate(NameTextBox->GetText().ToString());
	}

	RefreshTemplates(false);

	return FReply::Handled();
}

FReply STemplateBrowser::OnLoadClicked()
{
	if (!IsValid(TemplateManager) || !ListView.IsValid())
	{
		return FReply::Handled();
	}

	TArray<TSharedPtr<FString>> SelectedItems = ListView->GetSelectedItems();
	if (SelectedItems.Num() > 0)
	{
		TemplateManager->LoadTemplate(*SelectedItems[0].Get());
	}

	return FReply::Handled();
}

FReply STemplateBrowser::OnDeleteClicked()
{
	if (!IsValid(TemplateManager) || !ListView.IsValid())
	{
		return FReply::Handled();
	}

	TArray<TSharedPtr<FString>> SelectedItems = ListView->GetSelectedItems();
	if (SelectedItems.Num() > 0)
	{
		TemplateManager->DeleteTemplate(*SelectedItems[0].Get());
	}

	RefreshTemplates(false);

	return FReply::Handled();
}

TSharedRef<ITableRow> STemplateBrowser::OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock)
				.Text(FText::FromString(Item.IsValid() ? *Item.Get() : TEXT(EMPTY_STRING)))
		];
}
