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
class ALandscape;

#pragma endregion 

// Total number of wind direction options (including "Random").
#define WIND_DIRECTIONS 9

/**
 * SErosionPanel
 *
 * Slate widget that provides a comprehensive UI for configuring and applying
 * hydraulic erosion simulation to Unreal Engine landscapes.
 *
 * Features:
 * - Basic erosion parameters (cycles, wind direction).
 * - Advanced physical simulation parameters (inertia, capacity, gravity, etc.).
 * - Wind direction visualization preview.
 * - Template system for saving/loading/deleting erosion presets.
 * - Real-time parameter validation and clamping.
 *
 * The erosion simulation uses a particle-based approach where water droplets
 * flow across the terrain, picking up and depositing sediment to create
 * realistic erosion patterns.
 */
class SErosionPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SErosionPanel) {}
		/** Shared pointer to landscape generation settings. */
		SLATE_ARGUMENT(TSharedPtr<FLandscapeGenerationSettings>, Landscape)
		/** Shared pointer to erosion simulation parameters. */
		SLATE_ARGUMENT(TSharedPtr<FErosionSettings>, Erosion)
		/** Pointer to the currently active landscape actor in the scene. */
		SLATE_ARGUMENT(TObjectPtr<ALandscape>*, ActiveLandscape)
		/** Manager for saving/loading erosion configuration templates. */
		SLATE_ARGUMENT(TObjectPtr<UErosionTemplateManager>, TemplateManager)
	SLATE_END_ARGS()

	/**
	 * Constructs the erosion panel widget and all its child UI elements.
	 * See .cpp implementation for detailed structure.
	 */
	void Construct(const FArguments& Args);

private: // Members.
	/** Landscape generation settings reference. */
	TSharedPtr<FLandscapeGenerationSettings> Landscape;

	/** Erosion simulation parameters. */
	TSharedPtr<FErosionSettings> Erosion;

	/** Pointer to the active landscape being modified. */
	TObjectPtr<ALandscape>* ActiveLandscape;

	/** Template manager for preset save/load/delete functionality. */
	TObjectPtr<UErosionTemplateManager> TemplateManager;

	// Wind UI data.
	/** Array of available wind direction options for the dropdown menu. */
	TArray<TSharedPtr<FString>> WindDirections;

	/** Currently selected wind direction in the dropdown. */
	TSharedPtr<FString> CurrentWindDirection;

	/** Reference to the wind direction combo box widget. */
	TSharedPtr<SComboBox<TSharedPtr<FString>>> WindCombo;

	/**
	 * String names for all wind directions, mapped to "EWindDirection" enum.
	 */
	const TCHAR* WindDirectionsNames[WIND_DIRECTIONS] =
	{
		TEXT("Random"),

		TEXT("North"),
		TEXT("South"),
		TEXT("East"),
		TEXT("West"),
		TEXT("North_West"),
		TEXT("North_East"),
		TEXT("South_West"),
		TEXT("South_East")
	};

private: // Methods.
	/**
	 * Initializes the wind direction dropdown options.
	 * Populates "WindDirections" array from "WindDirectionsName"s and sets default selection.
	 */
	void BuildWindDirections();

	/**
	 * Handles the "Erode" button click event.
	 * Triggers the erosion generation process on the active landscape.
	 *
	 * @return FReply::Handled() to indicate the event was processed.
	 */
	FReply OnErodeClicked();

};