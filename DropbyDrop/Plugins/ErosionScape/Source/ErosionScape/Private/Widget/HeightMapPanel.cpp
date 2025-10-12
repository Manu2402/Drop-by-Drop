// © Manuel Solano
// © Roberto Capparelli

#include "Widget/HeightMapPanel.h"

#include "GeneratorHeightMapLibrary.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "DropByDropLogger.h"

#define EMPTY_STRING ""
#define DEFAULT_PRESET_INDEX 1 // "Medium".

#pragma region Presets
void SHeightMapPanel::InitPresets()
{
	if (!Heightmap.IsValid())
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"Heightmap\" resource is invalid!"));
		return;
	}

	// Preset "Low" 
	PresetLight = *Heightmap;
	PresetLight.NumOctaves = 4;
	PresetLight.Persistence = 0.35f;
	PresetLight.Lacunarity = 1.8f;
	PresetLight.InitialScale = 1.2f;
	PresetLight.MaxHeightDifference = 0.25f;

	// Preset "Middle"
	PresetMedium = *Heightmap;
	PresetMedium.NumOctaves = 6;
	PresetMedium.Persistence = 0.40f;
	PresetMedium.Lacunarity = 2.0f;
	PresetMedium.InitialScale = 1.5f;
	PresetMedium.MaxHeightDifference = 0.8f;

	// Preset "High"
	PresetHeavy = *Heightmap;
	PresetHeavy.NumOctaves = 10;
	PresetHeavy.Persistence = 0.45f;
	PresetHeavy.Lacunarity = 1.9f;
	PresetHeavy.InitialScale = 2.4f;
	PresetHeavy.MaxHeightDifference = 1.0f;
}

void SHeightMapPanel::ApplyPreset(const FHeightMapGenerationSettings& Preset)
{
	if (!Heightmap.IsValid()) 
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"Heightmap\" resource is invalid!"));
		return;
	}

	Heightmap->Seed = Preset.Seed;
	Heightmap->NumOctaves = Preset.NumOctaves;
	Heightmap->Persistence = Preset.Persistence;
	Heightmap->Lacunarity = Preset.Lacunarity;
	Heightmap->InitialScale = Preset.InitialScale;
	Heightmap->Size = Preset.Size;
	Heightmap->MaxHeightDifference = Preset.MaxHeightDifference;
}

void SHeightMapPanel::BuildPresetItems()
{
	PresetItems.Empty();
	PresetItems.Add(MakeShared<FString>(TEXT("Low")));
	PresetItems.Add(MakeShared<FString>(TEXT("Medium")));
	PresetItems.Add(MakeShared<FString>(TEXT("High")));

	SelectedPreset = PresetItems[DEFAULT_PRESET_INDEX];
}

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
	
	ApplyPreset(PresetMedium);
}

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

TSharedRef<SWidget> SHeightMapPanel::MakePresetItemWidget(TSharedPtr<FString> Item) const
{
	return SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? *Item : TEXT(EMPTY_STRING)));
}

FReply SHeightMapPanel::OnPresetLightClicked()
{
	ApplyPreset(PresetLight);

	return FReply::Handled();
}

FReply SHeightMapPanel::OnPresetMediumClicked()
{
	ApplyPreset(PresetMedium);

	return FReply::Handled();
}

FReply SHeightMapPanel::OnPresetHeavyClicked()
{
	ApplyPreset(PresetHeavy);

	return FReply::Handled();
}
#pragma endregion

void SHeightMapPanel::Construct(const FArguments& Args)
{
	Heightmap = Args._Heightmap;
	External = Args._External;

	check(Heightmap.IsValid() && External.IsValid());

	InitPresets();
	BuildPresetItems();

	OnCreateHeightmapClicked();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(STextBlock)
				.Text(FText::FromString("HeightMap"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(5) // Presets row.
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
						SNew(STextBlock)
							.Text_Lambda([this](){ return FText::FromString(SelectedPreset.IsValid() ? *SelectedPreset : TEXT("Select Preset")); })
					]
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(5) // Randomize Seed.
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
					.IsChecked_Lambda([this](){ return Heightmap->bRandomizeSeed ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
					.OnCheckStateChanged_Lambda([this](ECheckBoxState State){ Heightmap->bRandomizeSeed = (State == ECheckBoxState::Checked); })
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
		[
			SNew(SSeparator)
		]
		// --- Advanced ---
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SExpandableArea)
				.InitiallyCollapsed(true)
				.AreaTitle(FText::FromString("Advanced"))
				.BodyContent()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(2)
					[
						SNew(SHorizontalBox) // Seed.
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
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Octaves.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Octaves"))
								.ToolTipText(FText::FromString("The number of octaves used in the Perlin Noise generation. Octaves represent different layers of noise, each with varying frequencies and ampli tudes. More octaves introduce more detail to the heightmap, as finer features are added with each additional octave."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<uint32>)
								.Value_Lambda([this]() -> TOptional<uint32> { return Heightmap->NumOctaves; })
								.OnValueChanged_Lambda([this](uint32 Value) { Value = FMath::Clamp<uint32>(Value, 1, 8); Heightmap->NumOctaves = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Persistence.
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
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Lacunarity.
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
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Initial Scale.
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
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Size.
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
								.OnValueChanged_Lambda([this](uint32 Value) { Value = FMath::Clamp(Value, 1, 505); Heightmap->Size = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Max Height Difference.
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
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) // "Create HeightMap" button.
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
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) // External HeightMap.
		[
			SNew(STextBlock)
				.Text(FText::FromString("External HeightMap"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 5) // "Import External HeightMap" button.
		[
			SNew(SHorizontalBox) 
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SButton)
					.Text(FText::FromString("Import External Heightmap"))
					.OnClicked(this, &SHeightMapPanel::OnCreateFromExternalHeightmapClicked)
			]
		]
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

FReply SHeightMapPanel::OnCreateHeightmapClicked()
{
	if (External.IsValid())
	{
		External->bIsExternalHeightMap = false;
		External->LastPNGPath.Empty();
	}

	if (!Heightmap.IsValid())
	{
		UE_LOG(LogDropByDropHeightmap, Error, TEXT("The \"Heightmap\" resource is invalid!"));
		return FReply::Handled();
	}

	UGeneratorHeightMapLibrary::CreateAndSaveHeightMap(*Heightmap);

	return FReply::Handled();
}

FReply SHeightMapPanel::OnCreateFromExternalHeightmapClicked()
{
	UGeneratorHeightMapLibrary::OpenHeightmapFileDialog(External);

	return FReply::Handled();
}
