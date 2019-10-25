// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SequenoeEditorExtensions.h"
#include "SequencerChannelInterface.h"
#include "ISequencerModule.h"
#include "StaringTrackEditor.h"
#include "ILevelSequenceModule.h"
#include "LevelSequenceEditorUmgSpawner.h"
#include "SkeletalAnimationRateTrackEditor.h"
#include "PropertyTrackEditors/TextPropertyTrackEditor.h"
#include "PropertyTrackEditors/NamePropertyTrackEditor.h"
#include "UObject/TextProperty.h"
#include "ActorTickRateEditorTrack.h"
#include "ChannelEditor/MovieSceneChannelEditors.h"

#define LOCTEXT_NAMESPACE "FSequenoeEditorExtensionsModule"

void FSequenoeEditorExtensionsModule::StartupModule()
{
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	StaringTrackEditorDelegateHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FStaringTrackEditor::CreateTrackEditor));
	SkeletalAnimationRateTrackEditorDelegateHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FSkeletalAnimationRateTrackEditor::CreateTrackEditor));
	ActorTickRateEditorTrackDelegateHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FActorTickRateEditorTrack::CreateTrackEditor));
	TextEditorTrackEditorDelegateHandle = SequencerModule.RegisterPropertyTrackEditor<FTextPropertyTrackEditor>();
	NameEditorTrackEditorDelegateHandle = SequencerModule.RegisterPropertyTrackEditor<FNamePropertyTrackEditor>();
	SequencerModule.RegisterChannelInterface<FMovieSceneNameChannel>(MakeUnique<MovieSceneNameChannelEditor>());
	SequencerModule.RegisterChannelInterface<FMovieSceneTextChannel>(MakeUnique<MovieSceneTextChannelEditor>());

	ILevelSequenceModule& LevelSequenceModule = FModuleManager::LoadModuleChecked<ILevelSequenceModule>("LevelSequence");
	LevelSequenceEditorUmgSpawnerDelegateHandle = LevelSequenceModule.RegisterObjectSpawner(FOnCreateMovieSceneObjectSpawner::CreateStatic(&FLevelSequenceEditorUmgSpawner::CreateObjectSpawner));
}

void FSequenoeEditorExtensionsModule::ShutdownModule()
{
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.UnRegisterTrackEditor(StaringTrackEditorDelegateHandle);
	SequencerModule.UnRegisterTrackEditor(SkeletalAnimationRateTrackEditorDelegateHandle);
	SequencerModule.UnRegisterTrackEditor(ActorTickRateEditorTrackDelegateHandle);
	SequencerModule.UnRegisterTrackEditor(TextEditorTrackEditorDelegateHandle);
	SequencerModule.UnRegisterTrackEditor(NameEditorTrackEditorDelegateHandle);

	ILevelSequenceModule& LevelSequenceModule = FModuleManager::LoadModuleChecked<ILevelSequenceModule>("LevelSequence");
	LevelSequenceModule.UnregisterObjectSpawner(LevelSequenceEditorUmgSpawnerDelegateHandle);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSequenoeEditorExtensionsModule, SequenoeEditorExtensions)