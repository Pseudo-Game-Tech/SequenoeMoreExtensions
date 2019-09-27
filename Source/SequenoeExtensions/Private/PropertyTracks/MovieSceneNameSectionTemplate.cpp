#include "PropertyTracks/MovieSceneNameSectionTemplate.h"
#include "Tracks/MovieScenePropertyTrack.h"

FMovieSceneNameSectionTemplate::FMovieSceneNameSectionTemplate(const UMovieSceneNameSection& Section, const UMovieScenePropertyTrack& Track)
	: FMovieScenePropertySectionTemplate(Track.GetPropertyName(), Track.GetPropertyPath())
	, NameCurve(Section.GetChannel())
{}

void FMovieSceneNameSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	const FName* Result = NameCurve.Evaluate(Context.GetTime());
	if (Result)
	{
		ExecutionTokens.Add(TPropertyTrackExecutionToken<FName>(*Result));
	}
}