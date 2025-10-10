#include "Widget/ErosionPanel.h"
#include "Widget/TemplateBrowser.h"

#include "ErosionLibrary.h"
#include "GeneratorHeightMapLibrary.h"

#include "Landscape.h"

#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "DropByDropLandscape.h"

void SErosionPanel::Construct(const FArguments& Args)
{
	Heightmap = Args._Heightmap;
	External = Args._External;
	Landscape = Args._Landscape;
	SelectedLandscape = Args._SelectedLandscape;
	Erosion = Args._Erosion;
	TemplateManager = Args._TemplateManager;

	check(Heightmap.IsValid() && External.IsValid() && Landscape.IsValid() && Erosion.IsValid());

	BuildWindDirections();

	ChildSlot
		[
			SNew(SVerticalBox)

				// Title
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Erosion"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]

				// Basic controls (always visible) 
				// Erosion Cycles (keep width compact so it doesn't stretch with Wind controls)
				+SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(STextBlock).Text(FText::FromString("Erosion Cycles"))
							.ToolTipText(FText::FromString("Number of computed erosion cycles, equivalent to the num ber of drops falling from the sky that will follow their course. The more drops that come in, the more the erosion impact on the ground will be accentuated."))
						]
						+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
						[
							SNew(SNumericEntryBox<int64>)
								.Value_Lambda([E = Erosion]()-> TOptional<int64> { return E->ErosionCycles; })
								.OnValueChanged_Lambda([E = Erosion](int64 V) { V = V >= 0 ? V : 0; E->ErosionCycles = V; })
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
							SNew(STextBlock).Text(FText::FromString("Wind Direction"))
							.ToolTipText(FText::FromString("Direction of wind force application on each individual drop."))
						]

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
						[

							SNew(SHorizontalBox)

								// Wind Direction
								+ SHorizontalBox::Slot().AutoWidth()
								[

									SAssignNew(WindCombo, SComboBox<TSharedPtr<FString>>)
										.OptionsSource(&WindDirections)
										.InitiallySelectedItem(CurrentWindDirection)
										.MaxListHeight(300.f)
										.OnGenerateWidget_Lambda([](TSharedPtr<FString> Option) -> TSharedRef<SWidget>
											{
												return SNew(STextBlock).Text(FText::FromString(Option.IsValid() ? *Option : TEXT("")));
											})
										.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Option, ESelectInfo::Type)
											{
												if (!Option.IsValid())
													return;

												CurrentWindDirection = Option;

												if (WindDirectionEnumPtr)
												{
													const int32 WindValue = WindDirectionEnumPtr->GetValueByNameString(*Option);
													if (WindValue != INDEX_NONE)
													{
														Erosion->WindDirection = static_cast<EWindDirection>(WindValue);
														return;
													}
												}
												// Fallback mapping by name
												const FString& Name = *Option;
												EWindDirection Dir = EWindDirection::Random;
												if (Name == TEXT("Nord")) Dir = EWindDirection::Nord;
												else if (Name == TEXT("Sud")) Dir = EWindDirection::Sud;
												else if (Name == TEXT("Est")) Dir = EWindDirection::Est;
												else if (Name == TEXT("Ovest")) Dir = EWindDirection::Ovest;
												else if (Name == TEXT("Nord_Ovest")) Dir = EWindDirection::Nord_Ovest;
												else if (Name == TEXT("Nord_Est")) Dir = EWindDirection::Nord_Est;
												else if (Name == TEXT("Sud_Ovest")) Dir = EWindDirection::Sud_Ovest;
												else if (Name == TEXT("Sud_Est")) Dir = EWindDirection::Sud_Est;

												Erosion->WindDirection = Dir;
											})
										[
											// Selected content
											SNew(STextBlock).Text_Lambda([this]()
												{
													return CurrentWindDirection.IsValid()
														? FText::FromString(*CurrentWindDirection)
														: FText::FromString("Select\nWind Direction");
												})
										]
								]

							// Wind Bias
							+SHorizontalBox::Slot().AutoWidth().Padding(8, 0).VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Wind Bias"))
											.ToolTipText(FText::FromString("Bias applied to the selected wind direction (no random). This is used to make the application of the wind direction less pronounced, but still predominant, in order to make the final effect more realistic."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0)
										[
											SNew(SCheckBox)
												.OnCheckStateChanged_Lambda([E = Erosion](ECheckBoxState S)
													{
														E->bWindBias = (S == ECheckBoxState::Checked);
													})
										]
								]
						]

					+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 5)
						[

							SNew(SButton)
								.IsEnabled_Lambda([L = SelectedLandscape]() {
								if (L && IsValid(*L))
								{
									ULandscapeInfoComponent* Info = (*L)->FindComponentByClass<ULandscapeInfoComponent>();
									return IsValid(Info) && !Info->bIsSplittedIntoProxies;
								}

								return false; })
								.IsEnabled_Lambda([L = SelectedLandscape, E = Erosion]() { return L && IsValid(*L) && E->WindDirection != EWindDirection::Random; })
								.Text(FText::FromString("Wind Preview"))
								.OnClicked_Lambda([this]()
									{
										if (!Landscape.IsValid())
											return FReply::Handled();

										UGeneratorHeightMapLibrary::DrawWindDirectionPreview(*Erosion,
											*SelectedLandscape, /*ArrowLength*/8000.f, /*Thickness*/12.f, /*Head*/300.f,
											/*Duration*/6.f, /*Cone*/true, /*ConeHalf*/15.f);

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
								+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Inertia"))
											.ToolTipText(FText::FromString("Parameter indicating the inertia value of the drop. The closer the parameter is to 0 the more the drop will tend to follow a pit as it falls, thus creating more pronounced ruts, while the closer the value is to 1 the more the drop will tend to follow the course of its original path, thus creating a more unnatural effect."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([E = Erosion]()-> TOptional<float> { return E->Inertia; })
												.OnValueChanged_Lambda([E = Erosion](float V) { V = FMath::Clamp(V, 0.f, 1.f); E->Inertia = V; })
										]
								]

							// Capacity
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Capacity"))
												.ToolTipText(FText::FromString("Amount of sediment that is capable of transporting the droplet during its path. The greater this value is, the greater is both the erosion effect applied on the land and the deposition effect in flat areas."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<int32>)
												.Value_Lambda([E = Erosion]()-> TOptional<int32> { return E->Capacity; })
												.OnValueChanged_Lambda([E = Erosion](int32 V) { V = V >= 0 ? V : 0; E->Capacity = V; })
										]
								]

							// Minimal Slope
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Minimal Slope"))
											.ToolTipText(FText::FromString("Minimal slope taken into account when calculating the carrying capacity. The smaller this slope is, the more realistic the final terrain will be, due to the fact that the erosion effect is accounted for even in the finer movements of the drop. In contrast, the greater the value, the more intense the computed erosion will be, to the point of creating ruins in the most extreme cases."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([E = Erosion]()-> TOptional<float> { return E->MinimalSlope; })
												.OnValueChanged_Lambda([E = Erosion](float V) { V = V >= 0.f ? V : 0.f; E->MinimalSlope = V; })
										]
								]

							// Deposition Speed
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Deposition Speed"))
											.ToolTipText(FText::FromString("Deposition factor of the sediment transported by the droplet. The more this parameter tends to 1, the more sediment will be deposited on the occasions involved, while the closer it is to 0, the less this sediment will be deposited."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([E = Erosion]()-> TOptional<float> { return E->DepositionSpeed; })
												.OnValueChanged_Lambda([E = Erosion](float V) { V = FMath::Clamp(V, 0.f, 1.f); E->DepositionSpeed = V; })
										]
								]

							// Erosion Speed
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Erosion Speed"))
											.ToolTipText(FText::FromString("Erosion factor of the drop. The more this parameter tends to 1, the more sediment will be eroded during the lifetime of the drop, while the closer it is to 0, the less this sediment will be eroded from the soil."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([E = Erosion]()-> TOptional<float> { return E->ErosionSpeed; })
												.OnValueChanged_Lambda([E = Erosion](float V) { V = FMath::Clamp(V, 0.f, 1.f); E->ErosionSpeed = V; })
										]
								]

							// Gravity
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Gravity"))
											.ToolTipText(FText::FromString("Represents the force of gravity acting on the droplet. The greater this value, the greater the force of gravity applied on it, thus causing it to accelerate more or less downward. This will result in greater computation of the drop in flat areas."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<int32>)
												.Value_Lambda([E = Erosion]()-> TOptional<int32> { return E->Gravity; })
												.OnValueChanged_Lambda([E = Erosion](int32 V) { V = V >= 0 ? V : 0; E->Gravity = V; })
										]
								]

							// Evaporation
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Evaporation"))
											.ToolTipText(FText::FromString("Evaporation index of the droplet. The more this value tends to 1, the more the drop will be subject to evaporation, and thus it will become smaller and smaller in size during its iterations. This will cause less sediment collection and consequently also on erosion and deposition."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<float>)
												.Value_Lambda([E = Erosion]()-> TOptional<float> { return E->Evaporation; })
												.OnValueChanged_Lambda([E = Erosion](float V) { V = FMath::Clamp(V, 0.f, 1.f); E->Evaporation = V; })
										]
								]

							// Max Path
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Max Path"))
											.ToolTipText(FText::FromString("Represents the number of steps taken by the droplet before it dis appears. The higher this value, the greater the impact these drops will tend to have on the entire terrain, especially in flatter areas."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<int32>)
												.Value_Lambda([E = Erosion]()-> TOptional<int32> { return E->MaxPath; })
												.OnValueChanged_Lambda([E = Erosion](int32 V) { V = V >= 0 ? V : 0; E->MaxPath = V; })
										]
								]

							// Erosion Radius
							+ SVerticalBox::Slot().AutoHeight().Padding(2)
								[
									SNew(SHorizontalBox)
										+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
										[
											SNew(STextBlock).Text(FText::FromString("Erosion Radius"))
											.ToolTipText(FText::FromString("Represents the computation radius of erosion. The larger this value is, the more erosion will be applied over a larger range, smoothing out the furrows created."))
										]
										+ SHorizontalBox::Slot().AutoWidth().Padding(5, 0)
										[
											SNew(SNumericEntryBox<int32>)
												.Value_Lambda([E = Erosion]()-> TOptional<int32> { return E->ErosionRadius; })
												.OnValueChanged_Lambda([E = Erosion](int32 V) { V = V >= 0 ? V : 0; E->ErosionRadius = V; })
										]
								]

						]
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]

				// Run Erosion
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SButton)
								.Text(FText::FromString("Erode"))
								.IsEnabled_Lambda([L = SelectedLandscape]() { 
								if (L && IsValid(*L))
								{
									ULandscapeInfoComponent* Info = (*L)->FindComponentByClass<ULandscapeInfoComponent>();
									return IsValid(Info) && !Info->bIsEroded && !Info->bIsSplittedIntoProxies;
								}

								return false; })
								.OnClicked(this, &SErosionPanel::OnErodeClicked)
						]
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(8, 5)
				[
					SNew(SSeparator)
				]

				// Template Browser (Save/Load/Delete)
				+ SVerticalBox::Slot().FillHeight(1.f).Padding(0, 5)
				[
					SNew(STemplateBrowser)
						.TemplateManager(TemplateManager)
				]
		];
}

void SErosionPanel::BuildWindDirections()
{
	WindDirections.Empty();

	// Load enum by full path (no ANY_PACKAGE, no StaticEnum)
	WindDirectionEnumPtr = LoadObject<UEnum>(nullptr, TEXT("/Script/ErosionScape.EWindDirection"));

	if (WindDirectionEnumPtr)
	{
		const int32 Count = WindDirectionEnumPtr->NumEnums();
		const int32 Visible = Count > 0 ? Count - 1 : 0; // often last is _MAX/Hidden

		for (int32 i = 0; i < Visible; ++i)
		{
			WindDirections.Add(MakeShared<FString>(WindDirectionEnumPtr->GetNameStringByIndex(i)));
		}
	}

	// Fallback list so the combo always opens even if the enum can't be loaded
	if (WindDirections.Num() == 0)
	{
		static const TCHAR* Fallback[] = {
			TEXT("Random"), TEXT("Nord"), TEXT("Sud"), TEXT("Est"), TEXT("Ovest"),
			TEXT("Nord_Ovest"), TEXT("Nord_Est"), TEXT("Sud_Ovest"), TEXT("Sud_Est")
		};
		for (const TCHAR* N : Fallback)
		{
			WindDirections.Add(MakeShared<FString>(N));
		}
	}

	// Select first item and sync to UErosionLibrary
	if (WindDirections.Num() > 0)
	{
		CurrentWindDirection = WindDirections[0];

		if (WindDirectionEnumPtr && CurrentWindDirection.IsValid())
		{
			const int32 WindValue = WindDirectionEnumPtr->GetValueByNameString(*CurrentWindDirection);
			if (WindValue != INDEX_NONE)
			{
				Erosion->WindDirection = static_cast<EWindDirection>(WindValue);
			}
		}
		else
		{
			// Fallback mapping by name (if enum couldn't be loaded)
			const FString& Name = *CurrentWindDirection;
			EWindDirection Dir = EWindDirection::Random;
			if (Name == TEXT("Nord")) Dir = EWindDirection::Nord;
			else if (Name == TEXT("Sud")) Dir = EWindDirection::Sud;
			else if (Name == TEXT("Est")) Dir = EWindDirection::Est;
			else if (Name == TEXT("Ovest")) Dir = EWindDirection::Ovest;
			else if (Name == TEXT("Nord_Ovest")) Dir = EWindDirection::Nord_Ovest;
			else if (Name == TEXT("Nord_Est")) Dir = EWindDirection::Nord_Est;
			else if (Name == TEXT("Sud_Ovest")) Dir = EWindDirection::Sud_Ovest;
			else if (Name == TEXT("Sud_Est")) Dir = EWindDirection::Sud_Est;

			Erosion->WindDirection = Dir;
		}
	}
}


FReply SErosionPanel::OnErodeClicked()
{
	// Drive the erosion pipeline (reads current HeightMap from UGeneratorHeightMapLibrary)
	UGeneratorHeightMapLibrary::GenerateErosion(
		*SelectedLandscape, *Erosion
	);
	return FReply::Handled();
}
