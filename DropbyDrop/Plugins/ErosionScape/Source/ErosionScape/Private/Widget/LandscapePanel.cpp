// © Manuel Solano
// © Roberto Capparelli

#include "Widget/LandscapePanel.h"

#include "Widgets/Input/SNumericEntryBox.h"
#include "GeneratorHeightMapLibrary.h"
#include "DropByDropLandscape.h"
#include "Landscape.h"

void SLandscapePanel::Construct(const FArguments& Args)
{
	Landscape = Args._Landscape;
	External = Args._External;
	Heightmap = Args._Heightmap;
	ActiveLandscape = Args._ActiveLandscape;

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
		+ SVerticalBox::Slot().AutoHeight().Padding(5) // Kilometers.
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::FromString("Kilometers"))
					.ToolTipText(FText::FromString("Controls the overall size of the landscape in terms of real-world scale. The value entered represents the width and length of the landscape in kilometers."))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
			[
				SNew(SNumericEntryBox<uint32>)
					.Value_Lambda([L = Landscape]() -> TOptional<uint32> { return L->Kilometers; })
					.OnValueChanged_Lambda([L = Landscape](uint32 Value) { L->Kilometers = Value; })
			]
		]
	+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
		[
			SNew(SSeparator)
		]
		// --- Advanced ---
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
						+ SVerticalBox::Slot().AutoHeight().Padding(2) // Scale X.
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(FText::FromString("Scale X"))
									.ToolTipText(FText::FromString("Dimension on the X-axis of the landscape generated after selecting a heightmap from the file system."))
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
							[
								SNew(SNumericEntryBox<float>)
									.Value_Lambda([E = External]() -> TOptional<float> { return E->ScalingX; })
									.OnValueChanged_Lambda([E = External](float Value) { Value = Value >= 0.f ? Value : 0.f; E->ScalingX = Value; })
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(2) // Scale Y.
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(FText::FromString("Scale Y"))
									.ToolTipText(FText::FromString("Dimension on the Y-axis of the landscape generated after selecting a heightmap from the file system."))
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
							[
								SNew(SNumericEntryBox<float>)
									.Value_Lambda([E = External]() -> TOptional<float> { return E->ScalingY; })
									.OnValueChanged_Lambda([E = External](float Value) { Value = Value >= 0.f ? Value : 0.f; E->ScalingY = Value; })
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(2) // Scale Z.
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(FText::FromString("Scale Z"))
									.ToolTipText(FText::FromString("Dimension on the Z-axis of the landscape generated after selecting a heightmap from the file system."))
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
							[
								SNew(SNumericEntryBox<float>)
									.Value_Lambda([E = External]() -> TOptional<float> { return E->ScalingZ; })
									.OnValueChanged_Lambda([E = External](float Value) { Value = Value >= 0.f ? Value : 0.f; E->ScalingZ = Value; })
							]
						]
						+ SVerticalBox::Slot().AutoHeight().Padding(2) // World Partition Cell Size.
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
							[
								SNew(STextBlock)
									.Text(FText::FromString("World Partition Cell Size"))
									.ToolTipText(FText::FromString("Defines the size of the landscape proxy. The values represent the grid size, in quad, of the proxy. A higher value creates a bigger proxy while a lower value divides into more proxies, but increases the process time."))
							]
							+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
							[
								SNew(SNumericEntryBox<uint32>)
									.Value_Lambda([L = Landscape]() -> TOptional<uint32> { return L->WorldPartitionCellSize; })
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
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text(FText::FromString("Create Landscape")) // "Create Landscape" button.
					.OnClicked(this, &SLandscapePanel::OnCreateLandscapeClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10, 0)
			[
				SNew(SButton)
					.IsEnabled_Lambda([L = ActiveLandscape]() 
					{
						if (L && IsValid(*L))
						{
							ULandscapeInfoComponent* Info = (*L)->FindComponentByClass<ULandscapeInfoComponent>();
							return IsValid(Info) && !Info->bIsSplittedIntoProxies;
						}

						return false;
					})
					.Text(FText::FromString("Split in Proxies")) // "Split in Proxies" button.
					.OnClicked(this, &SLandscapePanel::OnSplitInProxiesClicked)
			]
		]
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

FReply SLandscapePanel::OnCreateLandscapeClicked()
{
	UGeneratorHeightMapLibrary::GenerateLandscapeAuto(*Heightmap, *External, *Landscape);

	return FReply::Handled();
}

FReply SLandscapePanel::OnSplitInProxiesClicked()
{
	// No pointer safety needed, this button is disabled if no landscape selected.
	UGeneratorHeightMapLibrary::SplitLandscapeIntoProxies(**ActiveLandscape);

	return FReply::Handled();
}
