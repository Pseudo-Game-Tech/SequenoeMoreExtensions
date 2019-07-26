// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "TrackEditorExtensions.h"
#include "ISequencerModule.h"
#include "StaringTrackEditor.h"

#define LOCTEXT_NAMESPACE "FTrackEditorExtensionsModule"

void FTrackEditorExtensionsModule::StartupModule()
{
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FStaringTrackEditor::CreateTrackEditor));
}

void FTrackEditorExtensionsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FTrackEditorExtensionsModule, TrackEditorExtensions)