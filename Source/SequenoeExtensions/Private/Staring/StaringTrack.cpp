#include "Staring/StaringTrack.h"
#include "Staring/StaringSceneSection.h"
#include "Staring/StaringMovieSceneTemplate.h"
#include "Evaluation/MovieSceneEvaluationTrack.h"

#define LOCTEXT_NAMESPACE "StaringTrack"

UMovieSceneSection* UStaringTrack::AddStaring(FFrameNumber Time, int32 Duration, const FMovieSceneObjectBindingID& StaringBindingID)
{
	UStaringSceneSection* NewSection = NewObject<UStaringSceneSection>(this, NAME_None, RF_Transactional);
	NewSection->SetStaringTargetBindingID(StaringBindingID);
	NewSection->InitialPlacement(StaringSections, Time, Duration, SupportsMultipleRows());

	StaringSections.Add(NewSection);

	return NewSection;
}

void UStaringTrack::RemoveAllAnimationData()
{
	// 什么都不做
}

bool UStaringTrack::HasSection(const UMovieSceneSection& Section) const
{
	return StaringSections.Contains(&Section);
}

void UStaringTrack::AddSection(UMovieSceneSection& Section)
{
	StaringSections.Add(&Section);
}

void UStaringTrack::RemoveSection(UMovieSceneSection& Section)
{
	StaringSections.Remove(&Section);
}

bool UStaringTrack::IsEmpty() const
{
	return StaringSections.Num() == 0;
}

const TArray<UMovieSceneSection*>& UStaringTrack::GetAllSections() const
{
	return StaringSections;
}

bool UStaringTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UStaringSceneSection::StaticClass();
}

UMovieSceneSection* UStaringTrack::CreateNewSection()
{
	UStaringSceneSection* NewSection = NewObject<UStaringSceneSection>(this, NAME_None, RF_Transactional);

	StaringSections.Add(NewSection);

	return NewSection;
}

FMovieSceneEvalTemplatePtr UStaringTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FStaringMovieSceneTemplate(*CastChecked<UStaringSceneSection>(&InSection));
}

#if WITH_EDITORONLY_DATA
FText UStaringTrack::GetDisplayName() const
{
	return LOCTEXT("TrackName", "Staring");
}
#endif

#undef LOCTEXT_NAMESPACE
