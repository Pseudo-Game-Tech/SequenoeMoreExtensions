// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SequenoeEditorExtensions.h"
#include "ISequencerModule.h"
#include "StaringTrackEditor.h"
#include "ILevelSequenceModule.h"
#include "LevelSequenceEditorUmgSpawner.h"

#define LOCTEXT_NAMESPACE "FSequenoeEditorExtensionsModule"

void FSequenoeEditorExtensionsModule::StartupModule()
{
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	StaringTrackEditorDelegateHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FStaringTrackEditor::CreateTrackEditor));

	ILevelSequenceModule& LevelSequenceModule = FModuleManager::LoadModuleChecked<ILevelSequenceModule>("LevelSequence");
	LevelSequenceEditorUmgSpawnerDelegateHandle = LevelSequenceModule.RegisterObjectSpawner(FOnCreateMovieSceneObjectSpawner::CreateStatic(&FLevelSequenceEditorUmgSpawner::CreateObjectSpawner));
}

void FSequenoeEditorExtensionsModule::ShutdownModule()
{
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.UnRegisterTrackEditor(StaringTrackEditorDelegateHandle);

	ILevelSequenceModule& LevelSequenceModule = FModuleManager::LoadModuleChecked<ILevelSequenceModule>("LevelSequence");
	LevelSequenceModule.UnregisterObjectSpawner(LevelSequenceEditorUmgSpawnerDelegateHandle);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSequenoeEditorExtensionsModule, SequenoeEditorExtensions)