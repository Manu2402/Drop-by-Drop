// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DropbyDrop : ModuleRules
{
	public DropbyDrop(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RHI", "RenderCore", "ProceduralMeshComponent", "Landscape", "EditorScriptingUtilities" });

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate",
			"SlateCore",
			"EditorStyle",
			"UnrealEd", // Importante per l'accesso ai componenti dell'editor
			"Landscape", "LandscapeEditor"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
