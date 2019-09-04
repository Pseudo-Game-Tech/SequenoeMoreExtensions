// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "Evaluation/MovieSceneTrackImplementation.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "ActorTickRateSection.h"

#include "ActorTickRateSectionTemplate.generated.h"

USTRUCT()
struct FActorTickRateSectionTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()

		FActorTickRateSectionTemplate() {}
	FActorTickRateSectionTemplate(const UActorTickRateSection& Section);

private:

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	UPROPERTY()
	FMovieSceneFloatChannel ActorTickRateCurve;
};
