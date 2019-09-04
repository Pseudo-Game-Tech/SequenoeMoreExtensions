// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ActorTickRateEditorTrack.h"
#include "GameFramework/Actor.h"
#include "ActorTickRateTrack.h"
#include "MovieSceneToolHelpers.h"

#define LOCTEXT_NAMESPACE "FActorTickRateEditorTrack"


/* FActorTickRateEditorTrack static functions
 *****************************************************************************/

TSharedRef<ISequencerTrackEditor> FActorTickRateEditorTrack::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FActorTickRateEditorTrack(InSequencer));
}


/* FActorTickRateEditorTrack structors
 *****************************************************************************/

FActorTickRateEditorTrack::FActorTickRateEditorTrack(TSharedRef<ISequencer> InSequencer)
	: FKeyframeTrackEditor<UActorTickRateTrack>(InSequencer)
{ }


/* ISequencerTrackEditor interface
 *****************************************************************************/

void FActorTickRateEditorTrack::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass)
{
	if (ObjectClass != nullptr && ObjectClass->IsChildOf(AActor::StaticClass()))
	{
		MenuBuilder.AddMenuEntry(
			NSLOCTEXT("ActorTickRate", "AddActorTickRate", "ActorTickRate"),
			NSLOCTEXT("ActorTickRate", "AddActorTickRateTooltip", "Adds a ActorTickRate track."),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &FActorTickRateEditorTrack::AddActorTickRateTrack, ObjectBinding)
			)
		);
	}
}

bool FActorTickRateEditorTrack::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UActorTickRateTrack::StaticClass();
}

void FActorTickRateEditorTrack::AddActorTickRateTrack(FGuid ObjectBinding)
{
	if (!GetSequencer()->IsAllowedToChange())
	{
		return;
	}

	auto Objects = GetSequencer()->FindObjectsInCurrentSequence(ObjectBinding);
	if (Objects.Num() > 0 && Objects[0].IsValid())
	{
		AActor* Actor = Cast<AActor>(Objects[0].Get());
		if (Actor)
		{
			TSharedRef<FGeneratedTrackKeys> GeneratedKeys = MakeShared<FGeneratedTrackKeys>();
			GeneratedKeys.Get().Add(FMovieSceneChannelValueSetter::Create<FMovieSceneFloatChannel>(0, Actor->CustomTimeDilation, true));

			static FName ActorTickRateName(TEXT("ActorTickRate"));
			auto InitializeNewTrack = [ObjectBinding, SequenceID = GetSequencer()->GetFocusedTemplateID()](UActorTickRateTrack* NewTrack)
			{
				NewTrack->SetPropertyNameAndPath(ActorTickRateName, ActorTickRateName.ToString());
			};

			auto OnKeyProperty = [=](FFrameNumber Time) -> FKeyPropertyResult
			{
				return this->AddKeysToObjects({ Objects[0].Get() }, Time, *GeneratedKeys, ESequencerKeyMode::ManualKey, UActorTickRateTrack::StaticClass(), ActorTickRateName, InitializeNewTrack);
			};

			AnimatablePropertyChanged(FOnKeyProperty::CreateLambda(OnKeyProperty));
		}
	}
}
#undef LOCTEXT_NAMESPACE
