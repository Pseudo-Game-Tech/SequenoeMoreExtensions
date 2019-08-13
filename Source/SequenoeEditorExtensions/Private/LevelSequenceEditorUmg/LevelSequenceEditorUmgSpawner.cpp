#include "LevelSequenceEditorUmgSpawner.h"
#include "LevelSequenceUmg/LevelSequenceUmg.h"
#include "ISequencer.h"
#include "Tracks/MovieSceneSpawnTrack.h"
#include "Sections/MovieSceneBoolSection.h"

#define LOCTEXT_NAMESPACE "FLevelSequenceEditorUmgSpawner"

TSharedRef<IMovieSceneObjectSpawner> FLevelSequenceEditorUmgSpawner::CreateObjectSpawner()
{
	return MakeShareable(new FLevelSequenceEditorUmgSpawner);
}

TValueOrError<FNewSpawnable, FText> FLevelSequenceEditorUmgSpawner::CreateNewSpawnableType(UObject& SourceObject, UMovieScene& OwnerMovieScene, UActorFactory* ActorFactory)
{
	FNewSpawnable NewSpawnable(nullptr, FName::NameToDisplayString(SourceObject.GetName(), false));
	const FName TemplateName = MakeUniqueObjectName(&OwnerMovieScene, UObject::StaticClass(), SourceObject.GetFName());

	FText ErrorText;

	if (ULevelSequenceUmg * UserWidget = Cast<ULevelSequenceUmg>(&SourceObject))
	{
		const bool bWasTransactional = UserWidget->HasAnyFlags(RF_Transactional);
		if (!bWasTransactional)
		{
			UserWidget->SetFlags(RF_Transactional);
		}

		ULevelSequenceUmg* SpawnedUserWidget = Cast<ULevelSequenceUmg>(StaticDuplicateObject(UserWidget, &OwnerMovieScene, TemplateName, RF_AllFlags));
		NewSpawnable.ObjectTemplate = SpawnedUserWidget;
		NewSpawnable.Name = UserWidget->GetDisplayLabel();

		if (!bWasTransactional)
		{
			UserWidget->ClearFlags(RF_Transactional);
		}
	}
	//  如果是蓝图对象需要做一些额外处理
	else if (UBlueprint * SourceBlueprint = Cast<UBlueprint>(&SourceObject))
	{
		if (!OwnerMovieScene.GetClass()->IsChildOf(SourceBlueprint->GeneratedClass->ClassWithin))
		{
			ErrorText = FText::Format(LOCTEXT("ClassWithin", "Unable to add spawnable for class of type '{0}' since it has a required outer class '{1}'."), FText::FromString(SourceObject.GetName()), FText::FromString(SourceBlueprint->GeneratedClass->ClassWithin->GetName()));
			return MakeError(ErrorText);
		}

		NewSpawnable.ObjectTemplate = NewObject<UObject>(&OwnerMovieScene, SourceBlueprint->GeneratedClass, TemplateName, RF_Transactional);
	}

	if (!NewSpawnable.ObjectTemplate || !NewSpawnable.ObjectTemplate->IsA<ULevelSequenceUmg>())
	{
		if (UClass * InClass = Cast<UClass>(&SourceObject))
		{
			if (!InClass->IsChildOf(ULevelSequenceUmg::StaticClass()))
			{
				ErrorText = FText::Format(LOCTEXT("NotAnActorClass", "Unable to add spawnable for class of type '{0}' since it is not a valid actor class."), FText::FromString(InClass->GetName()));
				return MakeError(ErrorText);
			}

			NewSpawnable.ObjectTemplate = NewObject<UObject>(&OwnerMovieScene, InClass, TemplateName, RF_Transactional);
		}

		if (!NewSpawnable.ObjectTemplate || !NewSpawnable.ObjectTemplate->IsA<ULevelSequenceUmg>())
		{
			if (ErrorText.IsEmpty())
			{
				ErrorText = FText::Format(LOCTEXT("UnknownClassError", "Unable to create a new spawnable object from {0}."), FText::FromString(SourceObject.GetName()));
			}

			return MakeError(ErrorText);
		}
	}

	return MakeValue(NewSpawnable);
}

void FLevelSequenceEditorUmgSpawner::SetupDefaultsForSpawnable(UObject* SpawnedObject, const FGuid& Guid, const TOptional<FTransformData>& TransformData, TSharedRef<ISequencer> Sequencer, USequencerSettings* Settings)
{
	ULevelSequenceUmg* SpawnedUserWidget = Cast<ULevelSequenceUmg>(SpawnedObject);
	if (SpawnedUserWidget)
	{

	}

	UMovieSceneSequence* Sequence = Sequencer->GetFocusedMovieSceneSequence();
	UMovieScene* OwnerMovieScene = Sequence->GetMovieScene();

	// 添加SpawnTrack
	{
		UMovieSceneSpawnTrack* SpawnTrack = Cast<UMovieSceneSpawnTrack>(OwnerMovieScene->FindTrack(UMovieSceneSpawnTrack::StaticClass(), Guid, NAME_None));
		if (!SpawnTrack)
		{
			SpawnTrack = Cast<UMovieSceneSpawnTrack>(OwnerMovieScene->AddTrack(UMovieSceneSpawnTrack::StaticClass(), Guid));
		}

		if (SpawnTrack)
		{
			UMovieSceneBoolSection* SpawnSection = Cast<UMovieSceneBoolSection>(SpawnTrack->CreateNewSection());
			SpawnSection->GetChannel().SetDefault(true);
			if (Sequencer->GetInfiniteKeyAreas())
			{
				SpawnSection->SetRange(TRange<FFrameNumber>::All());
			}
			SpawnTrack->AddSection(*SpawnSection);
			SpawnTrack->SetObjectId(Guid);
		}
	}
}

bool FLevelSequenceEditorUmgSpawner::CanSetupDefaultsForSpawnable(UObject* SpawnedObject) const
{
	if (SpawnedObject == nullptr)
	{
		return true;
	}

	return FLevelSequenceUmgSpawner::CanSetupDefaultsForSpawnable(SpawnedObject);
}

#undef LOCTEXT_NAMESPACE