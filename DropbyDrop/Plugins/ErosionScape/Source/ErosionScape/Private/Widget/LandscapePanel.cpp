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
		+ SVerticalBox::Slot().AutoHeight().Padding(6)
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
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([L=Landscape]()-> TOptional<int32> { return L->Kilometers; })
				.OnValueChanged_Lambda([L=Landscape](int32 V) { L->Kilometers = V; })
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
		[
			SNew(SSeparator)
		]

		// Toggle Advanced
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Advanced"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
			[
				SNew(SCheckBox)
				.IsChecked_Lambda([this]()
				{
					return bShowAdvanced ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				})
				.OnCheckStateChanged_Lambda(
					[this](ECheckBoxState S) { bShowAdvanced = (S == ECheckBoxState::Checked); })
			]
		]

		// --- Advanced ---
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SVerticalBox)
			.Visibility_Lambda([this]() { return GetAdvancedVisibility(); })

			// Scale X
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(FText::FromString("Scale X"))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
				[
					SNew(SNumericEntryBox<float>)
					.Value_Lambda([E=External]()-> TOptional<float> { return E->ScalingX; })
					.OnValueChanged_Lambda([E=External](float V) { E->ScalingX = V; })
				]
			]

			// Scale Y
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(FText::FromString("Scale Y"))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
				[
					SNew(SNumericEntryBox<float>)
					.Value_Lambda([E=External]()-> TOptional<float> { return E->ScalingY; })
					.OnValueChanged_Lambda([E=External](float V) { E->ScalingY = V; })
				]
			]

			// Scale Z
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(FText::FromString("Scale Z"))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
				[
					SNew(SNumericEntryBox<float>)
					.Value_Lambda([E=External]()-> TOptional<float> { return E->ScalingZ; })
					.OnValueChanged_Lambda([E=External](float V) { E->ScalingZ = V; })
				]
			]

			// World Partition Grid Size
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(STextBlock).Text(FText::FromString("World Partition Grid Size"))
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
				[
					SNew(SNumericEntryBox<int32>)
					.Value_Lambda([L=Landscape]()-> TOptional<int32> { return L->WorldPartitionGridSize; })
					.OnValueChanged_Lambda([L=Landscape](int32 V) { L->WorldPartitionGridSize = V; })
				]
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
		[
			SNew(SSeparator)
		]

		// Actions
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
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
	UGeneratorHeightMapLibrary::GenerateLandscapeAuto(*Heightmap, *External, *Landscape);
	return FReply::Handled();
}

FReply SLandscapePanel::OnDestroyLastClicked()
{
	if (Landscape->bDestroyLastLandscape && Landscape->TargetLandscape)
	{
		Landscape->TargetLandscape->Destroy();
	}
	return FReply::Handled();
}

FReply SLandscapePanel::OnSplitInProxiesClicked()
{
	UGeneratorHeightMapLibrary::SplitLandscapeIntoProxies(*Landscape);
	return FReply::Handled();
}

FReply SLandscapePanel::OnImportPNGClicked()
{
	UGeneratorHeightMapLibrary::OpenHeightmapFileDialog(External, Landscape, Heightmap);
	return FReply::Handled();
}
