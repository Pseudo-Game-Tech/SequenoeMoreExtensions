#pragma once

#include "CoreMinimal.h"
#include "Misc/Guid.h"
#include "ISequencerSection.h"
#include "ISequencer.h"
#include "ISequencerTrackEditor.h"
#include "MovieSceneTrackEditor.h"

class AActor;
class FMenuBuilder;
class FTrackEditorBindingIDPicker;
struct FMovieSceneObjectBindingID;

class FStaringTrackEditor : public FMovieSceneTrackEditor
{
public:

	struct FActorPickerID
	{
		FActorPickerID(AActor* InActorPicked, FMovieSceneObjectBindingID InExistingBindingID) : ActorPicked(InActorPicked), ExistingBindingID(InExistingBindingID) {}

		TWeakObjectPtr<AActor> ActorPicked;
		FMovieSceneObjectBindingID ExistingBindingID;
	};

	FStaringTrackEditor(TSharedRef<ISequencer> InSequencer);

	~FStaringTrackEditor();

	/** ����FStaringTrackEditorʵ���ýӿ�,��Sequencer����,�ο�ISequencerModule::RegisterTrackEditor */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer);

public:
	/** ISequencerTrackEditor �ӿ� */

	virtual void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass) override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;

public:
	void ShowActorMenu(FMenuBuilder& MenuBuilder, FGuid ObjectBinding, UMovieSceneSection* Section);

private:
	/** ActorMenu�˵�������actor��ť�¼� */
	void PickActorInteractive(FGuid ObjectBinding, UMovieSceneSection* Section);
	/** ���ڹ���SceneOutlinerѡ�����е�Actor */
	bool IsActorPickable(const AActor* const TargetActor, FGuid ObjectBinding, UMovieSceneSection* InSection);
	/** ѡ���˳����е�Actor */
	void ActorPicked(AActor* TargetActor, FGuid ObjectBinding, UMovieSceneSection* Section);
	/** ѡ�������еð󶨶��� */
	void ExistingBindingPicked(FMovieSceneObjectBindingID ExistingBindingID, FGuid ObjectBinding, UMovieSceneSection* Section);
	/** ѡ���˳����е�Actor�������еð󶨶���,��ActorPicked/ExistingBindingPicked��ʵ��ѡ���� */
	void ActorPickerIDPicked(FActorPickerID ActorPickerID, FGuid ObjectBinding, UMovieSceneSection* Section);

	/** AnimatablePropertyChanged�ûص��¼� */
	FKeyPropertyResult AddKeyInternal(FFrameNumber KeyTime, const TArray<TWeakObjectPtr<UObject>> Objects, FActorPickerID ActorPickerID);

	/** ��IDѡ����,����ͨ��ѡ���������µù켣��(section) */
	TSharedPtr<FTrackEditorBindingIDPicker> BindingIDPicker;
};