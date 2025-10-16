// © Manuel Solano
// © Roberto Capparelli
#pragma once
#include "CoreMinimal.h"
#include "DropByDropSettings.h"

/**
 * SHeightMapPanel
 *
 * Slate widget providing a comprehensive UI for heightmap generation and import.
 *
 * Features:
 * - Procedural heightmap generation using Perlin noise algorithm.
 * - Three built-in presets (Low, Medium, High) for quick terrain setup.
 * - Fine-grained control over Perlin noise parameters:
 *   * Octaves: Number of noise layers for detail.
 *   * Persistence: Amplitude decay between octaves.
 *   * Lacunarity: Frequency multiplication between octaves.
 *   * Initial Scale: Base feature size.
 *   * Max Height Difference: Elevation range scaling.
 * - Random seed generation or fixed seed for reproducibility.
 * - External heightmap import from image files (PNG format).
 * - Real-time parameter validation and clamping.
 *
 * The panel supports two workflows:
 * 1. Procedural: Generate heightmaps using Perlin noise.
 * 2. External: Import heightmaps from external terrain authoring tools.
 */
class SHeightMapPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SHeightMapPanel) {}
		/** Shared pointer to Perlin noise generation parameters. */
		SLATE_ARGUMENT(TSharedPtr<FHeightMapGenerationSettings>, Heightmap)
		/** Shared pointer to external heightmap import settings. */
		SLATE_ARGUMENT(TSharedPtr<FExternalHeightMapSettings>, External)
	SLATE_END_ARGS()

	/**
	 * Constructs the heightmap panel widget and all its child UI elements.
	 * Initializes presets and generates an initial heightmap automatically.
	 * See .cpp implementation for detailed structure.
	 */
	void Construct(const FArguments& Args);

private:
	/** Perlin noise generation settings reference. */
	TSharedPtr<FHeightMapGenerationSettings> Heightmap;

	/** External heightmap import settings reference. */
	TSharedPtr<FExternalHeightMapSettings> External;

	// Preset Configurations.
	/** Low complexity preset - flatter terrain with minimal detail. */
	FHeightMapGenerationSettings PresetLight;

	/** Medium complexity preset - balanced terrain (default). */
	FHeightMapGenerationSettings PresetMedium;

	/** High complexity preset - mountainous terrain with maximum detail. */
	FHeightMapGenerationSettings PresetHeavy;

	/** Array of preset names for dropdown menu. */
	TArray<TSharedPtr<FString>> PresetItems;

	/** Currently selected preset in the dropdown. */
	TSharedPtr<FString> SelectedPreset;

	// Preset Management Methods.
	/**
	 * Initializes all three preset configurations with predefined Perlin noise parameters.
	 * Creates Low (flat), Medium (balanced), and High (mountainous) terrain presets.
	 */
	void InitPresets();

	/**
	 * Applies a preset configuration to the current heightmap settings.
	 * Copies all Perlin noise parameters from the preset to active settings.
	 *
	 * @param Preset - The preset configuration to apply.
	 */
	void ApplyPreset(const FHeightMapGenerationSettings& Preset);

	/**
	 * Populates the preset dropdown menu with available preset names.
	 * Creates shared string pointers for Low, Medium, and High presets.
	 */
	void BuildPresetItems();

	/**
	 * Applies a preset by matching its name to a configuration.
	 * Performs case-insensitive comparison. Defaults to Medium if no match.
	 *
	 * @param Name - The preset name ("Low", "Medium", or "High").
	 */
	void ApplyPresetByName(const FString& Name);

	/**
	 * Handles preset selection changes from the dropdown menu.
	 *
	 * @param NewSelection - The newly selected preset name.
	 * @param SelectInfo - Type of selection event (required by Slate API).
	 */
	void OnPresetChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);

	/**
	 * Generates a widget for displaying a preset option in the dropdown.
	 *
	 * @param Item - The preset name to display.
	 * @return - A text block widget containing the preset name.
	 */
	TSharedRef<SWidget> MakePresetItemWidget(TSharedPtr<FString> Item) const;

	// Button Callback Methods.
	/**
	 * Handles the "Low" preset button click event.
	 * Applies the low complexity preset configuration.
	 *
	 * @return FReply::Handled() to indicate event processing.
	 */
	FReply OnPresetLightClicked();

	/**
	 * Handles the "Medium" preset button click event.
	 * Applies the medium complexity preset configuration.
	 *
	 * @return FReply::Handled() to indicate event processing.
	 */
	FReply OnPresetMediumClicked();

	/**
	 * Handles the "High" preset button click event.
	 * Applies the high complexity preset configuration.
	 *
	 * @return FReply::Handled() to indicate event processing.
	 */
	FReply OnPresetHeavyClicked();

	/**
	 * Handles the "Create HeightMap" button click event.
	 * Generates a procedural heightmap using current Perlin noise settings.
	 * Resets external heightmap flags to use procedural generation.
	 *
	 * @return FReply::Handled() to indicate event processing.
	 */
	FReply OnCreateHeightmapClicked();

	/**
	 * Handles the "Import External Heightmap" button click event.
	 * Opens a file dialog for selecting an external heightmap image file.
	 * Allows import of heightmaps from external terrain authoring tools.
	 *
	 * @return FReply::Handled() to indicate event processing.
	 */
	FReply OnCreateFromExternalHeightmapClicked();

};