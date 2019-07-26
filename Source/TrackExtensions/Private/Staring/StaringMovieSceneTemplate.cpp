#include "Staring/StaringMovieSceneTemplate.h"
#include "Staring/StaringSceneSection.h"
#include "Evaluation/MovieSceneEvaluation.h"
#include "IMovieScenePlayer.h"
#include "CineCameraActor.h"

/* ���������Ŀ�����մ��� */
struct FStaringTrackToken
{
	FStaringTrackToken(AActor* InStaringTrack) : StaringTrack(InStaringTrack)
	{}

	void Apply(ACineCameraActor& CineCameraActor) const
	{
		if (StaringTrack.IsValid())
		{
			CineCameraActor.LookatTrackingSettings.ActorToTrack = TSoftObjectPtr<AActor>(StaringTrack.Get());
			CineCameraActor.LookatTrackingSettings.bEnableLookAtTracking = true;
		}
		else
		{
			CineCameraActor.LookatTrackingSettings.ActorToTrack = TSoftObjectPtr<AActor>();
			CineCameraActor.LookatTrackingSettings.bEnableLookAtTracking = false;
		}
	}

	TWeakObjectPtr<AActor> StaringTrack;
};

/* ��ԭ״̬��ָ�� */
struct FStaringTrackPreAnimatedToken : FStaringTrackToken, IMovieScenePreAnimatedToken
{
	FStaringTrackPreAnimatedToken(AActor* StaringTrack) : FStaringTrackToken(StaringTrack){}

	virtual void RestoreState(UObject& InObject, IMovieScenePlayer& Player) override
	{
		ACineCameraActor* CineCameraActor = CastChecked<ACineCameraActor>(&InObject);

		Apply(*CineCameraActor);
	}
};

/* �ڿ�ʼִ��һ��Sectionʱ����һ��û����ʱ��״̬,���ڻ�ԭ״̬ */
struct FStaringTokenProducer : IMovieScenePreAnimatedTokenProducer
{
	FStaringTokenProducer() {};

	virtual IMovieScenePreAnimatedTokenPtr CacheExistingState(UObject& Object) const override
	{
		ACineCameraActor* CineCameraActor = CastChecked<ACineCameraActor>(&Object);
		return FStaringTrackPreAnimatedToken(CineCameraActor->LookatTrackingSettings.ActorToTrack.Get());
	}
};

/* ����Sectionʱÿһ֡������ô��� */
struct FStaringExecutionToken : IMovieSceneExecutionToken, FStaringTrackToken
{
	FStaringExecutionToken(FMovieSceneObjectBindingID InStaringTargetBindingID) : FStaringTrackToken(nullptr), StaringTargetBindingID(InStaringTargetBindingID) {};

	/** Execute this token, operating on all objects referenced by 'Operand' */
	virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
	{
		FMovieSceneSequenceID SequenceID = Operand.SequenceID;
		if (StaringTargetBindingID.GetSequenceID().IsValid())
		{
			// Ensure that this ID is resolvable from the root, based on the current local sequence ID
			FMovieSceneObjectBindingID RootBindingID = StaringTargetBindingID.ResolveLocalToRoot(SequenceID, Player.GetEvaluationTemplate().GetHierarchy());
			SequenceID = RootBindingID.GetSequenceID();
		}

		FMovieSceneEvaluationOperand StaringTargetOperand(SequenceID, StaringTargetBindingID.GetGuid());

		TArrayView<TWeakObjectPtr<>> OperandObjects = Player.FindBoundObjects(Operand);
		TArrayView<TWeakObjectPtr<>> TargetObjects = Player.FindBoundObjects(StaringTargetOperand);
		if (!TargetObjects.Num() && !OperandObjects.Num())
		{
			return;
		}

		ACineCameraActor* OperandActor = Cast<ACineCameraActor>(OperandObjects[0].Get());
		AActor* StaringTargetActor = Cast<AActor>(TargetObjects[0].Get());

		if (!StaringTargetActor && !OperandActor)
		{
			return;
		}

		Player.SavePreAnimatedState(*OperandActor, TMovieSceneAnimTypeID<FStaringExecutionToken>(), FStaringTokenProducer());

		StaringTrack = StaringTargetActor;
		Apply(*OperandActor);
	}

	FMovieSceneObjectBindingID StaringTargetBindingID;
};





FStaringMovieSceneTemplate::FStaringMovieSceneTemplate(const UStaringSceneSection& Section)
	: StaringTargetBindingID(Section.GetStaringTargetBindingID())
{
}

void FStaringMovieSceneTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
	ExecutionTokens.Add(FStaringExecutionToken(StaringTargetBindingID));
}
