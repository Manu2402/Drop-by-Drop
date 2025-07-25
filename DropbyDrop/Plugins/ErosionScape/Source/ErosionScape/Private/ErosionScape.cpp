// Copyright Epic Games, Inc. All Rights Reserved.

#include "ErosionScape.h"

#include "ErosionLibrary.h"
#include "ErosionScapeStyle.h"
#include "ErosionScapeCommands.h"
#include "GeneratorHeightMapLibrary.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSlider.h" // Per SSlider
#include "Widgets/DeclarativeSyntaxSupport.h" // Per SNew
#include "Subsystems/EditorAssetSubsystem.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

static const FName ErosionScapeTabName("ErosionScape");

#define LOCTEXT_NAMESPACE "FErosionScapeModule"
UTexture2D* FErosionScapeModule::LoadSavedTexture(const FString& AssetName)
{
	FString AssetPath = FString::Printf(TEXT("/Game/SavedAssets/%s.%s"), *AssetName, *AssetName);
	UE_LOG(LogTemp, Warning, TEXT("Trying to load texture: %s"), *AssetPath);

	UTexture2D* LoadedTexture = LoadObject<UTexture2D>(nullptr, *AssetPath);

	if (!LoadedTexture)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadSavedTexture: Failed to load texture %s"), *AssetPath);
		return nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("Loaded texture: %s"), *LoadedTexture->GetName());
	UE_LOG(LogTemp, Warning, TEXT("Texture Size: %d x %d"), LoadedTexture->GetSizeX(), LoadedTexture->GetSizeY());

	if (!LoadedTexture->GetResource())
	{
		UE_LOG(LogTemp, Error, TEXT("Texture has no valid resource!"));
	}

	return LoadedTexture;
}

TSharedPtr<FSlateBrush> FErosionScapeModule::CreateSlateBrushFromTexture(UTexture2D* Texture)
{
	if (!Texture)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSlateBrushFromTexture: Texture is null!"));
		return nullptr;
	}

	if (!Texture->GetResource())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSlateBrushFromTexture: Texture has no valid resource!"));
		return nullptr;
	}

	TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
	Brush->SetResourceObject(Texture);
	Brush->ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());

	return Brush;
}


void FErosionScapeModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FErosionScapeStyle::Initialize();
	FErosionScapeStyle::ReloadTextures();

	FErosionScapeCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FErosionScapeCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FErosionScapeModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FErosionScapeModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ErosionScapeTabName,
		FOnSpawnTab::CreateRaw(
			this, &FErosionScapeModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FErosionScapeTabTitle", "ErosionScape"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	SetErosionTemplates(TEXT("/Game/Custom/ErosionTemplates/DT_ErosionTemplate.DT_ErosionTemplate")); // DataTablePath.
	SetUpTemplates(GetErosionTemplates());

	SetWindDirectionsFromEnum();
}

void FErosionScapeModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module. For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (StoredBrush.IsValid())
	{
		StoredBrush->SetResourceObject(nullptr); // Scollega la texture prima della chiusura
		StoredBrush.Reset();
	}

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FErosionScapeStyle::Shutdown();

	FErosionScapeCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ErosionScapeTabName);

}

TSharedRef<SDockTab> FErosionScapeModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Colonna HeightMap
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					CreateHeightMapColumn()
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					CreateLandScapeColumn() // Metodi per le altre colonne
				]
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					CreateErosionColumn() // Metodi per le altre colonne
				]
		];
}

void FErosionScapeModule::SetUpTemplates(const UDataTable* ErosionTemplatesDataTable)
{
	if (!ErosionTemplatesDataTable)
	{
		return;
	}

	for (const auto& CurrentParam : ErosionTemplatesDataTable->GetRowNames())
	{
		if (CurrentParam.ToString().Equals("None"))
		{
			continue;
		}

		AddTemplate(CurrentParam.ToString());
	}
}

void FErosionScapeModule::AddTemplate(const FString& TemplateName)
{
	if (TemplateName.IsEmpty())
	{
		return;
	}

	TSharedPtr<FString> Template = MakeShared<FString>(TemplateName);

	if (!Template)
	{
		return;
	}

	ErosionTemplatesOptions.Add(Template);
	FilteredErosionTemplatesOptions.Add(Template);

	if (!ListView)
	{
		return;
	}

	ListView->RequestListRefresh();
}

void FErosionScapeModule::DeleteTemplate()
{
	ErosionTemplatesOptions.Remove(CurrentErosionTemplateOption);
	FilteredErosionTemplatesOptions.Remove(CurrentErosionTemplateOption);

	if (!ListView)
	{
		return;
	}

	ListView->RequestListRefresh();
}

UDataTable* FErosionScapeModule::GetErosionTemplates() const
{
	return UGeneratorHeightMapLibrary::GetErosionTemplates();
}

void FErosionScapeModule::SetErosionTemplates(const TCHAR* DataTablePath)
{
	UGeneratorHeightMapLibrary::SetErosionTemplates(DataTablePath);
}

void FErosionScapeModule::SetWindDirectionsFromEnum()
{
	// Unreal "reflection system" in order to get all fields of the "EWindDirection" enum at runtime.
	// This to avoid to rewrote all the fields of the enum every time has been changed.

	WindDirectionEnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EWindDirection"));
	if (!WindDirectionEnumPtr)
	{
		return;
	}

	for (int32 Index = 0; Index < WindDirectionEnumPtr->NumEnums() - 1; Index++)
	{
		WindDirections.Add(MakeShared<FString>(WindDirectionEnumPtr->GetNameStringByIndex(Index)));
	}

	CurrentWindDirection = WindDirections[0];
}

void FErosionScapeModule::ShowEditorNotification(const FString& Message, const bool bSuccess) const
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.bFireAndForget = true;
	Info.bUseLargeFont = true;

	if (bSuccess)
	{
		Info.ExpireDuration = 2.0f;
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.Success"));
	}
	else
	{
		Info.ExpireDuration = 5.0f;
		Info.Image = FCoreStyle::Get().GetBrush(TEXT("NotificationList.Fail"));
		Info.bUseThrobber = false;
		Info.FadeOutDuration = 1.5f;
	}

	FSlateNotificationManager::Get().AddNotification(Info);
}

void FErosionScapeModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ErosionScapeTabName);
}

void FErosionScapeModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FErosionScapeCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(
					FToolMenuEntry::InitToolBarButton(FErosionScapeCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

TSharedRef<SWidget> FErosionScapeModule::CreateHeightMapColumn()
{
	auto CreateNumericEntryWithHover = [](const FString& Label, float MinValue, float MaxValue, float DefaultValue,
		TFunction<void(float)> OnValueChanged)
		{
			// Variabile per mantenere il valore attuale
			TSharedPtr<float> CurrentValue = MakeShareable(new float(DefaultValue));

			return SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
								.Text(FText::FromString(Label))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SNumericEntryBox<float>)
								.Value_Lambda([CurrentValue]() -> TOptional<float>
									{
										return TOptional<float>(*CurrentValue);
									})
								.OnValueCommitted_Lambda(
									[OnValueChanged, CurrentValue](
										float NewValue, ETextCommit::Type CommitType)
									{
										*CurrentValue = NewValue; // Aggiorna il valore attuale
										OnValueChanged(NewValue);
									})
								.OnValueChanged_Lambda(
									[OnValueChanged, CurrentValue](float NewValue)
									{
										*CurrentValue = NewValue; // Aggiorna il valore attuale
										OnValueChanged(NewValue);
									})
								.MinValue(MinValue)
								.MaxValue(MaxValue)
								.AllowSpin(true)
								// Permette l'uso delle frecce per aumentare/diminuire il valore
								.Delta(0.1f) // Imposta il passo di incremento/decremento
								.Visibility(EVisibility::Visible)
								// Assicurati che il NumericEntryBox sia visibile
								.IsEnabled(true) // Abilita il NumericEntryBox
						]
				]
				// Mouse events to allow dragging to modify value
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
						.WidthOverride(200) // Imposta una larghezza fissa o adattiva
						.HeightOverride(20) // Imposta un'altezza fissa o adattiva
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Center)
						[
							SNew(SOverlay) // Utilizza SOverlay per sovrapporre il mouse
								+ SOverlay::Slot()
								[
									SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("NoBorder")) // Nessun bordo visibile
										.HAlign(HAlign_Fill)
										.VAlign(VAlign_Center)
										.OnMouseMove_Lambda(
											[OnValueChanged, MinValue, MaxValue, CurrentValue](
												const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
											{
												// Calcola il valore basato sulla posizione del mouse
												FVector2D LocalMousePos = MouseEvent.GetScreenSpacePosition() -
													MyGeometry.GetAbsolutePosition();
												float NewValue = FMath::Clamp(
													LocalMousePos.X / MyGeometry.GetLocalSize().X * (MaxValue -
														MinValue) + MinValue, MinValue, MaxValue);

												// Aggiorna il valore attuale e chiama la funzione di callback
												*CurrentValue = NewValue;
												OnValueChanged(NewValue);

												return FReply::Handled();
											})
								]
						]
				];
		};
	auto UpdateSeedValue = [](int32 NewValue)
		{
			UGeneratorHeightMapLibrary::SetSeed(NewValue);
			UE_LOG(LogTemp, Log, TEXT("New Seed: %d"), NewValue);
		};

	auto UpdateRandomSeed = [](bool bIsRandom)
		{
			UGeneratorHeightMapLibrary::SetRandomizeSeed(bIsRandom);
			UE_LOG(LogTemp, Log, TEXT("Randomize Seed: %s"), bIsRandom ? TEXT("True") : TEXT("False"));
		};

	auto CreateHeightMap = []()
		{
			int32 MapSize = UGeneratorHeightMapLibrary::GetMapSize();
			UGeneratorHeightMapLibrary::CreateHeightMap(MapSize);
			UE_LOG(LogTemp, Log, TEXT("Creating HeightMap with MapSize: %d"), MapSize);
		};
	//Texture HeightMap
	UTexture2D* LoadedTexture = LoadSavedTexture("TextureHeightMap");
	StoredBrush = CreateSlateBrushFromTexture(LoadedTexture);



	return SNew(SVerticalBox)

		// Nome della colonna
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString("HeightMap"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		// Randomize Seed Checkbox
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Randomize Seed"))
				]
				+ SHorizontalBox::Slot()
				.Padding(5)
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
						.IsChecked_Lambda([]() -> ECheckBoxState
							{
								return UGeneratorHeightMapLibrary::GetRandomizeSeed()
									? ECheckBoxState::Checked
									: ECheckBoxState::Unchecked;
							})
						.OnCheckStateChanged_Lambda([UpdateRandomSeed](ECheckBoxState NewState)
							{
								UpdateRandomSeed(NewState == ECheckBoxState::Checked);
							})
				]
		]

		// Seed
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Seed"))
				]
				+ SHorizontalBox::Slot()
				.Padding(5)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
						.Value_Lambda([]() -> TOptional<int32>
							{
								return TOptional<int32>(UGeneratorHeightMapLibrary::GetSeed());
							})
						.OnValueChanged_Lambda([UpdateSeedValue](int32 NewValue)
							{
								UpdateSeedValue(NewValue);
							})
						.MinValue(0) // Imposta un valore minimo, se necessario
						.MaxValue(TOptional<int32>(10000))
						// Imposta un valore massimo, modifica se necessario
				]
		]

		// Octaves
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				//.VAlign(VAlign_Center)
				.HAlign(HAlign_Left)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Octaves"))
				]
				+ SHorizontalBox::Slot()
				.Padding(5)
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
						.Value_Lambda([]() -> TOptional<int32>
							{
								return TOptional<int32>(UGeneratorHeightMapLibrary::GetOctaves());
							})
						.OnValueChanged_Lambda([](int32 NewValue)
							{
								UGeneratorHeightMapLibrary::SetNumOctaves(NewValue);
							})
						.MinValue(1) // Imposta un valore minimo
						.MaxValue(TOptional<int32>(8)) // Imposta un valore massimo
				]
		]

		// Persistence
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Persistance"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(
									UGeneratorHeightMapLibrary::GetPersistence());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetPersistence(NewValue);
							})
						.MinValue(1) // Imposta un valore minimo
						.MaxValue(TOptional<float>(8)) // Imposta un valore massimo
				]

		]

		// Lacunarity
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Top)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Lacunarity"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(
									UGeneratorHeightMapLibrary::GetLacunarity());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetLacunarity(NewValue);
							})
						.MinValue(0.0f) // Imposta un valore minimo
						.MaxValue(TOptional<float>(10.0f)) // Imposta un valore massimo
				]
		]

		// InitialScale
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Initial Scale"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(
									UGeneratorHeightMapLibrary::GetInitialScale());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetInitialScale(NewValue);
							})
						.MinValue(0.0f) // Imposta un valore minimo
						.MaxValue(TOptional<float>(100.0f)) // Imposta un valore massimo
				]
		]

		// MaxHeightDifference
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Max Height Difference"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(
									UGeneratorHeightMapLibrary::GetMaxHeightDifference());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetMaxHeightDifference(NewValue);
							})
						.MinValue(0.0f) // Imposta un valore minimo
						.MaxValue(TOptional<float>(1000.0f)) // Imposta un valore massimo
				]
		]
		// Create HeightMap button
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString("Create HeightMap"))
						.OnClicked_Lambda([this]()
							{
								int32 MapSize = UGeneratorHeightMapLibrary::GetMapSize();
								UGeneratorHeightMapLibrary::CreateHeightMap(MapSize);
								ShowEditorNotification(TEXT("Heightmap successfully generated!"), true);

								return FReply::Handled();
							})
				]
		]
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		.HAlign(HAlign_Left)  // Centra l'immagine e ne limita la larghezza
		[
			SNew(SBox)
				.WidthOverride(96)
				.HeightOverride(96)
				[
					SNew(SImage)
						.Image(StoredBrush.Get())
				]
		]

		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString("External HeightMap"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		//SCALE
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Scale X"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(
									UGeneratorHeightMapLibrary::GetScalingX());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetScalingX(NewValue);
							})
				]
		]
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Scale Y"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(
									UGeneratorHeightMapLibrary::GetScalingY());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetScalingY(NewValue);
							})
				]
		]
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Scale Z"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(
									UGeneratorHeightMapLibrary::GetScalingZ());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetScalingZ(NewValue);
							})
				]
		]
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString("Create HeightMap from png"))
						.OnClicked_Lambda([this]() // Correggi la lambda se necessario
							{
								// Passa il MapSize al metodo CreateHeightMap
								int32 MapSize = UGeneratorHeightMapLibrary::GetMapSize();
								FString HeightMapPath = FPaths::ProjectDir() + TEXT("Saved/HeightMap/heightmaptest.png");
								UGeneratorHeightMapLibrary::CreateLandscapeFromOtherHeightMap(HeightMapPath);

								ShowEditorNotification(TEXT("Heightmap successfully loaded!"), true);
								return FReply::Handled();
							})
				]
		];

}

TSharedRef<SWidget> FErosionScapeModule::CreateLandScapeColumn()
{
	FString HeightMapPath = FPaths::ProjectDir() + TEXT("Saved/HeightMap/HeightMap.png");
	return SNew(SVerticalBox)
		// Colonna Landscape
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("LandscapeLabel", "Landscape"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) // Font grassetto
		]
		// Flag Destroy Last Landscape
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SCheckBox)
				.OnCheckStateChanged_Lambda([](ECheckBoxState NewState)
					{
						UGeneratorHeightMapLibrary::SetbDestroyLastLandscape(static_cast<bool>(ECheckBoxState::Checked));
					})
				[
					SNew(STextBlock)
						.Text(FText::FromString("Destroy Last Landscape"))
				]
		]

		// Kilometers
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Kilometers"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(UGeneratorHeightMapLibrary::GetKilometers());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetKilometers(NewValue);
							})
				]
		]


		// Bottone Create Landscape
		+SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString("Create LandScape"))
						.OnClicked_Lambda([this, HeightMapPath]()
							{
								if (FPaths::FileExists(HeightMapPath))
								{
									UGeneratorHeightMapLibrary::GenerateLandscapeFromPNG(*HeightMapPath);
									ShowEditorNotification(TEXT("Landscape successfully generated!"), true);
								}
								else
								{
									ShowEditorNotification(TEXT("Landscape generation failed: \"Heightmap.png files doesn't exists!\""), false);
								}

								return FReply::Handled();
							})
				]
		]
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("World Partition Grid Size"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(UGeneratorHeightMapLibrary::GetWorldPartitionGridSize());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UGeneratorHeightMapLibrary::SetWorldPartitionGridSize(NewValue);
							})
				]
		]
		// Bottone Split in Proxies
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString("Split In Proxies"))
						.OnClicked_Lambda([this]()
							{
								UGeneratorHeightMapLibrary::SplitLandscapeIntoProxies();
								ShowEditorNotification(TEXT("Landscape successfully splitted!"), true);

								return FReply::Handled();
							})
				]
		];

}

TSharedPtr<SEditableTextBox> TemplateNameTextBox;

TSharedRef<SWidget> FErosionScapeModule::CreateErosionColumn()
{
	return SNew(SVerticalBox)
		// Colonna Erosion
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("ErosionLabel", "Erosion"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
		// ErosionCycles + Wind Direction
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Erosion Cycles"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
						.Value_Lambda([]() -> TOptional<int32>
							{
								return TOptional<int32>(UErosionLibrary::GetErosionCycles());
							})
						.OnValueChanged_Lambda([](int32 NewValue)
							{
								UErosionLibrary::SetErosion(NewValue);
							})
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(40)
				.AutoWidth()
				[
					SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(5)
						.AutoHeight()
						[
							SNew(STextBlock)
								.Text(FText::FromString("Wind Direction"))
						]

						+ SVerticalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(5)
						.AutoHeight()
						[
							SNew(SComboBox<TSharedPtr<FString>>)
								.OptionsSource(&WindDirections)
								.OnGenerateWidget_Lambda([this](TSharedPtr<FString> WindDirectionSelected) -> TSharedRef<SWidget>
									{
										return SNew(STextBlock).Text(FText::FromString(*WindDirectionSelected));
									})
								.OnSelectionChanged_Lambda([this](TSharedPtr<FString> WindDirectionSelected, ESelectInfo::Type)
									{
										int32 WindValue = WindDirectionEnumPtr->GetValueByNameString(*WindDirectionSelected);
										UErosionLibrary::SetWindDirection(static_cast<EWindDirection>(WindValue));
										CurrentWindDirection = WindDirectionSelected;
									})
								[
									SNew(STextBlock)
										.Text_Lambda([this]() -> FText
											{
												return CurrentWindDirection.IsValid()
													? FText::FromString(*CurrentWindDirection)
													: FText::FromString("Select\nWind Direction");
											})
								]
						]
						+ SVerticalBox::Slot()
						.VAlign(VAlign_Center)
						.Padding(5)
						.AutoHeight()
						[
							SNew(SCheckBox)
								.OnCheckStateChanged_Lambda([](ECheckBoxState NewState)
									{
										UErosionLibrary::SetWindBias(static_cast<bool>(NewState));
									})
								[
									SNew(STextBlock)
										.Text(FText::FromString("Enable Wind Bias"))
								]
						]
				]
		]
		// Inertia
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Inertia"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(UErosionLibrary::GetInertia());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UErosionLibrary::SetInertia(NewValue);
							})
				]
		]
		// Capacity
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Capacity"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
						.Value_Lambda([]() -> TOptional<int32>
							{
								return TOptional<int32>(UErosionLibrary::GetCapacity());
							})
						.OnValueChanged_Lambda([](int32 NewValue)
							{
								UErosionLibrary::SetCapacity(NewValue);
							})
				]
		]
		// MinimalSlope
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Minimal Slope"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(UErosionLibrary::GetMinimalSlope());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UErosionLibrary::SetMinimalSlope(NewValue);
							})
				]
		]
		// DepositionSpeed
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Deposition Speed"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(UErosionLibrary::GetDepositionSpeed());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UErosionLibrary::SetDepositionSpeed(NewValue);
							})
				]
		]
		// ErosionSpeed
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Erosion Speed"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(UErosionLibrary::GetErosionSpeed());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UErosionLibrary::SetErosionSpeed(NewValue);
							})
				]
		]
		// Gravity
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Gravity"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
						.Value_Lambda([]() -> TOptional<int32>
							{
								return TOptional<int32>(UErosionLibrary::GetGravity());
							})
						.OnValueChanged_Lambda([](int32 NewValue)
							{
								UErosionLibrary::SetGravity(NewValue);
							})
				]
		]
		// Evaporation
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Evaporation"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<float>)
						.Value_Lambda([]() -> TOptional<float>
							{
								return TOptional<float>(UErosionLibrary::GetEvaporation());
							})
						.OnValueChanged_Lambda([](float NewValue)
							{
								UErosionLibrary::SetEvaporation(NewValue);
							})
				]
		]
		// MaxPath
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Max Path"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
						.Value_Lambda([]() -> TOptional<int32>
							{
								return TOptional<int32>(UErosionLibrary::GetMaxPath());
							})
						.OnValueChanged_Lambda([](int32 NewValue)
							{
								UErosionLibrary::SetMaxPath(NewValue);
							})
				]
		]
		// ErosionRadius
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(STextBlock)
						.Text(FText::FromString("Erosion Radius"))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(5)
				.AutoWidth()
				[
					SNew(SNumericEntryBox<int32>)
						.Value_Lambda([]() -> TOptional<int32>
							{
								return TOptional<int32>(UErosionLibrary::GetErosionRadius());
							})
						.OnValueChanged_Lambda([](int32 NewValue)
							{
								UErosionLibrary::SetErosionRadius(NewValue);
							})
				]
		]
		// Erosion Button
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString("Erosion"))
						.OnClicked_Lambda([this]()
							{
								UGeneratorHeightMapLibrary::GenerateErosion();
								ShowEditorNotification(TEXT("Erosion successfully applied!"), true);

								return FReply::Handled();
							})
				]
		]

		// Null Widget
		+ SVerticalBox::Slot()
		.Padding(20)
		.AutoHeight()
		[
			SNullWidget::NullWidget // Void space.
		]

		// Save Button + Name Textbox 
		+ SVerticalBox::Slot()
		.Padding(5)
		.AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString("Save"))
						.OnClicked_Lambda([this]()
							{
								UDataTable* ErosionTemplates = GetErosionTemplates();
								if (!ErosionTemplates)
								{
									ShowEditorNotification(TEXT("Template has not been saved: data table doesn't exist!"), false);
									return FReply::Handled();
								}

								const FString TemplateNameString = TemplateNameTextBox->GetText().ToString();

								UGeneratorHeightMapLibrary::SaveErosionTemplate(TemplateNameString,
									UErosionLibrary::GetErosionCycles(), UErosionLibrary::GetWindDirectionTemp(), UErosionLibrary::GetInertia(), UErosionLibrary::GetCapacity(),
									UErosionLibrary::GetMinimalSlope(), UErosionLibrary::GetDepositionSpeed(), UErosionLibrary::GetErosionSpeed(),
									UErosionLibrary::GetGravity(), UErosionLibrary::GetEvaporation(), UErosionLibrary::GetMaxPath(),
									UErosionLibrary::GetErosionRadius());

								// Check for duplicates (unfortunately i can't use the "options" in the ComboBox as set)
								for (const auto& CurrentParam : ErosionTemplatesOptions)
								{
									if (!CurrentParam.IsValid())
									{
										continue;
									}

									if (CurrentParam->Equals(TemplateNameString))
									{
										ShowEditorNotification(TEXT("Template has not been saved: template already exists!"), false);
										return FReply::Handled();
									}
								}

								AddTemplate(TemplateNameString);
								ShowEditorNotification(TEXT("Template has been saved!"), true);

								return FReply::Handled();
							})
				]
				+ SHorizontalBox::Slot()
				.Padding(5)
				.AutoWidth()
				[
					SAssignNew(TemplateNameTextBox, SEditableTextBox)
						.MinDesiredWidth(70.0f)
				]
		]

		// Load Button + Remove Button
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
						.Text(FText::FromString("Load"))
						.OnClicked_Lambda([this]()
							{
								FErosionTemplateRow* SearchedRow = UGeneratorHeightMapLibrary::LoadErosionTemplate(*CurrentErosionTemplateOption);
								if (SearchedRow)
								{
									ShowEditorNotification(TEXT("Template has been loaded!"), true);

									UGeneratorHeightMapLibrary::LoadRowIntoErosionFields(SearchedRow);
									CurrentWindDirection = MakeShared<FString>(WindDirectionEnumPtr->GetNameStringByIndex(SearchedRow->WindDirection));
								}
								else
								{
									ShowEditorNotification(TEXT("Template has not been loaded: template doesn't exists!"), false);
								}

								return FReply::Handled();
							})
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5)
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
								.Text(FText::FromString("Delete"))
								.OnClicked_Lambda([this]()
									{
										UGeneratorHeightMapLibrary::DeleteErosionTemplate(*CurrentErosionTemplateOption);
										DeleteTemplate();

										ShowEditorNotification(TEXT("Template has been deleted!"), false);

										return FReply::Handled();
									})
						]
				]
		]

		// Search Box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SSearchBox)
				.HintText(FText::FromString("Search a template...")) // Query case-sensitive.
				.OnTextChanged_Lambda([this](const FText& FilterQuery)
					{
						FilteredErosionTemplatesOptions.Empty();

						if (FilterQuery.IsEmpty())
						{
							FilteredErosionTemplatesOptions.Append(ErosionTemplatesOptions);
							ListView->RequestListRefresh();

							return;
						}

						for (const TSharedPtr<FString>& ErosionTemplate : ErosionTemplatesOptions)
						{
							FString ErosionTemplateName = *ErosionTemplate;

							if (ErosionTemplateName.StartsWith(FilterQuery.ToString()))
							{
								FilteredErosionTemplatesOptions.Add(ErosionTemplate);
							}
						}

						ListView->RequestListRefresh();
					})
		]

		// List View
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SAssignNew(ListView, SListView<TSharedPtr<FString>>)
				.ListItemsSource(&FilteredErosionTemplatesOptions)
				.OnGenerateRow_Lambda([](TSharedPtr<FString> Template, const TSharedRef<STableViewBase>& OwnerTable)
					{
						return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
							[
								SNew(STextBlock).Text(FText::FromString(*Template))
							];
					})
				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> SelectedTemplate, ESelectInfo::Type SelectInfo)
					{
						CurrentErosionTemplateOption = SelectedTemplate;
					})
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FErosionScapeModule, ErosionScape)