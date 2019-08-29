// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SkeletalAnimationRateTrack/MovieSceneSkeletalAnimationRateTrack.h"
#include "Evaluation/MovieSceneEvaluationCustomVersion.h"
#include "SkeletalAnimationRateTrack/MovieSceneSkeletalAnimationRateSection.h"
#include "Compilation/MovieSceneCompilerRules.h"
#include "Evaluation/MovieSceneEvaluationTrack.h"
#include "SkeletalAnimationRateTrack/MovieSceneSkeletalAnimationRateTemplate.h"
#include "Compilation/IMovieSceneTemplateGenerator.h"
#include "MovieScene.h"

#define LOCTEXT_NAMESPACE "MovieSceneSkeletalAnimationRateTrack"


/* UMovieSceneSkeletalAnimationRateTrack structors
 *****************************************************************************/

UMovieSceneSkeletalAnimationRateTrack::UMovieSceneSkeletalAnimationRateTrack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bUseLegacySectionIndexBlend(false)
{
#if WITH_EDITORONLY_DATA
	TrackTint = FColor(124, 15, 124, 65);
	bSupportsDefaultSections = false;
#endif

	SupportedBlendTypes.Add(EMovieSceneBlendType::Absolute);

	EvalOptions.bEvaluateNearestSection_DEPRECATED = EvalOptions.bCanEvaluateNearestSection = true;
}


/* UMovieSceneSkeletalAnimationRateTrack interface
 *****************************************************************************/

UMovieSceneSection* UMovieSceneSkeletalAnimationRateTrack::AddNewAnimationOnRow(FFrameNumber KeyTime, UAnimSequenceBase* AnimSequence, int32 RowIndex)
{
	UMovieSceneSkeletalAnimationRateSection* NewSection = Cast<UMovieSceneSkeletalAnimationRateSection>(CreateNewSection());
	{
		FFrameTime AnimationLength = AnimSequence->SequenceLength * GetTypedOuter<UMovieScene>()->GetTickResolution();
		NewSection->InitialPlacementOnRow(AnimationSections, KeyTime, AnimationLength.FrameNumber.Value, RowIndex);
		NewSection->Params.Animation = AnimSequence;
	}

	AddSection(*NewSection);

	return NewSection;
}


TArray<UMovieSceneSection*> UMovieSceneSkeletalAnimationRateTrack::GetAnimSectionsAtTime(FFrameNumber Time)
{
	TArray<UMovieSceneSection*> Sections;
	for (auto Section : AnimationSections)
	{
		if (Section->IsTimeWithinSection(Time))
		{
			Sections.Add(Section);
		}
	}

	return Sections;
}


/* UMovieSceneTrack interface
 *****************************************************************************/

void UMovieSceneSkeletalAnimationRateTrack::PostLoad()
{
	Super::PostLoad();

	if (GetLinkerCustomVersion(FMovieSceneEvaluationCustomVersion::GUID) < FMovieSceneEvaluationCustomVersion::AddBlendingSupport)
	{
		bUseLegacySectionIndexBlend = true;
	}
}

const TArray<UMovieSceneSection*>& UMovieSceneSkeletalAnimationRateTrack::GetAllSections() const
{
	return AnimationSections;
}


bool UMovieSceneSkeletalAnimationRateTrack::SupportsMultipleRows() const
{
	return true;
}

bool UMovieSceneSkeletalAnimationRateTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneSkeletalAnimationRateSection::StaticClass();
}

UMovieSceneSection* UMovieSceneSkeletalAnimationRateTrack::CreateNewSection()
{
	return NewObject<UMovieSceneSkeletalAnimationRateSection>(this, NAME_None, RF_Transactional);
}


void UMovieSceneSkeletalAnimationRateTrack::RemoveAllAnimationData()
{
	AnimationSections.Empty();
}


bool UMovieSceneSkeletalAnimationRateTrack::HasSection(const UMovieSceneSection& Section) const
{
	return AnimationSections.Contains(&Section);
}


void UMovieSceneSkeletalAnimationRateTrack::AddSection(UMovieSceneSection& Section)
{
	AnimationSections.Add(&Section);
}


void UMovieSceneSkeletalAnimationRateTrack::RemoveSection(UMovieSceneSection& Section)
{
	AnimationSections.Remove(&Section);
}


bool UMovieSceneSkeletalAnimationRateTrack::IsEmpty() const
{
	return AnimationSections.Num() == 0;
}

#if WITH_EDITORONLY_DATA

FText UMovieSceneSkeletalAnimationRateTrack::GetDefaultDisplayName() const
{
	return LOCTEXT("TrackName", "Animation");
}

#endif

FMovieSceneTrackRowSegmentBlenderPtr UMovieSceneSkeletalAnimationRateTrack::GetRowSegmentBlender() const
{
	// Apply an upper bound exclusive blend
	struct FSkeletalAnimationRowCompilerRules : FMovieSceneTrackRowSegmentBlender
	{
		bool bUseLegacySectionIndexBlend;
		FSkeletalAnimationRowCompilerRules(bool bInUseLegacySectionIndexBlend) : bUseLegacySectionIndexBlend(bInUseLegacySectionIndexBlend) {}

		virtual void Blend(FSegmentBlendData& BlendData) const override
		{
			// Run the default high pass filter for overlap priority
			MovieSceneSegmentCompiler::FilterOutUnderlappingSections(BlendData);

			if (bUseLegacySectionIndexBlend)
			{
				// Weed out based on array index (legacy behaviour)
				MovieSceneSegmentCompiler::BlendSegmentLegacySectionOrder(BlendData);
			}
		}
	};
	return FSkeletalAnimationRowCompilerRules(bUseLegacySectionIndexBlend);
}

#undef LOCTEXT_NAMESPACE
