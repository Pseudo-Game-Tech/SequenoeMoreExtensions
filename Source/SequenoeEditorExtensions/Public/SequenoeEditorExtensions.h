// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"



class FSequenoeEditorExtensionsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	FDelegateHandle StaringTrackEditorDelegateHandle;
	FDelegateHandle SkeletalAnimationRateTrackEditorDelegateHandle;
	FDelegateHandle ActorTickRateEditorTrackDelegateHandle;
	FDelegateHandle TextEditorTrackEditorDelegateHandle;
	FDelegateHandle NameEditorTrackEditorDelegateHandle;

	FDelegateHandle LevelSequenceEditorUmgSpawnerDelegateHandle;
};
