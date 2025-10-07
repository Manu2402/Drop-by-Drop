#include "Widget/RootPanel.h"

#include "ErosionScapeSettings.h"
#include "ErosionTemplateManager.h"
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
	// Initialize the shared settings (we use TSharedPtr to avoid empty TSharedRef)
	Heightmap = MakeShared<FHeightMapGenerationSettings>();
	External  = MakeShared<FExternalHeightMapSettings>();
	Landscape = MakeShared<FLandscapeGenerationSettings>();
	ErosionTemplateManager = NewObject<UErosionTemplateManager>();

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

		// Switcher
		+ SHorizontalBox::Slot()
		.FillWidth(.5f)
		[
			BuildCenter()
		]

		// preview + rapid action
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			BuildRightPanel()
		]
	];

	RefreshRightPreview();
}

TSharedRef<SWidget> SRootPanel::BuildSidebar()
{
	auto MakeNavButton = [this](const FText& Label, int32 Index)
	{
		TSharedPtr<STextBlock> TextBlock;
		TSharedRef<SWidget> Button =
			SNew(SButton)
			.ButtonStyle(&FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
			.ContentPadding(FMargin(10, 6))
			.OnClicked(this, &SRootPanel::OnNavClicked, Index)
			[
				SAssignNew(TextBlock, STextBlock)
					.Text(Label)
					.Font(FCoreStyle::GetDefaultFontStyle(Index == ActiveIndex ? "Bold" : "Regular", 10))
			];

		NavButtonTexts.Add(TextBlock);
		return Button;
	};

	return SNew(SBorder)
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
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

	// HeightMap
	+ SWidgetSwitcher::Slot()
	[
		SNew(SHeightMapPanel)
		.Heightmap(Heightmap)
		.External(External)
		.Landscape(Landscape)
	]

	// Landscape
	+ SWidgetSwitcher::Slot()
	[
		SNew(SLandscapePanel)
		.Heightmap(Heightmap.ToSharedRef())
		.External(External.ToSharedRef())
		.Landscape(Landscape.ToSharedRef())
	]
		
	// Erosion
	+ SWidgetSwitcher::Slot()
	[
		SNew(SErosionPanel)
		.Heightmap(Heightmap.ToSharedRef())
		.External(External.ToSharedRef())
		.Landscape(Landscape.ToSharedRef())
		.TemplateManager(ErosionTemplateManager) 
	];
}

TSharedRef<SWidget> SRootPanel::BuildRightPanel()
{
	return
	SNew(SBorder)
	.Padding(8)
	[
		SNew(SVerticalBox)

		// Quick actions 
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("QuickActions", "Quick Actions"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("CreateHM", "Create HeightMap"))
			.OnClicked(this, &SRootPanel::OnActionCreateHeightMap)
			.HAlign(HAlign_Center)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("CreateLS", "Create Landscape"))
			.OnClicked(this, &SRootPanel::OnActionCreateLandscapeInternal)
			.HAlign(HAlign_Center)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("ImportPNG", "Create Landscape from External HeightMap"))
			.OnClicked(this, &SRootPanel::OnActionImportPNG)
			.HAlign(HAlign_Center)
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[
			SNew(SSeparator)
		]

		// Preview heightmap 
		+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
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
	if (Index == ActiveIndex)
	{
		return FReply::Handled();
	}

	if (NavButtonTexts.IsValidIndex(ActiveIndex) && NavButtonTexts[ActiveIndex].IsValid())
	{
		NavButtonTexts[ActiveIndex]->SetFont(
			FCoreStyle::GetDefaultFontStyle("Regular", 10)
		);
	}

	ActiveIndex = Index;

	if (NavButtonTexts.IsValidIndex(Index) && NavButtonTexts[Index].IsValid())
	{
		NavButtonTexts[Index]->SetFont(
			FCoreStyle::GetDefaultFontStyle("Bold", 10)
		);
	}

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
	static const FString DummyPath;
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
	UTexture2D* Loaded = Cast<UTexture2D>(
		StaticLoadObject(UTexture2D::StaticClass(), nullptr,
			TEXT("/ErosionScape/SavedAssets/TextureHeightMap.TextureHeightMap"))
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
