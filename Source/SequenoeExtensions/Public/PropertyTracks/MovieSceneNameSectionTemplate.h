#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "Evaluation/MovieScenePropertyTemplate.h"
#include "PropertyTracks/MovieSceneNameSection.h"
#include "Channels/MovieSceneNameChannel.h"
#include "MovieSceneNameSectionTemplate.generated.h"


USTRUCT()
struct FMovieSceneNameSectionTemplate : public FMovieScenePropertySectionTemplate
{
	GENERATED_BODY()

		FMovieSceneNameSectionTemplate() {}
	FMovieSceneNameSectionTemplate(const UMovieSceneNameSection& Section, const UMovieScenePropertyTrack& Track);

protected:

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void SetupOverrides() override { EnableOverrides(RequiresSetupFlag); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	UPROPERTY()
		FMovieSceneNameChannel NameCurve;
};

