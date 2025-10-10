// © Manuel Solano
// © Roberto Capparelli

#include "Widget/ErosionPanel.h"

#include "Widgets/Input/SNumericEntryBox.h"
#include "GeneratorHeightMapLibrary.h"
#include "Widget/TemplateBrowser.h"
#include "DropByDropLandscape.h"
#include "Landscape.h"

#define EMPTY_STRING ""
#define DEFAULT_WIND_DIRECTION EWindDirection::Random

void SErosionPanel::Construct(const FArguments& Args)
{
	Landscape = Args._Landscape;
	Erosion = Args._Erosion;
	ActiveLandscape = Args._ActiveLandscape;
	TemplateManager = Args._TemplateManager;

	check(Landscape.IsValid() && Erosion.IsValid());

	BuildWindDirections();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(STextBlock)
				.Text(FText::FromString("Erosion"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(STextBlock)
					.Text(FText::FromString("Erosion Cycles"))
					.ToolTipText(FText::FromString("Number of computed erosion cycles, equivalent to the num ber of drops falling from the sky that will follow their course. The more drops that come in, the more the erosion impact on the ground will be accentuated."))
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
			[
				SNew(SNumericEntryBox<int64>)
					.Value_Lambda([E = Erosion]() -> TOptional<int64> { return E->ErosionCycles; })
					.OnValueChanged_Lambda([E = Erosion](int64 Value) { Value = Value >= 0 ? Value : 0; E->ErosionCycles = Value; })
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
		[
			SNew(SSeparator)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(5)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[
				SNew(STextBlock)
					.Text(FText::FromString("Wind Direction"))
					.ToolTipText(FText::FromString("Direction of wind force application on each individual drop."))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
			[

				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth() // Wind Direction.
				[
					SAssignNew(WindCombo, SComboBox<TSharedPtr<FString>>)
						.OptionsSource(&WindDirections)
						.InitiallySelectedItem(CurrentWindDirection)
						.MaxListHeight(300.f)
						.OnGenerateWidget_Lambda([](TSharedPtr<FString> Option) -> TSharedRef<SWidget> { return SNew(STextBlock).Text(FText::FromString(Option.IsValid() ? *Option : TEXT(EMPTY_STRING)));})
						.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Option, ESelectInfo::Type)
						{
							if (!Option.IsValid())
							{
								return;
							}

							CurrentWindDirection = Option;

							const FString& Name = *Option;
							EWindDirection Dir = DEFAULT_WIND_DIRECTION;

							if (Name.Equals(TEXT("Nord")))
							{
								Dir = EWindDirection::Nord;
								goto set_wind;
							}

							if (Name.Equals(TEXT("Sud")))
							{
								Dir = EWindDirection::Sud;
								goto set_wind;
							}

							if (Name.Equals(TEXT("Est")))
							{
								Dir = EWindDirection::Est;
								goto set_wind;
							}

							if (Name.Equals(TEXT("Ovest")))
							{
								Dir = EWindDirection::Ovest;
								goto set_wind;
							}

							if (Name.Equals(TEXT("Nord_Ovest")))
							{
								Dir = EWindDirection::Nord_Ovest;
								goto set_wind;
							}

							if (Name.Equals(TEXT("Nord_Est")))
							{
								Dir = EWindDirection::Nord_Est;
								goto set_wind;
							}

							if (Name.Equals(TEXT("Sud_Ovest")))
							{
								Dir = EWindDirection::Sud_Ovest;
								goto set_wind;
							}

							if (Name.Equals(TEXT("Sud_Est")))
							{
								Dir = EWindDirection::Sud_Est;
								goto set_wind;
							}

set_wind:
							Erosion->WindDirection = Dir;
						})
						[
							SNew(STextBlock).Text_Lambda([this]()
							{
								return CurrentWindDirection.IsValid() ? FText::FromString(*CurrentWindDirection) : FText::FromString("Select\nWind Direction");
							})
						]
				]
				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0).VAlign(VAlign_Center) // Wind Bias.
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
					[
						SNew(STextBlock)
							.Text(FText::FromString("Wind Bias"))
							.ToolTipText(FText::FromString("Bias applied to the selected wind direction (no random). This is used to make the application of the wind direction less pronounced, but still predominant, in order to make the final effect more realistic."))
					]
					+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
					[
						SNew(SCheckBox)
							.OnCheckStateChanged_Lambda([E = Erosion](ECheckBoxState S){ E->bWindBias = (S == ECheckBoxState::Checked); })
					]
				]
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 5)
			[

				SNew(SButton)
					.IsEnabled_Lambda([L = ActiveLandscape, E = Erosion]() { return L && IsValid(*L) && E->WindDirection != EWindDirection::Random; })
					.Text(FText::FromString("Wind Preview"))
					.OnClicked_Lambda([this]()
					{
						if (!Landscape.IsValid())
						{
							return FReply::Handled();
						}

						UGeneratorHeightMapLibrary::DrawWindDirectionPreview(*Erosion, *ActiveLandscape, 8000.f, 12.f, 300.f, 6.f, true, 15.f);

						return FReply::Handled();
					})
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
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Inertia.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Inertia"))
								.ToolTipText(FText::FromString("Parameter indicating the inertia value of the drop. The closer the parameter is to 0 the more the drop will tend to follow a pit as it falls, thus creating more pronounced ruts, while the closer the value is to 1 the more the drop will tend to follow the course of its original path, thus creating a more unnatural effect."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([E = Erosion]() -> TOptional<float> { return E->Inertia; })
								.OnValueChanged_Lambda([E = Erosion](float Value) { Value = FMath::Clamp(Value, 0.f, 1.f); E->Inertia = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Capacity.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Capacity"))
								.ToolTipText(FText::FromString("Amount of sediment that is capable of transporting the droplet during its path. The greater this value is, the greater is both the erosion effect applied on the land and the deposition effect in flat areas."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<int32>)
								.Value_Lambda([E = Erosion]() -> TOptional<int32> { return E->Capacity; })
								.OnValueChanged_Lambda([E = Erosion](int32 Value) { Value = Value >= 0 ? Value : 0; E->Capacity = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Minimal Slope.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Minimal Slope"))
								.ToolTipText(FText::FromString("Minimal slope taken into account when calculating the carrying capacity. The smaller this slope is, the more realistic the final terrain will be, due to the fact that the erosion effect is accounted for even in the finer movements of the drop. In contrast, the greater the value, the more intense the computed erosion will be, to the point of creating ruins in the most extreme cases."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([E = Erosion]() -> TOptional<float> { return E->MinimalSlope; })
								.OnValueChanged_Lambda([E = Erosion](float Value) { Value = Value >= 0.f ? Value : 0.f; E->MinimalSlope = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Deposition Speed.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Deposition Speed"))
								.ToolTipText(FText::FromString("Deposition factor of the sediment transported by the droplet. The more this parameter tends to 1, the more sediment will be deposited on the occasions involved, while the closer it is to 0, the less this sediment will be deposited."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([E = Erosion]() -> TOptional<float> { return E->DepositionSpeed; })
								.OnValueChanged_Lambda([E = Erosion](float Value) { Value = FMath::Clamp(Value, 0.f, 1.f); E->DepositionSpeed = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Erosion Speed.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Erosion Speed"))
								.ToolTipText(FText::FromString("Erosion factor of the drop. The more this parameter tends to 1, the more sediment will be eroded during the lifetime of the drop, while the closer it is to 0, the less this sediment will be eroded from the soil."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([E = Erosion]() -> TOptional<float> { return E->ErosionSpeed; })
								.OnValueChanged_Lambda([E = Erosion](float Value) { Value = FMath::Clamp(Value, 0.f, 1.f); E->ErosionSpeed = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Gravity.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Gravity"))
								.ToolTipText(FText::FromString("Represents the force of gravity acting on the droplet. The greater this value, the greater the force of gravity applied on it, thus causing it to accelerate more or less downward. This will result in greater computation of the drop in flat areas."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<int32>)
								.Value_Lambda([E = Erosion]() -> TOptional<int32> { return E->Gravity; })
								.OnValueChanged_Lambda([E = Erosion](int32 Value) { Value = Value >= 0 ? Value : 0; E->Gravity = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Evaporation.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Evaporation"))
								.ToolTipText(FText::FromString("Evaporation index of the droplet. The more this value tends to 1, the more the drop will be subject to evaporation, and thus it will become smaller and smaller in size during its iterations. This will cause less sediment collection and consequently also on erosion and deposition."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([E = Erosion]()-> TOptional<float> { return E->Evaporation; })
								.OnValueChanged_Lambda([E = Erosion](float Value) { Value = FMath::Clamp(Value, 0.f, 1.f); E->Evaporation = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Max Path.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Max Path"))
								.ToolTipText(FText::FromString("Represents the number of steps taken by the droplet before it dis appears. The higher this value, the greater the impact these drops will tend to have on the entire terrain, especially in flatter areas."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<int32>)
								.Value_Lambda([E = Erosion]() -> TOptional<int32> { return E->MaxPath; })
								.OnValueChanged_Lambda([E = Erosion](int32 Value) { Value = Value >= 0 ? Value : 0; E->MaxPath = Value; })
						]
					]
					+ SVerticalBox::Slot().AutoHeight().Padding(2) // Erosion Radius.
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock)
								.Text(FText::FromString("Erosion Radius"))
								.ToolTipText(FText::FromString("Represents the computation radius of erosion. The larger this value is, the more erosion will be applied over a larger range, smoothing out the furrows created."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<int32>)
								.Value_Lambda([E = Erosion]() -> TOptional<int32> { return E->ErosionRadius; })
								.OnValueChanged_Lambda([E = Erosion](int32 Value) { Value = Value >= 0 ? Value : 0; E->ErosionRadius = Value; })
						]
					]
				]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
		[
			SNew(SSeparator)
		]
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) // "Erode" button.
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
			[
				SNew(SButton)
					.Text(FText::FromString("Erode"))
					.IsEnabled_Lambda([L = ActiveLandscape]() 
					{ 
						if (L && IsValid(*L))
						{
							ULandscapeInfoComponent* Info = (*L)->FindComponentByClass<ULandscapeInfoComponent>();
							return IsValid(Info) && !Info->bIsEroded && !Info->bIsSplittedIntoProxies;
						}

						return false; 
					})
					.OnClicked(this, &SErosionPanel::OnErodeClicked)
			]
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
		[
			SNew(SSeparator)
		]
		+ SVerticalBox::Slot().FillHeight(1.f).Padding(0, 5) // Template Browser (Save/Load/Delete).
		[
			SNew(STemplateBrowser)
				.TemplateManager(TemplateManager)
		]
	];
}

void SErosionPanel::BuildWindDirections()
{
	WindDirections.Empty();

	for (const TCHAR* WindDirectionName : WindDirectionsNames)
	{
		WindDirections.Add(MakeShared<FString>(WindDirectionName));
	}

	CurrentWindDirection = WindDirections[static_cast<int32>(DEFAULT_WIND_DIRECTION)];
}

FReply SErosionPanel::OnErodeClicked()
{
	UGeneratorHeightMapLibrary::GenerateErosion(*ActiveLandscape, *Erosion);

	return FReply::Handled();
}
