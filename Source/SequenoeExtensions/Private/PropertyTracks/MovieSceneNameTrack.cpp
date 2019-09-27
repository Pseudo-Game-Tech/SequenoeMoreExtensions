#include "PropertyTracks/MovieSceneNameTrack.h"
#include "MovieSceneCommonHelpers.h"
#include "PropertyTracks/MovieSceneNameSection.h"
#include "PropertyTracks/MovieSceneNameSectionTemplate.h"


#define LOCTEXT_NAMESPACE "MovieSceneNameTrack"


/* UMovieSceneTrack interface
 *****************************************************************************/

void UMovieSceneNameTrack::AddSection(UMovieSceneSection& Section)
{
	Sections.Add(&Section);
}


bool UMovieSceneNameTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneNameSection::StaticClass();
}

UMovieSceneSection* UMovieSceneNameTrack::CreateNewSection()
{
	return NewObject<UMovieSceneNameSection>(this, NAME_None, RF_Transactional);
}


FMovieSceneEvalTemplatePtr UMovieSceneNameTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FMovieSceneNameSectionTemplate(*CastChecked<UMovieSceneNameSection>(&InSection), *this);
}

const TArray<UMovieSceneSection*>& UMovieSceneNameTrack::GetAllSections() const
{
	return Sections;
}


bool UMovieSceneNameTrack::HasSection(const UMovieSceneSection& Section) const
{
	return Sections.Contains(&Section);
}


bool UMovieSceneNameTrack::IsEmpty() const
{
	return (Sections.Num() == 0);
}


void UMovieSceneNameTrack::RemoveAllAnimationData()
{
	Sections.Empty();
}


void UMovieSceneNameTrack::RemoveSection(UMovieSceneSection& Section)
{
	Sections.Remove(&Section);
}


#if WITH_EDITORONLY_DATA

FText UMovieSceneNameTrack::GetDefaultDisplayName() const
{
	return LOCTEXT("TrackName", "Names");
}

#endif


#undef LOCTEXT_NAMESPACE
