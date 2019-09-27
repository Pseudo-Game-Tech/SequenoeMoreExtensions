#include "PropertyTracks/MovieSceneTextSectionTemplate.h"
#include "Tracks/MovieScenePropertyTrack.h"

FMovieSceneTextSectionTemplate::FMovieSceneTextSectionTemplate(const UMovieSceneTextSection& Section, const UMovieScenePropertyTrack& Track)
	: FMovieScenePropertySectionTemplate(Track.GetPropertyName(), Track.GetPropertyPath())
	, TextCurve(Section.GetChannel())
{}

void FMovieSceneTextSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	const FText* Result = TextCurve.Evaluate(Context.GetTime());
	if (Result)
	{
		ExecutionTokens.Add(TPropertyTrackExecutionToken<FText>(*Result));
	}
}