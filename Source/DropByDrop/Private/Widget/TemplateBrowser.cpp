// © Manuel Solano
// © Roberto Capparelli

#include "Widget/TemplateBrowser.h"
#include "Widgets/Input/SSearchBox.h"
#include "ErosionTemplateManager.h"
#include "DropByDropLogger.h"

// Fallback string for empty or invalid template names.
#define EMPTY_STRING ""

void STemplateBrowser::Construct(const FArguments& Args)
{
	// Store reference to the template manager from arguments.
	TemplateManager = Args._TemplateManager;

	// Initial load of all available templates (no filtering).
	RefreshTemplates(false);

	// Construct the main vertical layout.
	ChildSlot
		[
			SNew(SVerticalBox)
				// Top: Search box for filtering templates by name.
				+ SVerticalBox::Slot().AutoHeight().Padding(4)
				[
					SAssignNew(SearchBox, SSearchBox)
						// Lambda callback: triggers filtering when search text changes.
						.OnTextChanged_Lambda([this](const FText& Text) { RefreshTemplates(true, Text.ToString()); })
						.HintText(FText::FromString("Search a template..."))
				]
				// Middle: Scrollable list view displaying template names.
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(4)
				[
					SAssignNew(ListView, SListView<TSharedPtr<FString>>)
						.ListItemsSource(&Items) // Bind to "Items" array.
						.OnGenerateRow(this, &STemplateBrowser::OnGenerateRow) // Custom row generator.
						.SelectionMode(ESelectionMode::Single) // Allow single selection.
				]
				// Bottom: Text input for entering new template names.
				+ SVerticalBox::Slot().AutoHeight().Padding(4)
				[
					SAssignNew(NameTextBox, SEditableTextBox)
						.HintText(FText::FromString("Template name"))
				]
				// Bottom: Action buttons (Save, Load, Delete).
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(4)
				[
					SNew(SHorizontalBox)
						// Save button: saves current settings as new template.
						+ SHorizontalBox::Slot().AutoWidth().Padding(2)
						[
							SNew(SButton)
								.Text(FText::FromString("Save"))
								.OnClicked(this, &STemplateBrowser::OnSaveClicked)
						]
						// Load button: applies selected template to current erosion settings.
						+ SHorizontalBox::Slot().AutoWidth().Padding(2)
						[
							SNew(SButton)
								.Text(FText::FromString("Load"))
								.OnClicked(this, &STemplateBrowser::OnLoadClicked)
						]
						// Delete button: removes selected template from storage.
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
	// Validate template manager is valid before accessing.
	if (!IsValid(TemplateManager))
	{
		UE_LOG(LogDropByDropTemplate, Error, TEXT("The \"TemplateManager\" widget is invalid!"));
		return;
	}

	// Clear current items before repopulating.
	Items.Empty();

	TArray<FString> TemplateNames;

	// Get template names based on filter state.
	if (bFilter)
	{
		// Search mode: find templates matching the query string.
		TemplateNames = TemplateManager->FindTemplateNames(Query);
	}
	else
	{
		// Normal mode: retrieve all available template names.
		TemplateNames = TemplateManager->GetAllTemplateNames();
	}

	// Convert template names to shared pointers for list view.
	for (const FString& TemplateName : TemplateNames)
	{
		Items.Add(MakeShared<FString>(TemplateName));
	}

	// Request list view to refresh and display updated items.
	if (!ListView.IsValid())
	{
		UE_LOG(LogDropByDropTemplate, Warning, TEXT("The \"ListView\" widget is invalid!"));
		return;
	}

	ListView->RequestListRefresh();
}

FReply STemplateBrowser::OnSaveClicked()
{
	// Validate manager and text box before attempting save.
	if (IsValid(TemplateManager) && NameTextBox.IsValid())
	{
		// Save current erosion settings with the name entered in text box.
		TemplateManager->SaveCurrentAsTemplate(NameTextBox->GetText().ToString());
	}

	// Refresh list to show the newly saved template.
	RefreshTemplates(false);

	return FReply::Handled();
}

FReply STemplateBrowser::OnLoadClicked()
{
	// Validate manager and list view before attempting load.
	if (!IsValid(TemplateManager) || !ListView.IsValid())
	{
		return FReply::Handled();
	}

	// Get currently selected items from list view.
	TArray<TSharedPtr<FString>> SelectedItems = ListView->GetSelectedItems();

	// Load the selected template if any item is selected.
	if (SelectedItems.Num() > 0)
	{
		// Apply the first selected template's settings to current erosion configuration.
		TemplateManager->LoadTemplate(*SelectedItems[0].Get());
	}

	return FReply::Handled();
}

FReply STemplateBrowser::OnDeleteClicked()
{
	// Validate manager and list view before attempting delete.
	if (!IsValid(TemplateManager) || !ListView.IsValid())
	{
		return FReply::Handled();
	}

	// Get currently selected items from list view.
	TArray<TSharedPtr<FString>> SelectedItems = ListView->GetSelectedItems();

	// Delete the selected template if any item is selected.
	if (SelectedItems.Num() > 0)
	{
		// Permanently remove the first selected template from storage.
		TemplateManager->DeleteTemplate(*SelectedItems[0].Get());
	}

	// Refresh list to reflect deletion.
	RefreshTemplates(false);

	return FReply::Handled();
}

TSharedRef<ITableRow> STemplateBrowser::OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Create a new table row containing a text block.
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(STextBlock)
				// Display the template name, or empty string if item is invalid.
				.Text(FText::FromString(Item.IsValid() ? *Item.Get() : TEXT(EMPTY_STRING)))
		];
}