// Copyright Epic Games, Inc. All Rights Reserved.

#include "DropByDropStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

// Macro for convenient content directory path construction.
#define RootToContentDir Style->RootToContentDir

// Initialize the static style instance pointer to null.
TSharedPtr<FSlateStyleSet> FDropByDropStyle::StyleInstance = nullptr;

/**
 * Creates the style set instance if it doesn't already exist and registers
 * it with Slate's global style registry. This makes the style accessible
 * throughout the editor via GetStyleSetName().
 *
 * Thread-safe check ensures only one initialization occurs.
 */
void FDropByDropStyle::Initialize()
{
	// Only create and register if not already initialized.
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

/**
 * Unregisters the style set from Slate's registry and releases the shared pointer.
 * The ensure() check verifies that no other code is holding references to the
 * style instance, which would indicate a potential memory leak.
 */
void FDropByDropStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique()); // Verify no dangling references.
	StyleInstance.Reset(); // Release the shared pointer.
}

/**
 * Returns the unique name identifier for this style set.
 * Using a static local variable ensures the FName is constructed only once
 * and remains valid for the lifetime of the program.
 */
FName FDropByDropStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("DropByDropStyle"));
	return StyleSetName;
}

// Standard icon size definitions for consistent UI scaling.
const FVector2D Icon16x16(16.0f, 16.0f); // Small icon size.
const FVector2D Icon20x20(20.0f, 20.0f); // Medium icon size (toolbar buttons).

/**
 * Factory method that constructs the FSlateStyleSet and populates it with
 * all visual resources needed by the plugin. This includes:
 * - Setting the root directory where resource files are located.
 * - Registering icon brushes for UI elements.
 * - Defining any custom styles, colors, or fonts.
 */
TSharedRef<FSlateStyleSet> FDropByDropStyle::Create()
{
	// Create a new style set with the plugin's style name.
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("DropByDropStyle"));

	// Set the content root to the plugin's Resources folder.
	// This is where all icons, images, and other assets are stored.
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("DropByDrop")->GetBaseDir() / TEXT("Resources"));

	// Register the icon for the "Open Plugin Window" command.
	// IMAGE_BRUSH_SVG macro creates an SVG brush from the specified file.
	// The brush is accessible via the key "DropByDrop.OpenPluginWindow".
	Style->Set("DropByDrop.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

/**
 * Forces the Slate renderer to reload all texture resources.
 * Useful during development when you modify icon files and want to see
 * changes without restarting the editor.
 *
 * Only performs the reload if the Slate application is fully initialized
 * to avoid crashes during startup/shutdown.
 */
void FDropByDropStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

/**
 * Accessor method that returns the style set interface.
 * This is the primary way other code retrieves style resources like brushes,
 * icons, fonts, and colors defined in this style set.
 */
const ISlateStyle& FDropByDropStyle::Get()
{
	return *StyleInstance;
}

// Clean up the temporary macro definition.
#undef RootToContentDir