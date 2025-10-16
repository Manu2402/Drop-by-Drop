// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**
 * FDropByDropStyle
 *
 * Manages the visual styling and UI resources for the "DropByDrop" plugin.
 * This class provides a centralized style set containing icons, brushes,
 * colors, and other visual assets used throughout the plugin's editor interface.
 *
 * Uses a singleton pattern to ensure a single style instance is shared
 * across all UI elements in the plugin.
 */
class FDropByDropStyle
{
public:
	/**
	 * Initializes the style set and registers it with Slate's style registry.
	 * Should be called once during plugin startup (typically in "StartupModule").
	 * Safe to call multiple times: will only initialize once.
	 */
	static void Initialize();

	/**
	 * Unregisters the style set from Slate and releases resources.
	 * Should be called during plugin shutdown (typically in "ShutdownModule").
	 * Ensures the style instance is properly cleaned up.
	 */
	static void Shutdown();

	/**
	 * Retrieves the active style set interface for accessing style resources.
	 * Use this to look up brushes, icons, and other visual assets by name.
	 *
	 * @return Reference to the ISlateStyle interface for the plugin.
	 */
	static const ISlateStyle& Get();

	/**
	 * Forces a reload of all texture resources in the style set.
	 * Useful during development when updating plugin icons or images.
	 * Only works when the Slate application is initialized.
	 */
	static void ReloadTextures();

	/**
	 * Returns the unique identifier name for this style set.
	 * Used for registration and lookup in the Slate style registry.
	 *
	 * @return FName containing "DropByDropStyle".
	 */
	static FName GetStyleSetName();

private:
	/**
	 * Factory method that creates and configures the FSlateStyleSet instance.
	 * Sets up the content root directory and registers all style resources
	 * (icons, brushes, etc.) used by the plugin.
	 *
	 * @return Shared reference to the newly created style set.
	 */
	static TSharedRef<class FSlateStyleSet> Create();

	/**
	 * Shared pointer holding the singleton style set instance.
	 * Null until Initialize() is called, and reset to null on Shutdown().
	 */
	static TSharedPtr<class FSlateStyleSet> StyleInstance;

};