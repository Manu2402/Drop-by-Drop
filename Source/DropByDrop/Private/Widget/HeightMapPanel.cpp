// © Manuel Solano
// © Roberto Capparelli

#include "Widget/HeightMapPanel.h"

#include "Widgets/Input/SNumericEntryBox.h"
#include "Libraries/PipelineLibrary.h"
#include "DropByDropNotifications.h"
#include "DropByDropLogger.h"

#define EMPTY_STRING ""
#define DEFAULT_PRESET_INDEX 1 // "Medium" preset selected by default.

#pragma region Presets
/**
 * Initializes the three built-in heightmap generation presets.
 *
 * Creates Low, Medium, and High complexity presets by copying the current
 * heightmap settings and adjusting key Perlin noise parameters:
 * - Low: Flatter terrain with fewer octaves and lower height variation.
 * - Medium: Balanced terrain with moderate detail and elevation changes.
 * - High: Mountainous terrain with many octaves and maximum height variation.
 *
 * These presets provide quick starting points for different terrain types.
 */
void SHeightMapPanel::InitPresets()
{
	if (!Heightmap.IsValid())
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"Heightmap\" resource is invalid!"));
		return;
	}

	// Preset "Low" - Creates flatter, gentler terrain.
	PresetLight = *Heightmap;
	PresetLight.NumOctaves = 4;              // Fewer layers of detail.
	PresetLight.Persistence = 0.35f;         // Lower amplitude for successive octaves.
	PresetLight.Lacunarity = 1.8f;           // Moderate frequency increase.
	PresetLight.InitialScale = 1.2f;         // Larger base features.
	PresetLight.MaxHeightDifference = 0.25f; // Minimal elevation change.

	// Preset "Medium" - Balanced terrain with moderate features.
	PresetMedium = *Heightmap;
	PresetMedium.NumOctaves = 6;              // Moderate detail layers.
	PresetMedium.Persistence = 0.40f;         // Balanced amplitude.
	PresetMedium.Lacunarity = 2.0f;           // Standard frequency doubling.
	PresetMedium.InitialScale = 1.5f;         // Medium-sized base features.
	PresetMedium.MaxHeightDifference = 0.8f;  // Significant elevation variation.

	// Preset "High" - Creates mountainous, highly detailed terrain.
	PresetHeavy = *Heightmap;
	PresetHeavy.NumOctaves = 10;             // Maximum detail layers.
	PresetHeavy.Persistence = 0.45f;         // Higher amplitude for fine details.
	PresetHeavy.Lacunarity = 1.9f;           // Slightly compressed frequency increase.
	PresetHeavy.InitialScale = 2.4f;         // Large base features.
	PresetHeavy.MaxHeightDifference = 1.0f;  // Maximum elevation variation.
}

/**
 * Applies a preset configuration to the current heightmap settings.
 * Copies all relevant Perlin noise parameters from the preset to the
 * active heightmap configuration, allowing users to quickly switch
 * between predefined terrain complexity levels.
 */
void SHeightMapPanel::ApplyPreset(const FHeightMapGenerationSettings& Preset)
{
	if (!Heightmap.IsValid())
	{
		UDropByDropNotifications::ShowErrorNotification("Unable to apply the heightmap preset!");
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"Heightmap\" resource is invalid!"));
		return;
	}

	// Copy all Perlin noise parameters from preset to active settings.
	Heightmap->Seed = Preset.Seed;
	Heightmap->NumOctaves = Preset.NumOctaves;
	Heightmap->Persistence = Preset.Persistence;
	Heightmap->Lacunarity = Preset.Lacunarity;
	Heightmap->InitialScale = Preset.InitialScale;
	Heightmap->Size = Preset.Size;
	Heightmap->MaxHeightDifference = Preset.MaxHeightDifference;
}

/**
 * Populates the preset dropdown menu with available preset names.
 * Creates shared string pointers for Low, Medium, and High presets
 * and sets Medium as the default selection.
 */
void SHeightMapPanel::BuildPresetItems()
{
	PresetItems.Empty();
	PresetItems.Add(MakeShared<FString>(TEXT("Low")));
	PresetItems.Add(MakeShared<FString>(TEXT("Medium")));
	PresetItems.Add(MakeShared<FString>(TEXT("High")));

	// Select "Medium" preset by default.
	SelectedPreset = PresetItems[DEFAULT_PRESET_INDEX];
}

/**
 * Applies a preset by matching its name to a predefined configuration.
 * Performs case-insensitive string comparison to map preset names
 * to their corresponding configurations. Defaults to Medium if the
 * name doesn't match Low or High.
 */
void SHeightMapPanel::ApplyPresetByName(const FString& Name)
{
	if (Name.Equals(TEXT("Low"), ESearchCase::IgnoreCase))
	{
		ApplyPreset(PresetLight);
		return;
	}

	if (Name.Equals(TEXT("High"), ESearchCase::IgnoreCase))
	{
		ApplyPreset(PresetHeavy);
		return;
	}

	// Default to Medium preset if name doesn't match Low or High.
	ApplyPreset(PresetMedium);
}

/**
 * Handles preset selection changes from the dropdown menu.
 * Called when the user selects a different preset from the combo box.
 * Updates the selected preset reference and applies the corresponding
 * configuration to the heightmap settings.
 */
void SHeightMapPanel::OnPresetChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type)
{
	if (!NewSelection.IsValid())
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The selected heightmap's preset is invalid!"));
		return;
	}

	SelectedPreset = NewSelection;
	ApplyPresetByName(*NewSelection);
}

/**
 * Generates a widget for displaying a preset option in the dropdown menu.
 * Creates a simple text block showing the preset name for each dropdown item.
 */
TSharedRef<SWidget> SHeightMapPanel::MakePresetItemWidget(TSharedPtr<FString> Item) const
{
	return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : TEXT(EMPTY_STRING)));
}

/**
 * Handles the "Low" preset button click event.
 */
FReply SHeightMapPanel::OnPresetLightClicked()
{
	ApplyPreset(PresetLight);
	return FReply::Handled();
}

/**
 * Handles the "Medium" preset button click event.
 */
FReply SHeightMapPanel::OnPresetMediumClicked()
{
	ApplyPreset(PresetMedium);
	return FReply::Handled();
}

/**
 * Handles the "High" preset button click event.
 */
FReply SHeightMapPanel::OnPresetHeavyClicked()
{
	ApplyPreset(PresetHeavy);
	return FReply::Handled();
}
#pragma endregion

/**
 * Constructs the heightmap panel UI with all parameter controls.
 *
 * Creates a comprehensive interface for heightmap generation including:
 * - Preset selection dropdown for quick configuration.
 * - Random seed toggle.
 * - Advanced Perlin noise parameters (octaves, persistence, lacunarity, etc.).
 * - Heightmap generation button.
 * - External heightmap import functionality.
 *
 * Initializes presets and automatically generates an initial heightmap.
 */
void SHeightMapPanel::Construct(const FArguments& Args)
{
	// Store references to heightmap generation settings.
	Heightmap = Args._Heightmap;
	External = Args._External;

	// Validate critical references.
	check(Heightmap.IsValid() && External.IsValid());

	// Initialize preset configurations.
	InitPresets();
	BuildPresetItems();

	// Generate initial heightmap automatically.
	OnCreateHeightmapClicked();

	ChildSlot
		[
			SNew(SVerticalBox)

				// --- Panel Title ---
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString("HeightMap"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]

				// --- Preset Selection Dropdown ---
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Preset"))
								.ToolTipText(FText::FromString("List of three presets that define three different versions of heightmap generation configurations, starting from a flatter heightmap to a more mountainous heightmap."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
						[
							SNew(SComboBox<TSharedPtr<FString>>)
								.OptionsSource(&PresetItems)
								.InitiallySelectedItem(SelectedPreset)
								.OnSelectionChanged(this, &SHeightMapPanel::OnPresetChanged)
								.OnGenerateWidget(this, &SHeightMapPanel::MakePresetItemWidget)
								[
									// Display currently selected preset name.
									SNew(STextBlock)
										.Text_Lambda([this]() { return FText::FromString(SelectedPreset.IsValid() ? *SelectedPreset : TEXT("Select Preset")); })
								]
						]
				]

			// --- Randomize Seed Checkbox ---
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Randomize Seed"))
								.ToolTipText(FText::FromString("Whether a new random seed is generated for each heightmap. If set to true, a random seed will be used, creating a unique heightmap each time. If false, the specified seed will be used, ensuring consistent results."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
						[
							SNew(SCheckBox)
								.IsChecked_Lambda([this]() { return Heightmap->bRandomizeSeed ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
								.OnCheckStateChanged_Lambda([this](ECheckBoxState State) { Heightmap->bRandomizeSeed = (State == ECheckBoxState::Checked); })
						]
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]

				// --- Advanced Perlin Noise Parameters (Collapsible) ---
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SExpandableArea)
						.InitiallyCollapsed(true)
						.AreaTitle(FText::FromString("Advanced"))
						.BodyContent()
						[
							SNew(SVerticalBox)

								// Seed Parameter: Random number generator initialization.
								+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString("Seed"))
												.ToolTipText(FText::FromString("Initialization of the random number generator. The seed determines the starting point of the random sequence, which affects the noise output. A fixed seed will result in the same heightmap for a given set of parameters. Changing the seed will generate a different heightmap."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<int32>)
												.Value_Lambda([this]() -> TOptional<int32> { return Heightmap->Seed; })
												.OnValueChanged_Lambda([this](int32 Value) { Heightmap->Seed = Value; })
										]
								]

							// Octaves Parameter: Number of noise layers (1-8).
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString("Octaves"))
												.ToolTipText(FText::FromString("The number of octaves used in the Perlin Noise generation. Octaves represent different layers of noise, each with varying frequencies and amplitudes. More octaves introduce more detail to the heightmap, as finer features are added with each additional octave."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<uint32>)
												.Value_Lambda([this]() -> TOptional<uint32> { return Heightmap->NumOctaves; })
												// Clamp between 1 and 8 octaves for performance and quality balance.
												.OnValueChanged_Lambda([this](uint32 Value) { Value = FMath::Clamp<uint32>(Value, 1, 8); Heightmap->NumOctaves = Value; })
										]
								]

							// Persistence Parameter: Controls amplitude decay (0.0 - 1.0).
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString("Persistence"))
												.ToolTipText(FText::FromString("Controls the amplitude of each successive octave. It determines how much influence each octave has on the final noise value. Lower persistence values result in smoother terrain because higher octaves (which add finer details) contribute less to the final noise. Higher persistence makes the finer details more pronounced."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([this]() -> TOptional<float> { return Heightmap->Persistence; })
												.OnValueChanged_Lambda([this](float Value) { Value = FMath::Clamp<float>(Value, 0.f, 1.f);  Heightmap->Persistence = Value; })
										]
								]

							// Lacunarity Parameter: Controls frequency multiplication.
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString("Lacunarity"))
												.ToolTipText(FText::FromString("Controls the frequency of successive octaves, essentially defining how zoomed in the noise features become in each additional layer. Higher lacunarity values increase the frequency, creating smaller, more frequent details in the heightmap. A lower value produces smoother and larger features."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([this]() -> TOptional<float> { return Heightmap->Lacunarity; })
												.OnValueChanged_Lambda([this](float Value) { Value = Value >= 0.f ? Value : 0.f; Heightmap->Lacunarity = Value; })
										]
								]

							// Initial Scale Parameter: Base noise feature size.
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString("Initial Scale"))
												.ToolTipText(FText::FromString("Starting scale for the Perlin Noise calculation and determines the size of features in the generated heightmap. A larger initial scale produces larger, more gradual features, while a smaller scale results in more frequent, smaller features. It sets the base level for how stretched or compressed the noise pattern is."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([this]() -> TOptional<float> { return Heightmap->InitialScale; })
												.OnValueChanged_Lambda([this](float Value) { Value = Value >= 0.f ? Value : 0.f; Heightmap->InitialScale = Value; })
										]
								]

							// Size Parameter: Heightmap resolution.
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString("Size"))
												.ToolTipText(FText::FromString("Represents the resolution of the heightmap, defining both width and height simultaneously. The maximum supported value is 505 pixels."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<uint32>)
												.Value_Lambda([this]() -> TOptional<uint32> { return Heightmap->Size; })
												// Clamp between 1px and 505px.
												.OnValueChanged_Lambda([this](uint32 Value) { Value = FMath::Clamp(Value, 1, 505); Heightmap->Size = Value; })
										]
								]

							// Max Height Difference Parameter: Elevation scaling factor.
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString("Max Height Difference"))
												.ToolTipText(FText::FromString("The maximum height difference in the generated terrain. The noise values typically range between [0, 1] and are then multiplied by MaxHeightDifference to scale them to the desired height range. It directly influences the elevation variation in the final heightmap."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([this]() -> TOptional<float> { return Heightmap->MaxHeightDifference; })
												.OnValueChanged_Lambda([this](float Value) { Value = Value >= 0.f ? Value : 0.f; Heightmap->MaxHeightDifference = Value; })
										]
								]
						]
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]

				// --- Create HeightMap Button (Procedural Generation) ---
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SButton)
								.Text(FText::FromString("Create HeightMap"))
								.OnClicked(this, &SHeightMapPanel::OnCreateHeightmapClicked)
						]
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]

				// --- External HeightMap Section ---
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString("External HeightMap"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]

				// --- Import External HeightMap Button ---
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SButton)
								.Text(FText::FromString("Import External Heightmap"))
								.OnClicked(this, &SHeightMapPanel::OnCreateFromExternalHeightmapClicked)
						]
				]

			// Visual spacing separators.
			+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SSeparator)
				]
		];
}

/**
 * Handles the "Create HeightMap" button click event.
 * Generates a procedural heightmap using Perlin noise based on current settings.
 * Resets external heightmap flags to ensure the system uses the procedurally
 * generated heightmap instead of an imported file.
 */
FReply SHeightMapPanel::OnCreateHeightmapClicked()
{
	if (!External.IsValid())
	{
		UDropByDropNotifications::ShowErrorNotification("Unable to create the heightmap!");
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"External\" resource is invalid!"));
		return FReply::Handled();
	}

	// Mark that we're using procedural generation, not external import.
	External->bIsExternalHeightMap = false;
	External->LastPNGPath.Empty();

	if (!Heightmap.IsValid())
	{
		UDropByDropNotifications::ShowErrorNotification("Unable to create the heightmap!");
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"Heightmap\" resource is invalid!"));
		return FReply::Handled();
	}

	// Generate and save the heightmap using current Perlin noise settings.
	if (!UPipelineLibrary::CreateAndSaveHeightMap(*Heightmap))
	{
		UDropByDropNotifications::ShowErrorNotification("Failed to create the heightmap!");
		return FReply::Handled();
	}

	UDropByDropNotifications::ShowSuccessNotification("Heightmap created successfully!");

	return FReply::Handled();
}

/**
 * Handles the "Import External Heightmap" button click event.
 * Opens a file dialog allowing the user to select an external heightmap
 * image file (typically PNG format) to import instead of using procedural
 * generation. This is useful for working with heightmaps created in external
 * tools.
 */
FReply SHeightMapPanel::OnCreateFromExternalHeightmapClicked()
{
	if (!External.IsValid())
	{
		UDropByDropNotifications::ShowErrorNotification("Unable to import the external heightmap!");
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"External\" resource is invalid!"));
		return FReply::Handled();
	}

	// Open native file dialog for heightmap selection.
	if (!UPipelineLibrary::OpenHeightmapFileDialog(External))
	{
		return FReply::Handled();
	}

	UDropByDropNotifications::ShowSuccessNotification("External heightmap imported successfully!");

	return FReply::Handled();
}