#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "Misc/Guid.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "MovieSceneObjectBindingID.h"
#include "StaringMovieSceneTemplate.generated.h"

class UStaringSceneSection;

USTRUCT()
struct FStaringMovieSceneTemplate : public FMovieSceneEvalTemplate
{
	GENERATED_BODY()

	FStaringMovieSceneTemplate() {};
	FStaringMovieSceneTemplate(const UStaringSceneSection& Section);

	/** 摄像机盯着得目标 */
	UPROPERTY()
		FMovieSceneObjectBindingID StaringTargetBindingID;

private:

	virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }
	virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
};