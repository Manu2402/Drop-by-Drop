#include "Widget/ErosionPanel.h"
#include "Widget/TemplateBrowser.h"
#include "ErosionLibrary.h"
#include "GeneratorHeightMapLibrary.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SSeparator.h"


void SErosionPanel::Construct(const FArguments& Args)
{
	Heightmap = Args._Heightmap;
	External = Args._External;
	Landscape = Args._Landscape;
	TemplateManager = Args._TemplateManager;
	check(Heightmap.IsValid() && External.IsValid() && Landscape.IsValid());

	BuildWindDirections();

	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(STextBlock).Text(FText::FromString("Erosion")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// Erosion cycles + Wind Direction
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Erosion Cycles"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([]()-> TOptional<int32> { return UErosionLibrary::GetErosionCycles(); })
				.OnValueChanged_Lambda([](int32 Value) { UErosionLibrary::SetErosion(Value); })
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(40)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(STextBlock).Text(FText::FromString("Wind Direction"))
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SComboBox<TSharedPtr<FString>>)
					.OptionsSource(&WindDirections)
					.OnGenerateWidget_Lambda([](TSharedPtr<FString> Options)
					{
						return SNew(STextBlock).Text(FText::FromString(*Options));
					})
					.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Options, ESelectInfo::Type)
					{
						if (!Options.IsValid() || !WindDirectionEnumPtr) return;
						const int32 WindValue = WindDirectionEnumPtr->GetValueByNameString(*Options);
						UErosionLibrary::SetWindDirection(static_cast<EWindDirection>(WindValue));
						CurrentWindDirection = Options;
					})
					[
						SNew(STextBlock).Text_Lambda([this]()
						{
							return CurrentWindDirection.IsValid()
								       ? FText::FromString(*CurrentWindDirection)
								       : FText::FromString("Select\nWind Direction");
						})
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SCheckBox)
					.OnCheckStateChanged_Lambda([](ECheckBoxState S)
					{
						UErosionLibrary::SetWindBias(static_cast<bool>(S));
					})
					[
						SNew(STextBlock).Text(FText::FromString("Wind Bias"))
					]
				]
			]
		]

		// Inertia
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Inertia"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([]()-> TOptional<float> { return UErosionLibrary::GetInertia(); })
				.OnValueChanged_Lambda([](float V) { UErosionLibrary::SetInertia(V); })
			]
		]

		// Capacity
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Capacity"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([]()-> TOptional<int32> { return UErosionLibrary::GetCapacity(); })
				.OnValueChanged_Lambda([](int32 V) { UErosionLibrary::SetCapacity(V); })
			]
		]

		// Minimal Slope
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Minimal Slope"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([]()-> TOptional<float> { return UErosionLibrary::GetMinimalSlope(); })
				.OnValueChanged_Lambda([](float V) { UErosionLibrary::SetMinimalSlope(V); })
			]
		]

		// Deposition Speed
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Deposition Speed"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([]()-> TOptional<float> { return UErosionLibrary::GetDepositionSpeed(); })
				.OnValueChanged_Lambda([](float V) { UErosionLibrary::SetDepositionSpeed(V); })
			]
		]

		// Erosion Speed
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Erosion Speed"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([]()-> TOptional<float> { return UErosionLibrary::GetErosionSpeed(); })
				.OnValueChanged_Lambda([](float V) { UErosionLibrary::SetErosionSpeed(V); })
			]
		]

		// Gravity
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Gravity"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([]()-> TOptional<int32> { return UErosionLibrary::GetGravity(); })
				.OnValueChanged_Lambda([](int32 V) { UErosionLibrary::SetGravity(V); })
			]
		]

		// Evaporation
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Evaporation"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([]()-> TOptional<float> { return UErosionLibrary::GetEvaporation(); })
				.OnValueChanged_Lambda([](float V) { UErosionLibrary::SetEvaporation(V); })
			]
		]

		// Max Path
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Max Path"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([]()-> TOptional<int32> { return UErosionLibrary::GetMaxPath(); })
				.OnValueChanged_Lambda([](int32 V) { UErosionLibrary::SetMaxPath(V); })
			]
		]

		// Erosion Radius
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(FText::FromString("Erosion Radius"))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([]()-> TOptional<int32> { return UErosionLibrary::GetErosionRadius(); })
				.OnValueChanged_Lambda([](int32 V) { UErosionLibrary::SetErosionRadius(V); })
			]
		]

		// Run Erosion
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SButton)
			.Text(FText::FromString("Erosion"))
			.OnClicked(this, &SErosionPanel::OnErodeClicked)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SSeparator)
		]

		// Template Browser (Save/Load/Delete)
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(5)
		[
			SNew(STemplateBrowser).TemplateManager(TemplateManager)
		]
	];
}

void SErosionPanel::BuildWindDirections()
{
	WindDirectionEnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EWindDirection"));
	if (!WindDirectionEnumPtr) return;
	for (int32 i = 0; i < WindDirectionEnumPtr->NumEnums() - 1; ++i)
	{
		WindDirections.Add(MakeShared<FString>(WindDirectionEnumPtr->GetNameStringByIndex(i)));
	}
	if (WindDirections.Num() > 0) CurrentWindDirection = WindDirections[0];
}

FReply SErosionPanel::OnErodeClicked()
{
	UGeneratorHeightMapLibrary::GenerateErosion(*External, *Landscape, *Heightmap, Heightmap->Size);
	return FReply::Handled();
}
