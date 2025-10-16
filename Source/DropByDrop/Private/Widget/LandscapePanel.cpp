// © Manuel Solano
// © Roberto Capparelli

#include "Widget/LandscapePanel.h"

#include "Components/LandscapeInfoComponent.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Libraries/PipelineLibrary.h"
#include "DropByDropNotifications.h"
#include "Landscape.h"

/**
 * Constructs the SLandscapePanel widget hierarchy.
 *
 * This method builds the complete UI layout including:
 * - A title label ("Landscape").
 * - Kilometers input for landscape scale (for internal heightmaps).
 * - An expandable "Advanced" section containing:
 *   - Scale X, Y, Z inputs (for external heightmaps).
 *   - World Partition Cell Size input.
 * - Action buttons ("Create Landscape" and "Split in Proxies").
 *
 * The function extracts the construction arguments and stores references to the
 * landscape settings and active landscape for later use in event handlers.
 */
void SLandscapePanel::Construct(const FArguments& Args)
{
	// Extract and store the construction arguments as member variables.
	// These shared pointers contain the settings that will be modified by the UI controls.
	Landscape = Args._Landscape;
	External = Args._External;
	Heightmap = Args._Heightmap;
	ActiveLandscape = Args._ActiveLandscape;

	// Validate that all required settings pointers are valid before proceeding.
	check(Landscape.IsValid() && External.IsValid() && Heightmap.IsValid());

	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Landscape"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]

				// Kilometers input section.
				// This control sets the overall landscape size for internally generated heightmaps.
				+SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Kilometers"))
								.ToolTipText(FText::FromString("Controls the overall size of the landscape in terms of real-world scale. The value entered represents the width and length of the landscape in kilometers. This unit of measurement is only valid for landscapes generated using heightmaps generated internally in Drop by Drop."))
						]
						// Numeric input box for kilometers value.
						// Binds to Landscape->Kilometers for reading and writing.
						+SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<uint32>)
								// Lambda to retrieve current kilometers value from settings.
								.Value_Lambda([L = Landscape]() -> TOptional<uint32> { return L->Kilometers; })
								// Lambda to update kilometers value when user changes it.
								.OnValueChanged_Lambda([L = Landscape](uint32 Value) { Value = (Value < 1) ? 1 : Value;  L->Kilometers = Value; })
						]
				]
			+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]
				// Advanced settings section (collapsible).
				// Contains controls specific to external heightmap import and world partition.
				+SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SExpandableArea)
						.InitiallyCollapsed(true)
						.AreaTitle(FText::FromString("Advanced"))
						.BodyContent()
						[
							SNew(SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SVerticalBox)
										// Scale X input section.
										// Controls X-axis dimension for external heightmaps.
										+SVerticalBox::Slot().AutoHeight().Padding(2)
										[
											SNew(SHorizontalBox)
												+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
												[
													SNew(STextBlock)
														.Text(FText::FromString("Scale X"))
														.ToolTipText(FText::FromString("Dimension on the X-axis of the landscape. This unit of measurement is only valid for landscapes generated using external heightmaps, i.e., imported from the file system."))
												]
												// Numeric input for Scale X value.
												// Ensures non-negative values through clamping.
												+SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
												[
													SNew(SNumericEntryBox<float>)
														// Lambda to retrieve current ScalingX value.
														.Value_Lambda([E = External]() -> TOptional<float> { return E->ScalingX; })
														// Lambda to update ScalingX, clamping to minimum of 0.
														.OnValueChanged_Lambda([E = External](float Value) { Value = Value >= 0.f ? Value : 0.f; E->ScalingX = Value; })
												]
										]
									// Scale Y input section.
									// Controls Y-axis dimension for external heightmaps.
									+SVerticalBox::Slot().AutoHeight().Padding(2)
										[
											SNew(SHorizontalBox)

												// Label for Scale Y.
												+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
												[
													SNew(STextBlock)
														.Text(FText::FromString("Scale Y"))
														.ToolTipText(FText::FromString("Dimension on the Y-axis of the landscape. This unit of measurement is only valid for landscapes generated using external heightmaps, i.e., imported from the file system."))
												]
												// Numeric input for Scale Y value.
												// Ensures non-negative values through clamping.
												+SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
												[
													SNew(SNumericEntryBox<float>)
														// Lambda to retrieve current ScalingY value.
														.Value_Lambda([E = External]() -> TOptional<float> { return E->ScalingY; })
														// Lambda to update ScalingY, clamping to minimum of 0.
														.OnValueChanged_Lambda([E = External](float Value) { Value = Value >= 0.f ? Value : 0.f; E->ScalingY = Value; })
												]
										]
									// Scale Z input section.
									// Controls Z-axis (height) dimension for external heightmaps.
									+SVerticalBox::Slot().AutoHeight().Padding(2)
										[
											SNew(SHorizontalBox)
												// Label for Scale Z.
												+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
												[
													SNew(STextBlock)
														.Text(FText::FromString("Scale Z"))
														.ToolTipText(FText::FromString("Dimension on the Z-axis of the landscape. This unit of measurement is only valid for landscapes generated using external heightmaps, i.e., imported from the file system."))
												]
												// Numeric input for Scale Z value.
												// Ensures non-negative values through clamping.
												+SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
												[
													SNew(SNumericEntryBox<float>)
														// Lambda to retrieve current ScalingZ value.
														.Value_Lambda([E = External]() -> TOptional<float> { return E->ScalingZ; })
														// Lambda to update ScalingZ, clamping to minimum of 0.
														.OnValueChanged_Lambda([E = External](float Value) { Value = Value >= 0.f ? Value : 0.f; E->ScalingZ = Value; })
												]
										]
									// World Partition Cell Size input section.
									// Defines the size of landscape proxies for streaming.
									+SVerticalBox::Slot().AutoHeight().Padding(2)
										[
											SNew(SHorizontalBox)
												// Label for World Partition Cell Size.
												+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
												[
													SNew(STextBlock)
														.Text(FText::FromString("World Partition Cell Size"))
														.ToolTipText(FText::FromString("Defines the size of the landscape proxy. The values represent the grid size, in quad, of the proxy. A higher value creates a bigger proxy while a lower value divides into more proxies, but increases the process time."))
												]
												// Numeric input for cell size.
												// Clamped between 1 and the landscape's total kilometers.
												+SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
												[
													SNew(SNumericEntryBox<uint32>)
														// Lambda to retrieve current "WorldPartitionCellSize" value.
														.Value_Lambda([L = Landscape]() -> TOptional<uint32> { return L->WorldPartitionCellSize; })
														// Lambda to update cell size, clamping between 1 and Kilometers.
														.OnValueChanged_Lambda([L = Landscape](uint32 Value) { Value = FMath::Clamp(Value, 1, L->Kilometers);  L->WorldPartitionCellSize = Value; })
												]
										]
								]
						]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]
				// Action buttons section.
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)

						// "Create Landscape" button.
						// Generates a new landscape using the configured settings.
						+SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SButton)
								.Text(FText::FromString("Create Landscape"))
								.OnClicked(this, &SLandscapePanel::OnCreateLandscapeClicked)
						]
						// "Split in Proxies" button
						// Only enabled when a valid landscape is selected and hasn't been split yet.
						+SHorizontalBox::Slot().AutoWidth().Padding(10, 0)
						[
							SNew(SButton)
								// Lambda to determine if button should be enabled.
								// Checks if ActiveLandscape exists, is valid, and hasn't been split.
								.IsEnabled_Lambda([L = ActiveLandscape]()
									{
										// Verify ActiveLandscape pointer is valid and points to a valid actor.
										if (L && IsValid(*L))
										{
											// Get the LandscapeInfoComponent to check split status.
											ULandscapeInfoComponent* Info = (*L)->FindComponentByClass<ULandscapeInfoComponent>();
											// Enable only if component is valid and landscape hasn't been split yet.
											return IsValid(Info) && !Info->GetIsSplittedIntoProxies();
										}

										// Disable button if no valid landscape is selected.
										return false;
									})
								.Text(FText::FromString("Split in Proxies"))
								.OnClicked(this, &SLandscapePanel::OnSplitInProxiesClicked)
						]
				]

			// Additional visual separators for spacing at bottom.
			+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]
		];
}

/**
 * Event handler for the "Create Landscape" button.
 *
 * This method is called when the user clicks the "Create Landscape" button.
 * It delegates the actual landscape generation to "UPipelineLibrary::GenerateLandscapeAuto",
 * passing the current heightmap, external heightmap, and landscape settings.
 *
 * The generation process will create a new landscape in the level based on:
 * - The heightmap data (either internal or external).
 * - The external scaling settings (Scale X, Y, Z).
 * - The landscape generation settings (Kilometers, World Partition Cell Size).
 */
FReply SLandscapePanel::OnCreateLandscapeClicked()
{
	// Call the pipeline library function to generate the landscape.
	// Dereferences the shared pointers to pass the actual settings structs.
	if (!UPipelineLibrary::GenerateLandscapeAuto(*Heightmap, *External, *Landscape))
	{
		UDropByDropNotifications::ShowErrorNotification(TEXT("Failed to create the landscape!"));
		return FReply::Handled();
	}

	UDropByDropNotifications::ShowSuccessNotification(TEXT("Landscape created successfully!"));

	// Return Handled to indicate the UI event was processed.
	return FReply::Handled();
}

/**
 * Event handler for the "Split in Proxies" button.
 *
 * This method is called when the user clicks the "Split in Proxies" button.
 * It splits the currently active landscape into multiple proxy actors for world partition
 * streaming. This improves performance by allowing the engine to load/unload landscape
 * sections based on the player's location.
 *
 * Note: Pointer safety checks are not needed here because the button is disabled
 * (via the IsEnabled_Lambda in Construct) when no valid landscape is selected or
 * when the landscape has already been split.
 */
FReply SLandscapePanel::OnSplitInProxiesClicked()
{
	// Call the pipeline library function to split the landscape into proxies.
	// Double dereference: first to get ALandscape* from TObjectPtr<ALandscape>*,
	// then to get ALandscape& for the function parameter.
	if (!UPipelineLibrary::SplitLandscapeIntoProxies(**ActiveLandscape))
	{
		UDropByDropNotifications::ShowErrorNotification(TEXT("Failed to split the landscape into proxies!"));
		return FReply::Handled();
	}

	UDropByDropNotifications::ShowSuccessNotification(TEXT("Landscape successfully split into proxies!"));

	// Return Handled to indicate the UI event was processed.
	return FReply::Handled();
}