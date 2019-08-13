#pragma once

#include "IMovieSceneObjectSpawner.h"

class SEQUENOEEXTENSIONS_API FLevelSequenceUmgSpawner : public IMovieSceneObjectSpawner
{
public:

	static TSharedRef<IMovieSceneObjectSpawner> CreateObjectSpawner();

	// IMovieSceneObjectSpawner interface
	virtual UClass* GetSupportedTemplateType() const override;
	virtual UObject* SpawnObject(FMovieSceneSpawnable& Spawnable, FMovieSceneSequenceIDRef TemplateID, IMovieScenePlayer& Player) override;
	virtual void DestroySpawnedObject(UObject& Object) override;
};