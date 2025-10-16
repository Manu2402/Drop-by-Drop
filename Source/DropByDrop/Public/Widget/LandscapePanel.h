// © Manuel Solano
// © Roberto Capparelli

#pragma once
#include "CoreMinimal.h"

#pragma region ForwardDeclarations

struct FLandscapeGenerationSettings;
struct FHeightMapGenerationSettings;
struct FExternalHeightMapSettings;
class ALandscape;

#pragma endregion

/**
 * SLandscapePanel
 *
 * A Slate widget panel that provides UI controls for landscape generation and manipulation.
 * This custom compound widget allows users to configure landscape parameters, create new
 * landscapes from heightmaps, and split existing landscapes into proxy actors for
 * world partition streaming.
 *
 * Key Features:
 * - Landscape size configuration (in kilometers).
 * - External heightmap scaling controls (on X, Y, Z axes).
 * - World partition cell size settings.
 * - Landscape creation from heightmaps.
 * - Landscape splitting into proxy actors.
 */
class SLandscapePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLandscapePanel) {}
		// Settings for landscape generation (size, world partition, etc.).
		SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)

		// Settings for heightmap generation (internal heightmap parameters).
		SLATE_ARGUMENT(TSharedPtr<FHeightMapGenerationSettings>, Heightmap)

		// Settings for external heightmap import (scaling, file path, etc.).
		SLATE_ARGUMENT(TSharedPtr<FExternalHeightMapSettings>, External)

		// Pointer to the currently active landscape in the level.
		// Used for operations like splitting into proxies.
		SLATE_ARGUMENT(TObjectPtr<ALandscape>*, ActiveLandscape)
	SLATE_END_ARGS()

	/**
	 * Constructs the widget hierarchy and binds data to UI controls.
	 *
	 * @param InArgs - Construction arguments containing references to landscape settings
	 *                 and the active landscape.
	 */
	void Construct(const FArguments& InArgs);

private:
	// Shared pointer to landscape generation settings (kilometers, cell size, etc.).
	TSharedPtr<FLandscapeGenerationSettings> Landscape;

	// Shared pointer to heightmap generation settings (for internal heightmap creation).
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;

	// Shared pointer to external heightmap settings (scaling for imported heightmaps).
	TSharedPtr<FExternalHeightMapSettings> External;

	// Pointer to the currently selected landscape in the level.
	// Allows the panel to perform operations on the active landscape.
	TObjectPtr<ALandscape>* ActiveLandscape;

	/**
	 * Handler for the "Create Landscape" button click event.
	 * Generates a new landscape based on the configured heightmap and settings.
	 *
	 * @return FReply::Handled() to indicate the event was processed.
	 */
	FReply OnCreateLandscapeClicked();

	/**
	 * Handler for the "Split in Proxies" button click event.
	 * Divides the active landscape into multiple proxy actors for world partition streaming.
	 * This button is only enabled when a valid landscape is selected that hasn't been split yet.
	 *
	 * @return FReply::Handled() to indicate the event was processed.
	 */
	FReply OnSplitInProxiesClicked();

};