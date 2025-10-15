// © Manuel Solano
// © Roberto Capparelli

#pragma once
#include "CoreMinimal.h"

#pragma region ForwardDeclarations

struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
struct FLandscapeGenerationSettings;
struct FErosionSettings;
class UErosionTemplateManager;

class SHeightMapPanel;
class SLandscapePanel;
class SErosionPanel;
class SWidgetSwitcher;

class SImage;
class UTexture2D;

class ALandscape;

#pragma endregion

/**
 * SRootPanel - Main UI container for the DropByDrop plugin.
 *
 * This compound widget serves as the primary interface hub, organizing three main workflow panels:
 * - HeightMap generation and configuration.
 * - Landscape creation and management.
 * - Erosion simulation and effects.
 *
 * Layout structure:
 * - Left: Navigation sidebar with panel selection buttons.
 * - Center: Widget switcher displaying the active panel.
 * - Right: Quick action buttons and heightmap preview.
 */
class SRootPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SRootPanel) {}
		/** Pointer to the currently active landscape in the level. */
		SLATE_ARGUMENT(TObjectPtr<ALandscape>*, ActiveLandscape)
	SLATE_END_ARGS()

	/** Constructs the root panel and initializes all child widgets and settings. */
	void Construct(const FArguments& InArgs);

private:
	/** Configuration for procedural heightmap generation (noise, octaves, etc.). */
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;

	/** Settings for importing external heightmap files. */
	TSharedPtr<FExternalHeightMapSettings> External;

	/** Parameters for landscape creation. */
	TSharedPtr<FLandscapeGenerationSettings> Landscape;

	/** Erosion simulation parameters. */
	TSharedPtr<FErosionSettings> Erosion;

	/** Pointer to the active landscape in the current level. */
	TObjectPtr<ALandscape>* ActiveLandscape;

	/** Manages erosion preset templates (save/load/delete configurations). */
	TObjectPtr<UErosionTemplateManager> ErosionTemplateManager;

	/** Widget switcher that toggles between HeightMap, Landscape, and Erosion panels. */
	TSharedPtr<SWidgetSwitcher> Switcher;

	/** Slate brush for rendering the heightmap preview image. */
	TSharedPtr<FSlateBrush> RightPreviewBrush;

	/** Image widget displaying the heightmap preview in the right panel. */
	TSharedPtr<SImage> RightPreviewImage;

	/** Strong reference to the loaded heightmap texture to prevent garbage collection. */
	TStrongObjectPtr<UTexture2D> RightPreviewTexture;

	/** Text blocks for navigation buttons, used to update font styles on selection. */
	TArray<TSharedPtr<STextBlock>> NavButtonTexts;

	/** Currently active panel index: 0 = HeightMap, 1 = Landscape, 2 = Erosion. */
	int32 ActiveIndex = 0;

private:
	/**
	 * Builds the left navigation sidebar.
	 * @return Widget - Containing vertical stack of navigation buttons.
	 */
	TSharedRef<SWidget> BuildSidebar();

	/**
	 * Builds the center content area with panel switcher.
	 * @return Widget - Switcher containing HeightMap, Landscape, and Erosion panels.
	 */
	TSharedRef<SWidget> BuildCenter();

	/**
	 * Builds the right panel with quick actions and preview.
	 * @return Widget  - Containing action buttons and heightmap preview image.
	 */
	TSharedRef<SWidget> BuildRightPanel();

	/**
	 * Handles navigation button clicks to switch between panels.
	 * Updates active index, button styling, and widget switcher state.
	 * @param Index - Target panel index (0 = HeightMap, 1 = Landscape, 2 = Erosion).
	 * @return FReply::Handled() to consume the event.
	 */
	FReply OnNavClicked(const int32 Index);

	/**
	 * Quick action: Creates and saves a heightmap texture from current settings.
	 * Uses procedural generation parameters from Heightmap settings.
	 * @return FReply::Handled() after creating heightmap.
	 */
	FReply OnActionCreateHeightMap();

	/**
	 * Quick action: Creates a landscape from internally generated heightmap.
	 * @return FReply::Handled() after creating landscape.
	 */
	FReply OnActionCreateLandscapeInternal();

	/**
	 * Quick action: Opens file dialog, imports external heightmap, and creates landscape.
	 * @return FReply::Handled() after import and landscape creation (or if cancelled).
	 */
	FReply OnActionImportAndCreateLandscapeExternal();

	/**
	 * Reloads and updates the heightmap preview image in the right panel.
	 * Called after heightmap creation or modification to keep preview in sync.
	 * Loads texture from predefined asset path and updates brush/image widget.
	 */
	void RefreshRightPreview();

};