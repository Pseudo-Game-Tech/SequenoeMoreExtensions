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

	/** 创建FStaringTrackEditor实例得接口,由Sequencer调用,参考ISequencerModule::RegisterTrackEditor */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer);

public:
	/** ISequencerTrackEditor 接口 */

	virtual void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass) override;
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;

public:
	void ShowActorMenu(FMenuBuilder& MenuBuilder, FGuid ObjectBinding, UMovieSceneSection* Section);

private:
	/** ActorMenu菜单界面点击actor按钮事件 */
	void PickActorInteractive(FGuid ObjectBinding, UMovieSceneSection* Section);
	/** 用于过滤SceneOutliner选择器中的Actor */
	bool IsActorPickable(const AActor* const TargetActor, FGuid ObjectBinding, UMovieSceneSection* InSection);
	/** 选择了场景中得Actor */
	void ActorPicked(AActor* TargetActor, FGuid ObjectBinding, UMovieSceneSection* Section);
	/** 选择了现有得绑定对象 */
	void ExistingBindingPicked(FMovieSceneObjectBindingID ExistingBindingID, FGuid ObjectBinding, UMovieSceneSection* Section);
	/** 选择了场景中得Actor或者现有得绑定对象,是ActorPicked/ExistingBindingPicked得实际选择处理 */
	void ActorPickerIDPicked(FActorPickerID ActorPickerID, FGuid ObjectBinding, UMovieSceneSection* Section);

	/** AnimatablePropertyChanged得回调事件 */
	FKeyPropertyResult AddKeyInternal(FFrameNumber KeyTime, const TArray<TWeakObjectPtr<UObject>> Objects, FActorPickerID ActorPickerID);

	/** 绑定ID选择器,可以通过选择器创建新得轨迹段(section) */
	TSharedPtr<FTrackEditorBindingIDPicker> BindingIDPicker;
};