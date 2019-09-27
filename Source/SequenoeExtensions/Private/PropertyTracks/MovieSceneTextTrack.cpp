#include "PropertyTracks/MovieSceneTextTrack.h"
#include "MovieSceneCommonHelpers.h"
#include "PropertyTracks/MovieSceneTextSection.h"
#include "PropertyTracks/MovieSceneTextSectionTemplate.h"


#define LOCTEXT_NAMESPACE "MovieSceneTextTrack"


/* UMovieSceneTrack interface
 *****************************************************************************/

void UMovieSceneTextTrack::AddSection(UMovieSceneSection& Section)
{
	Sections.Add(&Section);
}


bool UMovieSceneTextTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass == UMovieSceneTextSection::StaticClass();
}

UMovieSceneSection* UMovieSceneTextTrack::CreateNewSection()
{
	return NewObject<UMovieSceneTextSection>(this, NAME_None, RF_Transactional);
}


FMovieSceneEvalTemplatePtr UMovieSceneTextTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	return FMovieSceneTextSectionTemplate(*CastChecked<UMovieSceneTextSection>(&InSection), *this);
}

const TArray<UMovieSceneSection*>& UMovieSceneTextTrack::GetAllSections() const
{
	return Sections;
}


bool UMovieSceneTextTrack::HasSection(const UMovieSceneSection& Section) const
{
	return Sections.Contains(&Section);
}


bool UMovieSceneTextTrack::IsEmpty() const
{
	return (Sections.Num() == 0);
}


void UMovieSceneTextTrack::RemoveAllAnimationData()
{
	Sections.Empty();
}


void UMovieSceneTextTrack::RemoveSection(UMovieSceneSection& Section)
{
	Sections.Remove(&Section);
}


#if WITH_EDITORONLY_DATA

FText UMovieSceneTextTrack::GetDefaultDisplayName() const
{
	return LOCTEXT("TrackName", "Texts");
}

#endif


#undef LOCTEXT_NAMESPACE


