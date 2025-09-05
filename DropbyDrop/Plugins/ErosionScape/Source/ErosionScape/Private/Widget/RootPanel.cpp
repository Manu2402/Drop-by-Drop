#include "Widget/RootPanel.h"

#include "ErosionScapeSettings.h"
#include "GeneratorHeightMapLibrary.h"

#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBorder.h"

#include "Widget/HeightMapPanel.h"
#include "Widget/LandscapePanel.h"
#include "Widget/ErosionPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"

#define LOCTEXT_NAMESPACE "RootPanel"

void SRootPanel::Construct(const FArguments& InArgs)
{
	// Inizializza i settings condivisi (usiamo TSharedPtr per evitare TSharedRef vuoti)
	Heightmap = MakeShared<FHeightMapGenerationSettings>();
	External  = MakeShared<FExternalHeightMapSettings>();
	Landscape = MakeShared<FLandscapeGenerationSettings>();

	// Brush & preview
	RightPreviewBrush = MakeShared<FSlateBrush>();

	ChildSlot
	[
		SNew(SHorizontalBox)

		// Sidebar
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			BuildSidebar()
		]

		// Contenuto centrale (switcher)
		+ SHorizontalBox::Slot()
		.FillWidth(.5f)
		[
			BuildCenter()
		]

		// Colonna destra (preview + azioni rapide)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			BuildRightPane()
		]
	];

	RefreshRightPreview();
}

TSharedRef<SWidget> SRootPanel::BuildSidebar()
{
	auto MakeNavButton = [this](const FText& Label, int32 Index)
	{
		return
		SNew(SButton)
		.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
		.ContentPadding(FMargin(10,6))
		.OnClicked(this, &SRootPanel::OnNavClicked, Index)
		[
			SNew(STextBlock)
			.Text(Label)
			.Font(FCoreStyle::GetDefaultFontStyle(Index == ActiveIndex ? "Bold" : "Regular", 10))
		];
	};

	return SNew(SBorder)
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Tools", "Tools"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]

			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				MakeNavButton(LOCTEXT("HeightMap",  "HeightMap"), 0)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				MakeNavButton(LOCTEXT("Landscape", "Landscape"), 1)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				MakeNavButton(LOCTEXT("Erosion",   "Erosion"), 2)
			]
		];
}

TSharedRef<SWidget> SRootPanel::BuildCenter()
{
	return
	SAssignNew(Switcher, SWidgetSwitcher)
	.WidgetIndex(ActiveIndex)

	// 0) HeightMap
	+ SWidgetSwitcher::Slot()
	[
		SNew(SHeightMapPanel)
		.Heightmap(Heightmap)
		.External(External)
		.Landscape(Landscape)
	]

	// 1) Landscape
	+ SWidgetSwitcher::Slot()
	[
		SNew(SLandscapePanel)
		.Heightmap(Heightmap.ToSharedRef())
		.External(External.ToSharedRef())
		.Landscape(Landscape.ToSharedRef())
	]

	// 2) Erosion
	+ SWidgetSwitcher::Slot()
	[
		SNew(SErosionPanel)
		.Heightmap(Heightmap.ToSharedRef())
		.External(External.ToSharedRef())
		.Landscape(Landscape.ToSharedRef())
		// .TemplateManager(nullptr) // Passa il tuo manager se/quando serve
	];
}

TSharedRef<SWidget> SRootPanel::BuildRightPane()
{
	// Barra azioni + preview persistente
	return
	SNew(SBorder)
	.Padding(8)
	[
		SNew(SVerticalBox)

		// Azioni rapide (dipendono poco dal pannello corrente; non cambiamo la logica)
		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("QuickActions", "Quick actions"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("CreateHM", "Create HeightMap"))
			.OnClicked(this, &SRootPanel::OnActionCreateHeightMap)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("CreateLS", "Create Landscape"))
			.OnClicked(this, &SRootPanel::OnActionCreateLandscapeInternal)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("ImportPNG", "Create Landscape from PNG"))
			.OnClicked(this, &SRootPanel::OnActionImportPNG)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[
			SNew(SSeparator)
		]

		// Preview persistente della heightmap salvata
		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Preview", "Preview"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(2).HAlign(HAlign_Center)
		[
			SNew(SBox).WidthOverride(200).HeightOverride(200)
			[
				SAssignNew(RightPreviewImage, SImage)
				.Image(RightPreviewBrush.Get())
			]
		]
	];
}

FReply SRootPanel::OnNavClicked(const int32 Index)
{
	ActiveIndex = Index;
	if (Switcher.IsValid())
	{
		Switcher->SetActiveWidgetIndex(Index);
	}
	return FReply::Handled();
}

FReply SRootPanel::OnActionCreateHeightMap()
{
	UGeneratorHeightMapLibrary::CreateAndSaveHeightMap(*Heightmap);
	RefreshRightPreview();
	return FReply::Handled();
}

FReply SRootPanel::OnActionCreateLandscapeInternal()
{
	static const FString DummyPath; // l’API accetta una stringa; internamente usa i settings
	UGeneratorHeightMapLibrary::GenerateLandscapeFromPNG(DummyPath, *Heightmap, *External, *Landscape);
	RefreshRightPreview();
	return FReply::Handled();
}

FReply SRootPanel::OnActionImportPNG()
{
	UGeneratorHeightMapLibrary::OpenHeightmapFileDialog(External, Landscape, Heightmap);
	RefreshRightPreview();
	return FReply::Handled();
}

void SRootPanel::RefreshRightPreview()
{
	// Ricarica l’asset salvato dal generatore (stesso path usato ovunque nel progetto)
	UTexture2D* Loaded = Cast<UTexture2D>(
		StaticLoadObject(UTexture2D::StaticClass(), nullptr,
			TEXT("/Game/SavedAssets/TextureHeightMap.TextureHeightMap"))
	);

	RightPreviewTexture.Reset(Loaded);

	if (RightPreviewBrush.IsValid())
	{
		if (RightPreviewTexture.IsValid())
		{
			RightPreviewBrush->SetResourceObject(RightPreviewTexture.Get());
			RightPreviewBrush->ImageSize = FVector2D(
				RightPreviewTexture->GetSizeX(), RightPreviewTexture->GetSizeY());
		}
		else
		{
			RightPreviewBrush->SetResourceObject(nullptr);
			RightPreviewBrush->ImageSize = FVector2D(0,0);
		}
	}

	if (RightPreviewImage.IsValid())
	{
		RightPreviewImage->SetImage(RightPreviewBrush.Get());
	}
}

#undef LOCTEXT_NAMESPACE
