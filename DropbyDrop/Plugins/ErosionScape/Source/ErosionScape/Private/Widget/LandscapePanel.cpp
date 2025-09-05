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
	Heightmap = Args._Heightmap;
	External = Args._External;
	check(Heightmap.IsValid() && External.IsValid() && Landscape.IsValid());
	
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(STextBlock)
			.Text(FText::FromString("Landscape"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// Destroy Last Landscape
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged_Lambda([this](ECheckBoxState State)
			{
				Landscape->bDestroyLastLandscape = (State == ECheckBoxState::Checked);
			})
			[
				SNew(STextBlock).Text(FText::FromString("Destroy Last Landscape"))
			]
		]

		// Kilometers
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString("Kilometers"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([this]()-> TOptional<float> { return Landscape->Kilometers; })
				.OnValueChanged_Lambda([this](float Value)
				{
					Landscape->Kilometers = Value;
				})
			]
		]

		// Create / Destroy / Split
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SUniformGridPanel)
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton).Text(FText::FromString("Create Landscape")).OnClicked(
					this, &SLandscapePanel::OnCreateLandscapeClicked)
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton).Text(FText::FromString("Destroy Last")).OnClicked(
					this, &SLandscapePanel::OnDestroyLastClicked)
			]
			+ SUniformGridPanel::Slot(0, 1)
			[
				SNew(SButton).Text(FText::FromString("Split In Proxies")).OnClicked(
					this, &SLandscapePanel::OnSplitInProxiesClicked)
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SSeparator)
		]
	];
}

FReply SLandscapePanel::OnCreateLandscapeClicked()
{
	//TODO: Change LandscapeFromPNG args
	static const FString Dummy;
	UGeneratorHeightMapLibrary::GenerateLandscapeFromPNG(Dummy, *Heightmap, *External, *Landscape);
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
