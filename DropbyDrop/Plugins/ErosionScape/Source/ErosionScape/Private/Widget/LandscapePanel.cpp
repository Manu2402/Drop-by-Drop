#include "Widget/LandscapePanel.h"
#include "GeneratorHeightMapLibrary.h"
#include "Landscape.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SSeparator.h"

void SLandscapePanel::Construct(const FArguments& Args)
{
	Landscape = Args._Landscape;
	External = Args._External;
	Heightmap = Args._Heightmap;
	check(Landscape.IsValid() && External.IsValid() && Heightmap.IsValid());

	ChildSlot
	[
		SNew(SVerticalBox)

		// Title
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Landscape"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// Destroy Last Landscape
		+SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(FText::FromString("Destroy Last Landscape"))
					.ToolTipText(FText::FromString("Whether the last generated landscape will be destroyed when a new landscape is generated."))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
				[
					SNew(SCheckBox)
						.IsChecked_Lambda([L = Landscape]()
							{
								return L->bDestroyLastLandscape ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
							})
						.OnCheckStateChanged_Lambda([L = Landscape](ECheckBoxState S)
							{
								L->bDestroyLastLandscape = (S == ECheckBoxState::Checked);
							})
				]
		]

		// Kilometers 
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Kilometers"))
				.ToolTipText(FText::FromString("Controls the overall size of the landscape in terms of real-world scale. The value entered represents the width and length of the landscape in kilometers."))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
			[
				SNew(SNumericEntryBox<uint32>)
				.Value_Lambda([L=Landscape]()-> TOptional<uint32> { return L->Kilometers; })
				.OnValueChanged_Lambda([L=Landscape](uint32 V) { L->Kilometers = V; })
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
								SNew(SVerticalBox)
									// Scale X
									+ SVerticalBox::Slot().AutoHeight().Padding(2)
									[
										SNew(SHorizontalBox)
											+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
											[
												SNew(STextBlock).Text(FText::FromString("Scale X"))
												.ToolTipText(FText::FromString("Dimension on the X-axis of the landscape generated after selecting a heightmap from the file system."))
											]
											+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
											[
												SNew(SNumericEntryBox<float>)
													.Value_Lambda([E = External]()-> TOptional<float> { return E->ScalingX; })
													.OnValueChanged_Lambda([E = External](float V) { V = V >= 0.f ? V : 0.f; E->ScalingX = V; })
											]
									]

								// Scale Y
								+ SVerticalBox::Slot().AutoHeight().Padding(2)
									[
										SNew(SHorizontalBox)
											+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
											[
												SNew(STextBlock).Text(FText::FromString("Scale Y"))
												.ToolTipText(FText::FromString("Dimension on the Y-axis of the landscape generated after selecting a heightmap from the file system."))
											]
											+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
											[
												SNew(SNumericEntryBox<float>)
													.Value_Lambda([E = External]()-> TOptional<float> { return E->ScalingY; })
													.OnValueChanged_Lambda([E = External](float V) { V = V >= 0.f ? V : 0.f; E->ScalingY = V; })
											]
									]

								// Scale Z
								+ SVerticalBox::Slot().AutoHeight().Padding(2)
									[
										SNew(SHorizontalBox)
											+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
											[
												SNew(STextBlock).Text(FText::FromString("Scale Z"))
												.ToolTipText(FText::FromString("Dimension on the Z-axis of the landscape generated after selecting a heightmap from the file system."))
											]
											+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
											[
												SNew(SNumericEntryBox<float>)
													.Value_Lambda([E = External]()-> TOptional<float> { return E->ScalingZ; })
													.OnValueChanged_Lambda([E = External](float V) { V = V >= 0.f ? V : 0.f; E->ScalingZ = V; })
											]
									]

								// World Partition Cell Size
								+ SVerticalBox::Slot().AutoHeight().Padding(2)
									[
										SNew(SHorizontalBox)
											+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
											[
												SNew(STextBlock).Text(FText::FromString("World Partition Cell Size"))
												.ToolTipText(FText::FromString("Defines the size of the landscape proxy. The values represent the grid size, in quad, of the proxy. A higher value creates a bigger proxy while a lower value divides into more proxies, but increases the process time."))
											]
											+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
											[
												SNew(SNumericEntryBox<uint32>)
													.Value_Lambda([L = Landscape]()-> TOptional<uint32> { return L->WorldPartitionCellSize; })
													.OnValueChanged_Lambda([L = Landscape](uint32 V) { V = FMath::Clamp(V, 1, L->Kilometers);  L->WorldPartitionCellSize = V; })
											]
									]
							]
					]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
		[
			SNew(SSeparator)
		]

		// Actions
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
				.Text(FText::FromString("Create Landscape"))
				.OnClicked(this, &SLandscapePanel::OnCreateLandscapeClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(10, 0)
			[
				SNew(SButton)
				.IsEnabled_Lambda([L = Landscape]() { return L && IsValid(L->TargetLandscape) && !L->bIsSplittedIntoProxies; })
				.Text(FText::FromString("Split in Proxies"))
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
	Landscape->bIsSplittedIntoProxies = false;

	UGeneratorHeightMapLibrary::GenerateLandscapeAuto(*Heightmap, *External, *Landscape);
	return FReply::Handled();
}

FReply SLandscapePanel::OnDestroyLastClicked()
{
	if (Landscape->bDestroyLastLandscape && IsValid(Landscape->TargetLandscape))
	{
		Landscape->TargetLandscape->Destroy();
	}
	return FReply::Handled();
}

FReply SLandscapePanel::OnSplitInProxiesClicked()
{
	Landscape->bIsSplittedIntoProxies = true;

	UGeneratorHeightMapLibrary::SplitLandscapeIntoProxies(*Landscape);
	return FReply::Handled();
}

FReply SLandscapePanel::OnImportPNGClicked()
{
	UGeneratorHeightMapLibrary::OpenHeightmapFileDialog(External, Landscape, Heightmap);
	return FReply::Handled();
}
