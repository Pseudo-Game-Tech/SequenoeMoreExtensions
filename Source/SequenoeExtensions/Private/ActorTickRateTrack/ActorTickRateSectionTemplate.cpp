// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ActorTickRateSectionTemplate.h"
#include "ActorTickRateTrack.h"
#include "MovieSceneSequence.h"
#include "Evaluation/MovieSceneEvaluationTemplateInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/WorldSettings.h"
#include "EngineGlobals.h"
#include "Evaluation/MovieSceneEvaluation.h"
#include "IMovieScenePlayer.h"
#include "Particles/Emitter.h"
#include "Particles/ParticleSystemComponent.h"
#pragma optimize("", off)
struct FActorTickRateTrackToken
{
	FActorTickRateTrackToken(float InActorTickRateValue)
		: ActorTickRateValue(InActorTickRateValue)
	{}

	float ActorTickRateValue;

	void Apply(AActor& Actor)
	{
		if (AEmitter * Emitter = Cast<AEmitter>(&Actor)) /* 独立窗口时设置Actor.CustomTimeDilation不会影响粒子组件,所以这里直接设置粒子组件得CustomTimeDilation */
		{
			Emitter->GetParticleSystemComponent()->CustomTimeDilation = ActorTickRateValue;
			return;
		}

		Actor.CustomTimeDilation = ActorTickRateValue;
	}
};

struct FActorTickRateTrackData : IPersistentEvaluationData
{
	TOptional<FActorTickRateTrackToken> PreviousActorTickRateValue;
};

struct FActorTickRatePreAnimatedGlobalToken : FActorTickRateTrackToken, IMovieScenePreAnimatedGlobalToken
{
	FActorTickRatePreAnimatedGlobalToken(float InActorTickRateValue, AActor* Actor)
		: FActorTickRateTrackToken(InActorTickRateValue), Actor(Actor)
	{}

	virtual void RestoreState(IMovieScenePlayer& Player) override
	{
		if (IsValid(Actor))
		{
			Apply(*Actor);
		}
	}

	AActor* Actor;
};

struct FActorTickRatePreAnimatedGlobalTokenProducer : IMovieScenePreAnimatedGlobalTokenProducer
{
	FActorTickRatePreAnimatedGlobalTokenProducer(AActor* Actor)
		: Actor(Actor)
	{}

	virtual IMovieScenePreAnimatedGlobalTokenPtr CacheExistingState() const override
	{
		if (IsValid(Actor))
		{
			return FActorTickRatePreAnimatedGlobalToken(Actor->CustomTimeDilation, Actor);
		}
		return IMovieScenePreAnimatedGlobalTokenPtr();
	}

	AActor* Actor;
};

/** A movie scene execution token that applies ActorTickRate */
struct FActorTickRateExecutionToken : IMovieSceneExecutionToken, FActorTickRateTrackToken
{
	FActorTickRateExecutionToken(float InActorTickRateValue)
		: FActorTickRateTrackToken(InActorTickRateValue)
	{}

	static FMovieSceneAnimTypeID GetAnimTypeID()
	{
		return TMovieSceneAnimTypeID<FActorTickRateExecutionToken>();
	}

	/** Execute this token, operating on all objects referenced by 'Operand' */
	virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
	{
		TArrayView<TWeakObjectPtr<>> OperandObjects = Player.FindBoundObjects(Operand);
		if (!OperandObjects.Num())
		{
			return;
		}
		AActor* Actor = Cast<AActor>(OperandObjects[0].Get());

		Player.SavePreAnimatedState(GetAnimTypeID(), FActorTickRatePreAnimatedGlobalTokenProducer(Actor));
		Apply(*Actor);
	}
};

FActorTickRateSectionTemplate::FActorTickRateSectionTemplate(const UActorTickRateSection& Section)
	: ActorTickRateCurve(Section.GetChannel())
{
	
}

void FActorTickRateSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	float ActorTickRateValue = 1.f;
	if (ActorTickRateCurve.Evaluate(Context.GetTime(), ActorTickRateValue) && ActorTickRateValue >= 0.f)
	{
		ExecutionTokens.Add(FActorTickRateExecutionToken(ActorTickRateValue));
	}
}
#pragma optimize("", on)