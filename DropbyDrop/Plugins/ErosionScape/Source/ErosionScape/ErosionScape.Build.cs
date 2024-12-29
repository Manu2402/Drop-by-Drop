// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;


public class ErosionScape : ModuleRules
{
	public ErosionScape(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject", 
				"Engine", 
				"InputCore", 
				"RenderCore",
				"RHI",
				"UnrealEd",
				"EditorScriptingUtilities",
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore", 
				"LandscapeEditor",
				"Landscape",
				
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		// Add FBX only if the target is the editor
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("FBX");
		}
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
