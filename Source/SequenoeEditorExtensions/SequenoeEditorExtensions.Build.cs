// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class SequenoeEditorExtensions : ModuleRules
{
	public SequenoeEditorExtensions(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {

            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                "SequenoeEditorExtensions/Private/Staring",
                "SequenoeEditorExtensions/Private/LevelSequenceEditorUmg",
                "SequenoeEditorExtensions/Private/SkeletalAnimationRateTrack",
	            "SequenoeEditorExtensions/Private/ActorTickRateTrack",
	            "SequenoeEditorExtensions/Private/PropertyTrackEditors",
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
				// ... add other public dependencies that you statically link with here ...
			}
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Sequencer",
                "CinematicCamera",
                "Slate",
                "SlateCore",
                "ActorPickerMode",
                "UnrealEd",
                "Engine",
                "CurveEditor",
                "SceneOutliner",
                "EditorStyle",
                "EditorWidgets",
                "MovieSceneTracks",
                "MovieSceneTools",
                "MovieScene",
                "LevelEditor",
                "LevelSequence",
                "SequenoeExtensions",
                "UMG",
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
				// ... add any modules that your module loads dynamically here ...
			}
            );
    }
}
