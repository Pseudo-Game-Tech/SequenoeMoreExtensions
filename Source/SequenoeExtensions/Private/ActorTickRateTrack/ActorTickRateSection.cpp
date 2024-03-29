// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ActorTickRateSection.h"
#include "UObject/SequencerObjectVersion.h"


/* UActorTickRateTrack structors
 *****************************************************************************/

UActorTickRateSection::UActorTickRateSection()
	: UMovieSceneFloatSection()
{
#if WITH_EDITORONLY_DATA
	bIsInfinite_DEPRECATED = true;
#endif

	bSupportsInfiniteRange = true;
	SetRange(TRange<FFrameNumber>::All());
	FloatCurve.SetDefault(1.f);
	EvalOptions.EnableAndSetCompletionMode
	(GetLinkerCustomVersion(FSequencerObjectVersion::GUID) < FSequencerObjectVersion::WhenFinishedDefaultsToProjectDefault ?
		EMovieSceneCompletionMode::RestoreState :
		EMovieSceneCompletionMode::ProjectDefault);
}
