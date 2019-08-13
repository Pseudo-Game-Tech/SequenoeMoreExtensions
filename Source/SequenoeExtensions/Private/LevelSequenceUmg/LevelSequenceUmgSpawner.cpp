#include "LevelSequenceUmg/LevelSequenceUmgSpawner.h"
#include "MovieSceneSpawnable.h"
#include "IMovieScenePlayer.h"
#include "LevelSequenceUmg/LevelSequenceUmg.h"

TSharedRef<IMovieSceneObjectSpawner> FLevelSequenceUmgSpawner::CreateObjectSpawner()
{
	return MakeShareable(new FLevelSequenceUmgSpawner);
}

UClass* FLevelSequenceUmgSpawner::GetSupportedTemplateType() const
{
	return ULevelSequenceUmg::StaticClass();
}

UObject* FLevelSequenceUmgSpawner::SpawnObject(FMovieSceneSpawnable& Spawnable, FMovieSceneSequenceIDRef TemplateID, IMovieScenePlayer& Player)
{
	ULevelSequenceUmg* ObjectTemplate = Cast<ULevelSequenceUmg>(Spawnable.GetObjectTemplate());
	if (!ObjectTemplate)
	{
		return nullptr;
	}

	const EObjectFlags ObjectFlags = RF_Transient;

	// @todo sequencer livecapture: Consider using SetPlayInEditorWorld() and RestoreEditorWorld() here instead

	// @todo sequencer UserWidgets: We need to make sure puppet objects aren't copied into PIE/SIE sessions!  They should be omitted from that duplication!

	UWorld* WorldContext = Cast<UWorld>(Player.GetPlaybackContext());
	if (WorldContext == nullptr)
	{
		WorldContext = GWorld;
	}

	// Construct the object with the same name that we will set later on the UserWidget to avoid renaming it inside SetUserWidgetLabel
	FName SpawnName =
#if WITH_EDITOR
		MakeUniqueObjectName(WorldContext->PersistentLevel, ObjectTemplate->GetClass(), *Spawnable.GetName());
#else
		NAME_None;
#endif

	ULevelSequenceUmg* SpawnedUserWidget = CreateWidget<ULevelSequenceUmg>(WorldContext, ObjectTemplate->GetClass());
	if (!SpawnedUserWidget)
	{
		return nullptr;
	}

#if WITH_EDITOR
	if (GIsEditor)
	{
		// Explicitly set RF_Transactional on spawned UserWidgets so we can undo/redo properties on them. We don't add this as a spawn flag since we don't want to transact spawn/destroy events.
		SpawnedUserWidget->SetFlags(RF_Transactional);
	}
#endif

	if (WorldContext->IsGameWorld())
	{
		SpawnedUserWidget->AddToViewport();
	}
	else if (WorldContext->WorldType == EWorldType::Editor)
	{
#if WITH_EDITOR
		SpawnedUserWidget->AddToEditorViewport();
#endif
	}
	else
	{
		return nullptr;
	}
	
	return SpawnedUserWidget;
}

void FLevelSequenceUmgSpawner::DestroySpawnedObject(UObject& Object)
{
	ULevelSequenceUmg* UserWidget = Cast<ULevelSequenceUmg>(&Object);
	if (!ensure(UserWidget))
	{
		return;
	}

#if WITH_EDITOR
	if (GIsEditor)
	{
		// Explicitly remove RF_Transactional on spawned UserWidgets since we don't want to trasact spawn/destroy events
		UserWidget->ClearFlags(RF_Transactional);
	}
#endif

	UWorld* World = UserWidget->GetWorld();
	if (ensure(World))
	{
		UserWidget->RemoveFromParent();
	}
}
