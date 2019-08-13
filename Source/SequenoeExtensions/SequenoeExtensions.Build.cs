// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SequenoeExtensions : ModuleRules
{
	public SequenoeExtensions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
				
		PrivateIncludePaths.AddRange(
			new string[] {
                "SequenoeExtensions/Private/Staring",
                "SequenoeExtensions/Private/LevelSequenceUmg",
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
                "CoreUObject",
				"MovieScene",
				"Engine",
                "UMG",
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CinematicCamera",
                "SlateCore",
                "Slate",
                "LevelSequence",
            }
			);

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[] {
                    "UnrealEd",
                }
            );
        }
    }
}
