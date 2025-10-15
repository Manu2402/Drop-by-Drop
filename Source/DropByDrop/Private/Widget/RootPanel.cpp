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

// Path to the saved heightmap texture asset used for preview.
#define HEIGHTMAP_TEXTURE_PATH "/DropByDrop/SavedAssets/TextureHeightMap.TextureHeightMap"

void SRootPanel::Construct(const FArguments& InArgs)
{
	// Initialize shared settings (using TSharedPtr to allow empty TSharedRef initially).
	Heightmap = MakeShared<FHeightMapGenerationSettings>();
	External = MakeShared<FExternalHeightMapSettings>();
	Landscape = MakeShared<FLandscapeGenerationSettings>();
	Erosion = MakeShared<FErosionSettings>();

	// Create and configure erosion template manager.
	ErosionTemplateManager = NewObject<UErosionTemplateManager>();
	ErosionTemplateManager->SetErosionSettings(Erosion);

	// Store reference to active landscape from arguments.
	ActiveLandscape = InArgs._ActiveLandscape;

	// Initialize preview brush for heightmap display.
	RightPreviewBrush = MakeShared<FSlateBrush>();

	// Construct the main horizontal layout: Sidebar | Center Panel | Right Panel
	ChildSlot
		[
			SNew(SHorizontalBox)
				// Left: Navigation sidebar.
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					BuildSidebar()
				]
				// Center: Panel switcher (50% width).
				+ SHorizontalBox::Slot()
				.FillWidth(0.5f)
				[
					BuildCenter()
				]
				// Right: Preview and quick actions.
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					BuildRightPanel()
				]
		];

	// Load and display the initial heightmap preview.
	RefreshRightPreview();
}

TSharedRef<SWidget> SRootPanel::BuildSidebar()
{
	// Lambda helper: Creates a navigation button with consistent styling.
	// Stores text block reference for later font updates when selection changes.
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
						// Bold if this is the active panel, regular otherwise.
						.Font(FCoreStyle::GetDefaultFontStyle(Index == ActiveIndex ? "Bold" : "Regular", 10))
				];

			// Store text block reference for font style updates.
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

				// Navigation button: HeightMap panel (index 0).
				+ SVerticalBox::Slot().AutoHeight().Padding(2)
				[
					MakeNavButton(LOCTEXT("HeightMap", "HeightMap"), 0)
				]
				// Navigation button: Landscape panel (index 1).
				+ SVerticalBox::Slot().AutoHeight().Padding(2)
				[
					MakeNavButton(LOCTEXT("Landscape", "Landscape"), 1)
				]
				// Navigation button: Erosion panel (index 2).
				+ SVerticalBox::Slot().AutoHeight().Padding(2)
				[
					MakeNavButton(LOCTEXT("Erosion", "Erosion"), 2)
				]
		];
}

TSharedRef<SWidget> SRootPanel::BuildCenter()
{
	return SAssignNew(Switcher, SWidgetSwitcher).WidgetIndex(ActiveIndex)
		// Slot 0: HeightMap generation panel.
		+ SWidgetSwitcher::Slot()
		[
			SNew(SHeightMapPanel)
				.Heightmap(Heightmap)
				.External(External)
		]
		// Slot 1: Landscape creation panel.
		+ SWidgetSwitcher::Slot()
		[
			SNew(SLandscapePanel)
				.Heightmap(Heightmap)
				.External(External)
				.Landscape(Landscape)
				.ActiveLandscape(ActiveLandscape)
		]
	// Slot 2: Erosion simulation panel.
	+ SWidgetSwitcher::Slot()
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
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(LOCTEXT("QuickActions", "Quick Actions"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
				// Quick action: Create heightmap from procedural settings.
				+ SVerticalBox::Slot().AutoHeight().Padding(2)
				[
					SNew(SButton)
						.Text(LOCTEXT("CreateHeightMap", "Create HeightMap"))
						.OnClicked(this, &SRootPanel::OnActionCreateHeightMap)
						.HAlign(HAlign_Center)
				]
				// Quick action: Create landscape from internal heightmap.
				+ SVerticalBox::Slot().AutoHeight().Padding(2)
				[
					SNew(SButton)
						.Text(LOCTEXT("CreateLandscape", "Create Landscape"))
						.OnClicked(this, &SRootPanel::OnActionCreateLandscapeInternal)
						.HAlign(HAlign_Center)
				]
				// Quick action: Import external heightmap and create landscape.
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
				+ SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(LOCTEXT("Preview", "Preview"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
				// Preview image: 200x200 heightmap display.
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
	// Early exit if clicking already active panel.
	if (Index == ActiveIndex)
	{
		return FReply::Handled();
	}

	// Update previous button: Change font to Regular.
	if (NavButtonTexts.IsValidIndex(ActiveIndex) && NavButtonTexts[ActiveIndex].IsValid())
	{
		NavButtonTexts[ActiveIndex]->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 10));
	}

	// Update active index.
	ActiveIndex = Index;

	// Update new button: Change font to Bold.
	if (NavButtonTexts.IsValidIndex(Index) && NavButtonTexts[Index].IsValid())
	{
		NavButtonTexts[Index]->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10));
	}

	// Switch to the new panel.
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
	// Validate heightmap settings exist.
	if (!Heightmap.IsValid())
	{
		UE_LOG(LogDropByDrop, Error, TEXT("The \"Heightmap\" resource is invalid!"));
		return FReply::Handled();
	}

	// Create heightmap using pipeline library.
	UPipelineLibrary::CreateAndSaveHeightMap(*Heightmap);

	// Update preview to show newly created heightmap.
	RefreshRightPreview();

	return FReply::Handled();
}

FReply SRootPanel::OnActionCreateLandscapeInternal()
{
	// Validate all required settings exist.
	if (!Heightmap.IsValid() || !External.IsValid() || !Landscape.IsValid())
	{
		UE_LOG(LogDropByDrop, Error, TEXT("One of \"Heightmap\", \"External\" or \"Landscape\" resources is invalid!"));
		return FReply::Handled();
	}

	// Create landscape from internal generated heightmap.
	UPipelineLibrary::CreateLandscapeFromInternalHeightMap(*Heightmap, *External, *Landscape);

	// Refresh preview (in case heightmap was regenerated).
	RefreshRightPreview();

	return FReply::Handled();
}

FReply SRootPanel::OnActionImportAndCreateLandscapeExternal()
{
	// Validate all required settings exist.
	if (!Heightmap.IsValid() || !External.IsValid() || !Landscape.IsValid())
	{
		UE_LOG(LogDropByDrop, Error, TEXT("One of \"Heightmap\", \"External\" or \"Landscape\" resources is invalid!"));
		return FReply::Handled();
	}

	// Open file dialog for heightmap selection (returns false if user cancels).
	if (!UPipelineLibrary::OpenHeightmapFileDialog(External))
	{
		return FReply::Handled();
	}

	// Import heightmap and create landscape.
	UPipelineLibrary::CreateLandscapeFromExternalHeightMap(External->LastPNGPath, *External, *Landscape, *Heightmap);

	// Update preview to show imported heightmap.
	RefreshRightPreview();

	return FReply::Handled();
}

void SRootPanel::RefreshRightPreview()
{
	// Load heightmap texture from saved asset path.
	UTexture2D* LoadedTexture = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT(HEIGHTMAP_TEXTURE_PATH)));
	RightPreviewTexture.Reset(LoadedTexture);

	// Update slate brush with loaded texture.
	if (RightPreviewBrush.IsValid())
	{
		if (RightPreviewTexture.IsValid())
		{
			// Set texture and update brush dimensions.
			RightPreviewBrush->SetResourceObject(RightPreviewTexture.Get());
			RightPreviewBrush->ImageSize = FVector2D(RightPreviewTexture->GetSizeX(), RightPreviewTexture->GetSizeY());
		}
		else
		{
			// Clear brush if texture failed to load.
			RightPreviewBrush->SetResourceObject(nullptr);
			RightPreviewBrush->ImageSize = FVector2D(0, 0);
		}
	}

	// Update image widget to display new brush.
	if (RightPreviewImage.IsValid())
	{
		RightPreviewImage->SetImage(RightPreviewBrush.Get());
	}
}

#undef LOCTEXT_NAMESPACE