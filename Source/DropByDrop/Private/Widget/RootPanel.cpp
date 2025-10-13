// © Manuel Solano
// © Roberto Capparelli

#include "Widget/RootPanel.h"

#include "DropByDropSettings.h"
#include "ErosionTemplateManager.h"
#include "Libraries/PipelineLibrary.h"
#include "Widget/HeightMapPanel.h"
#include "Widget/LandscapePanel.h"
#include "Widget/ErosionPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "DropByDropLogger.h"

#define LOCTEXT_NAMESPACE "RootPanel"
#define HEIGHTMAP_TEXTURE_PATH "/DropByDrop/SavedAssets/TextureHeightMap.TextureHeightMap"

void SRootPanel::Construct(const FArguments& InArgs)
{
	// Shared settings (we use "TSharedPtr" to avoid empty "TSharedRef").
	Heightmap = MakeShared<FHeightMapGenerationSettings>();
	External  = MakeShared<FExternalHeightMapSettings>();
	Landscape = MakeShared<FLandscapeGenerationSettings>();
	Erosion = MakeShared<FErosionSettings>();

	ErosionTemplateManager = NewObject<UErosionTemplateManager>();
	ErosionTemplateManager->SetErosionSettings(Erosion);

	ActiveLandscape = InArgs._ActiveLandscape;

	// Brush and preview.
	RightPreviewBrush = MakeShared<FSlateBrush>();

	ChildSlot
	[
		SNew(SHorizontalBox)
			+ SHorizontalBox::Slot() // Sidebar.
			.AutoWidth()
			[
				BuildSidebar()
			]
			+ SHorizontalBox::Slot() // Switcher.
			.FillWidth(0.5f)
			[
				BuildCenter()
			]
			+ SHorizontalBox::Slot() // Preview + Quick Actions.
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

		TSharedRef<SWidget> Button = SNew(SButton)
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
	return SAssignNew(Switcher, SWidgetSwitcher).WidgetIndex(ActiveIndex)
		+ SWidgetSwitcher::Slot() // HeightMap.
		[
			SNew(SHeightMapPanel)
				.Heightmap(Heightmap)
				.External(External)
		]
		+ SWidgetSwitcher::Slot() // Landscape.
		[
			SNew(SLandscapePanel)
				.Heightmap(Heightmap)
				.External(External)
				.Landscape(Landscape)
				.ActiveLandscape(ActiveLandscape)
		]
		+ SWidgetSwitcher::Slot() // Erosion.
		[
			SNew(SErosionPanel)
				.Landscape(Landscape)
				.ActiveLandscape(ActiveLandscape)
				.Erosion(Erosion)
				.TemplateManager(ErosionTemplateManager)
		];
}

TSharedRef<SWidget> SRootPanel::BuildRightPanel()
{
	return SNew(SBorder)
		.Padding(8)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) // Quick actions.
			[
				SNew(STextBlock)
					.Text(LOCTEXT("QuickActions", "Quick Actions"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				SNew(SButton)
					.Text(LOCTEXT("CreateHeightMap", "Create HeightMap"))
					.OnClicked(this, &SRootPanel::OnActionCreateHeightMap)
					.HAlign(HAlign_Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				SNew(SButton)
					.Text(LOCTEXT("CreateLandscape", "Create Landscape"))
					.OnClicked(this, &SRootPanel::OnActionCreateLandscapeInternal)
					.HAlign(HAlign_Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(2)
			[
				SNew(SButton)
					.Text(LOCTEXT("ExternalHeightmap", "Create Landscape from External HeightMap"))
					.OnClicked(this, &SRootPanel::OnActionImportAndCreateLandscapeExternal)
					.HAlign(HAlign_Center)
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(8)
			[
				SNew(SSeparator)
			]
			+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center) // Preview heightmap.
			[
				SNew(STextBlock)
					.Text(LOCTEXT("Preview", "Preview"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(2).HAlign(HAlign_Center)
			[
				SNew(SBox)
					.WidthOverride(200)
					.HeightOverride(200)
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
		NavButtonTexts[ActiveIndex]->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 10));
	}

	ActiveIndex = Index;

	if (NavButtonTexts.IsValidIndex(Index) && NavButtonTexts[Index].IsValid())
	{
		NavButtonTexts[Index]->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10));
	}

	if (!Switcher.IsValid())
	{
		UE_LOG(LogDropByDrop, Error, TEXT("The \"Switcher\" widget is invalid!"));
		return FReply::Handled();
	}
		
	Switcher->SetActiveWidgetIndex(Index);

	return FReply::Handled();
}

FReply SRootPanel::OnActionCreateHeightMap()
{
	if (!Heightmap.IsValid())
	{
		UE_LOG(LogDropByDrop, Error, TEXT("The \"Heightmap\" resource is invalid!"));
		return FReply::Handled();
	}

	UPipelineLibrary::CreateAndSaveHeightMap(*Heightmap);
	RefreshRightPreview();

	return FReply::Handled();
}

FReply SRootPanel::OnActionCreateLandscapeInternal()
{
	if (!Heightmap.IsValid() || !External.IsValid() || !Landscape.IsValid())
	{
		UE_LOG(LogDropByDrop, Error, TEXT("One of \"Heightmap\", \"External\" or \"Landscape\" resources is invalid!"));
		return FReply::Handled();
	}

	UPipelineLibrary::CreateLandscapeFromInternalHeightMap(*Heightmap, *External, *Landscape);
	RefreshRightPreview();

	return FReply::Handled();
}

FReply SRootPanel::OnActionImportAndCreateLandscapeExternal()
{
	if (!Heightmap.IsValid() || !External.IsValid() || !Landscape.IsValid())
	{
		UE_LOG(LogDropByDrop, Error, TEXT("One of \"Heightmap\", \"External\" or \"Landscape\" resources is invalid!"));
		return FReply::Handled();
	}

	if (!UPipelineLibrary::OpenHeightmapFileDialog(External))
	{
		return FReply::Handled();
	}

	UPipelineLibrary::CreateLandscapeFromExternalHeightMap(External->LastPNGPath, *External, *Landscape, *Heightmap);
	RefreshRightPreview();

	return FReply::Handled();
}

void SRootPanel::RefreshRightPreview()
{
	UTexture2D* LoadedTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT(HEIGHTMAP_TEXTURE_PATH)));
	RightPreviewTexture.Reset(LoadedTexture);

	if (RightPreviewBrush.IsValid())
	{
		if (RightPreviewTexture.IsValid())
		{
			RightPreviewBrush->SetResourceObject(RightPreviewTexture.Get());
			RightPreviewBrush->ImageSize = FVector2D(RightPreviewTexture->GetSizeX(), RightPreviewTexture->GetSizeY());
		}
		else
		{
			RightPreviewBrush->SetResourceObject(nullptr);
			RightPreviewBrush->ImageSize = FVector2D(0, 0);
		}
	}

	if (RightPreviewImage.IsValid())
	{
		RightPreviewImage->SetImage(RightPreviewBrush.Get());
	}
}

#undef LOCTEXT_NAMESPACE
