#include "Widget/HeightMapPanel.h"
#include "GeneratorHeightMapLibrary.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SSeparator.h"

namespace
{
    static TSharedPtr<FSlateBrush> MakeBrush(UTexture2D* Texture)
    {
        if (!Texture) return nullptr;
        TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
        Brush->SetResourceObject(Texture);
        Brush->ImageSize = FVector2D(Texture->GetSizeX(), Texture->GetSizeY());
        return Brush;
    }
}

void SHeightMapPanel::Construct(const FArguments& Args)
{
    Heightmap = Args._Heightmap;
    External  = Args._External;
    Landscape = Args._Landscape;
    check(Heightmap.IsValid() && External.IsValid() && Landscape.IsValid());
    

    RefreshPreview();

    ChildSlot
    [
        SNew(SVerticalBox)

        // Hero
        + SVerticalBox::Slot().AutoHeight().Padding(6)
        [ SNew(STextBlock).Text(FText::FromString("HeightMap")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]

        // Randomize Seed
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(STextBlock).Text(FText::FromString("Randomize Seed")) ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SCheckBox)
                .OnCheckStateChanged_Lambda([this](ECheckBoxState State){ Heightmap->bRandomizeSeed = (State == ECheckBoxState::Checked); })
            ]
        ]

        // Seed
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(STextBlock).Text(FText::FromString("Seed")) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(5)
            [
                SNew(SNumericEntryBox<int32>)
                .Value_Lambda([this]()->TOptional<int32>{ return Heightmap->Seed; })
                .OnValueChanged_Lambda([this](int32 Value){ Heightmap->Seed = Value; })
            ]
        ]

        // NumOctaves, Persistence, Lacunarity, InitialScale, Size, MaxHeightDifference
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Octaves")) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(5)
            [
                SNew(SNumericEntryBox<int32>)
                .Value_Lambda([this]()->TOptional<int32>{ return Heightmap->NumOctaves; })
                .OnValueChanged_Lambda([this](int32 Value){ Heightmap->NumOctaves = Value; })
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Persistence")) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(5)
            [
                SNew(SNumericEntryBox<float>)
                .Value_Lambda([this]()->TOptional<float>{ return Heightmap->Persistence; })
                .OnValueChanged_Lambda([this](float Value){ Heightmap->Persistence = Value; })
                .MinValue(0.0f).MaxValue(10.0f)
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Lacunarity")) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(5)
            [
                SNew(SNumericEntryBox<float>)
                .Value_Lambda([this]()->TOptional<float>{ return Heightmap->Lacunarity; })
                .OnValueChanged_Lambda([this](float Value){ Heightmap->Lacunarity = Value; })
                .MinValue(0.0f).MaxValue(10.0f)
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Initial Scale")) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(5)
            [
                SNew(SNumericEntryBox<float>)
                .Value_Lambda([this]()->TOptional<float>{ return Heightmap->InitialScale; })
                .OnValueChanged_Lambda([this](float Value){ Heightmap->InitialScale = Value; })
                .MinValue(0.0f).MaxValue(100.0f)
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Size")) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(5)
            [
                SNew(SNumericEntryBox<int32>)
                .Value_Lambda([this]()->TOptional<int32>{ return Heightmap->Size; })
                .OnValueChanged_Lambda([this](int32 Value){ Heightmap->Size = Value; })
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Max Height Difference")) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(5)
            [
                SNew(SNumericEntryBox<float>)
                .Value_Lambda([this]()->TOptional<float>{ return Heightmap->MaxHeightDifference; })
                .OnValueChanged_Lambda([this](float Value){ Heightmap->MaxHeightDifference = Value; })
                .MinValue(0.0f).MaxValue(1000.0f)
            ]
        ]

        // Create HeightMap
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SButton)
            .Text(FText::FromString("Create HeightMap"))
            .OnClicked(this, &SHeightMapPanel::OnCreateHeightmapClicked)
        ]

        // Preview
        + SVerticalBox::Slot().AutoHeight().Padding(5).HAlign(HAlign_Left)
        [
            SNew(SBox).WidthOverride(96).HeightOverride(96)
            [
                SNew(SImage).Image(HeightPreviewBrush.Get())
            ]
        ]

        // External HeightMap
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [ SNew(STextBlock).Text(FText::FromString("External HeightMap")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12)) ]

        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Scale X")) ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SNumericEntryBox<float>)
                .Value_Lambda([this]()->TOptional<float>{ return External->ScalingX; })
                .OnValueChanged_Lambda([this](float Value){ External->ScalingX = Value; })
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Scale Y")) ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SNumericEntryBox<float>)
                .Value_Lambda([this]()->TOptional<float>{ return External->ScalingY; })
                .OnValueChanged_Lambda([this](float Value){ External->ScalingY = Value; })
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()[ SNew(STextBlock).Text(FText::FromString("Scale Z")) ]
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SNumericEntryBox<float>)
                .Value_Lambda([this]()->TOptional<float>{ return External->ScalingZ; })
                .OnValueChanged_Lambda([this](float Value){ External->ScalingZ = Value; })
            ]
        ]

        // Create Landscape from internal / from PNG external
        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()
            [
                SNew(SButton).Text(FText::FromString("Create Landscape (Internal)"))
                .OnClicked(this, &SHeightMapPanel::OnCreateLandscapeFromInternalClicked)
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(5)
            [
                SNew(SButton).Text(FText::FromString("Create Landscape from PNG..."))
                .OnClicked(this, &SHeightMapPanel::OnCreateLandscapeFromPNGClicked)
            ]
        ]

        + SVerticalBox::Slot().AutoHeight().Padding(5)
        [ SNew(SSeparator) ]
    ];
}

void SHeightMapPanel::RefreshPreview()
{
    if (UTexture2D* Existing = Cast<UTexture2D>(StaticLoadObject(UTexture2D::StaticClass(), nullptr, TEXT("/Game/SavedAssets/TextureHeightMap.TextureHeightMap"))))
    {
        HeightPreviewBrush = MakeBrush(Existing);
    }
}

FReply SHeightMapPanel::OnCreateHeightmapClicked()
{
    UGeneratorHeightMapLibrary::CreateAndSaveHeightMap(*Heightmap);
    RefreshPreview();
    return FReply::Handled();
}

FReply SHeightMapPanel::OnCreateLandscapeFromInternalClicked()
{
    static const FString Dummy; // GenerateLandscapeFromPNG don't use path
    UGeneratorHeightMapLibrary::GenerateLandscapeFromPNG(Dummy, *Heightmap, *External, *Landscape);
    return FReply::Handled();
}

FReply SHeightMapPanel::OnCreateLandscapeFromPNGClicked()
{
    UGeneratorHeightMapLibrary::OpenHeightmapFileDialog(External, Landscape, Heightmap);
    return FReply::Handled();
}
