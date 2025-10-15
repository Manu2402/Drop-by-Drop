// © Manuel Solano
// © Roberto Capparelli

#pragma once
#include "CoreMinimal.h"

#pragma region ForwardDeclarations

class UErosionTemplateManager;
class SEditableTextBox;
class SSearchBox;

#pragma endregion

/**
 * STemplateBrowser - UI widget for managing erosion preset templates.
 *
 * This compound widget provides a complete interface for managing erosion configuration templates:
 * - Search and filter templates by name.
 * - Browse available templates in a scrollable list.
 * - Save current erosion settings as a new template.
 * - Load existing templates to apply their settings.
 * - Delete unwanted templates.
 *
 * Layout structure:
 * - Top: Search box for filtering templates.
 * - Middle: Scrollable list view displaying template names.
 * - Bottom: Text input for new template names.
 * - Bottom: Action buttons (Save, Load, Delete).
 */
class STemplateBrowser : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STemplateBrowser) {}
		/** Template manager that handles template storage and retrieval operations. */
		SLATE_ARGUMENT(TObjectPtr<UErosionTemplateManager>, TemplateManager)
	SLATE_END_ARGS()

	/** Constructs the template browser UI and initializes the template list. */
	void Construct(const FArguments& InArgs);

private:
	/** Manager responsible for saving, loading, and deleting erosion templates. */
	TObjectPtr<UErosionTemplateManager> TemplateManager;

	/** Array of template names currently displayed in the list view. */
	TArray<TSharedPtr<FString>> Items;

	/** List view widget displaying all available template names. */
	TSharedPtr<class SListView<TSharedPtr<FString>>> ListView;

	/** Text input box for entering new template names when saving. */
	TSharedPtr<SEditableTextBox> NameTextBox;

	/** Search box for filtering templates by name in real-time. */
	TSharedPtr<SSearchBox> SearchBox;

private:
	/**
	 * Refreshes the template list from the template manager.
	 *
	 * @param bFilter - If true, applies search query filtering; if false, shows all templates.
	 * @param QueryFilter - Search string to filter template names (default: "Default").
	 */
	void RefreshTemplates(const bool bFilter, const FString QueryFilter = TEXT("Default"));

	/**
	 * Handles Save button click - saves current erosion settings as a new template.
	 * Uses the name entered in "NameTextBox" as the template identifier.
	 * 
	 * @return FReply::Handled() after saving template.
	 */
	FReply OnSaveClicked();

	/**
	 * Handles Load button click - applies selected template's settings to current erosion configuration.
	 * Loads the template selected in the ListView.
	 * 
	 * @return FReply::Handled() after loading template (or if no selection).
	 */
	FReply OnLoadClicked();

	/**
	 * Handles Delete button click - permanently removes selected template from storage.
	 * Deletes the template selected in the ListView and refreshes the list.
	 * 
	 * @return FReply::Handled() after deleting template (or if no selection).
	 */
	FReply OnDeleteClicked();

	/**
	 * Generates a table row widget for displaying a template name in the list view.
	 * Called automatically by SListView for each visible item.
	 *
	 * @param Item - Shared pointer to the template name string.
	 * @param OwnerTable - Reference to the parent table view widget.
	 * @return TableRow - Widget containing a text block with the template name.
	 */
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable);

};