#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Evaluation/MovieScenePropertyTemplate.h"
#include "PropertyTracks/MovieSceneTextSection.h"
#include "Channels/MovieSceneTextChannel.h"
#include "MovieSceneTextSectionTemplate.generated.h"


USTRUCT()
struct FMovieSceneTextSectionTemplate : public FMovieScenePropertySectionTemplate
{
	GENERATED_BODY()

		FMovieSceneTextSectionTemplate() {}
	FMovieSceneTextSectionTemplate(const UMovieSceneTextSection& Section, const UMovieScenePropertyTrack& Track);

protected:

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void SetupOverrides() override { EnableOverrides(RequiresSetupFlag); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	UPROPERTY()
		FMovieSceneTextChannel TextCurve;
};

