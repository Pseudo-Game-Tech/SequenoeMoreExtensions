// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "SkeletalAnimationRateTrack/MovieSceneSkeletalAnimationRateSection.h"
#include "MovieSceneSkeletalAnimationRateTemplate.generated.h"

USTRUCT()
struct FMovieSceneSkeletalAnimationRateSectionTemplateParameters : public FMovieSceneSkeletalAnimationRateParams
{
	GENERATED_BODY()

		FMovieSceneSkeletalAnimationRateSectionTemplateParameters() {}
	FMovieSceneSkeletalAnimationRateSectionTemplateParameters(const FMovieSceneSkeletalAnimationRateParams& BaseParams, FFrameNumber InSectionStartTime, FFrameNumber _InSectionEndTime)
		: FMovieSceneSkeletalAnimationRateParams(BaseParams), SectionStartTime(InSectionStartTime)
	{}

	float MapTimeToAnimation(FFrameTime InPosition, FFrameRate FrameRate) const;

	UPROPERTY()
	FFrameNumber SectionStartTime;
};

USTRUCT()
struct FMovieSceneSkeletalAnimationRateSectionTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()

		FMovieSceneSkeletalAnimationRateSectionTemplate() {}
	FMovieSceneSkeletalAnimationRateSectionTemplate(const UMovieSceneSkeletalAnimationRateSection& Section);

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;

	UPROPERTY()
		FMovieSceneSkeletalAnimationRateSectionTemplateParameters Params;
};
