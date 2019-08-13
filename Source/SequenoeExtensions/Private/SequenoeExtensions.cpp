// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SequenoeExtensions.h"
#include "ILevelSequenceModule.h"
#include "LevelSequenceUmg/LevelSequenceUmgSpawner.h"

#define LOCTEXT_NAMESPACE "FSequenoeExtensionsModule"

void FSequenoeExtensionsModule::StartupModule()
{
	ILevelSequenceModule& LevelSequenceModule = FModuleManager::LoadModuleChecked<ILevelSequenceModule>("LevelSequence");
	LevelSequenceUmgSpawnerDelegateHandle = LevelSequenceModule.RegisterObjectSpawner(FOnCreateMovieSceneObjectSpawner::CreateStatic(&FLevelSequenceUmgSpawner::CreateObjectSpawner));
}

void FSequenoeExtensionsModule::ShutdownModule()
{
	ILevelSequenceModule& LevelSequenceModule = FModuleManager::LoadModuleChecked<ILevelSequenceModule>("LevelSequence");
	LevelSequenceModule.UnregisterObjectSpawner(LevelSequenceUmgSpawnerDelegateHandle);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSequenoeExtensionsModule, SequenoeExtensions)