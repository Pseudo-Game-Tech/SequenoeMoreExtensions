// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ActorTickRateTrack.h"
#include "ActorTickRateSection.h"
#include "ActorTickRateSectionTemplate.h"
#include "Evaluation/MovieSceneEvaluationTrack.h"
#include "Templates/Casts.h"

#define LOCTEXT_NAMESPACE "UActorTickRateTrack"


/* UMovieSceneEventTrack interface
 *****************************************************************************/
UActorTickRateTrack::UActorTickRateTrack(const FObjectInitializer& Init)
	: Super(Init)
{
	SupportedBlendTypes = FMovieSceneBlendTypeField::All();

#if WITH_EDITORONLY_DATA
	TrackTint = FColor(65, 173, 164, 65);
#endif

	EvalOptions.bEvaluateNearestSection_DEPRECATED = EvalOptions.bCanEvaluateNearestSection = true;
}

bool UActorTickRateTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UActorTickRateSection::StaticClass();
}

UMovieSceneSection* UActorTickRateTrack::CreateNewSection()
{
	return NewObject<UActorTickRateSection>(this, NAME_None, RF_Transactional);
}

FMovieSceneEvalTemplatePtr UActorTickRateTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FActorTickRateSectionTemplate(*CastChecked<UActorTickRateSection>(&InSection));
}

#undef LOCTEXT_NAMESPACE
