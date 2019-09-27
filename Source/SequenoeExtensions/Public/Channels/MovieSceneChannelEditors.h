#pragma once

#include "SequencerChannelInterface.h"
#include "Channels/MovieSceneTextChannel.h"
#include "Channels/MovieSceneNameChannel.h"
#include "Templates/SharedPointer.h"
#include "MovieSceneSection.h"
#include "ISequencer.h"
#include "MovieSceneCommonHelpers.h"
#include "Channels/MovieSceneChannelTraits.h"
#include "SequencerChannelTraits.h"
#include "Channels/MovieSceneChannelHandle.h"
#include "MovieSceneTimeHelpers.h"

namespace MovieSceneClipboard
{
	template<> inline FName GetKeyTypeName<FText>()
	{
		static FName Name("Text");
		return Name;
	}
	template<> inline FName GetKeyTypeName<FName>()
	{
		static FName Name("Name");
		return Name;
	}
}

template<typename ChannelType, typename ValueType>
struct SEQUENOEEXTENSIONS_API TSequencerKeyEditor
{
	TSequencerKeyEditor()
	{}

	TSequencerKeyEditor(
		FGuid                                    InObjectBindingID,
		TMovieSceneChannelHandle<ChannelType>    InChannelHandle,
		TWeakObjectPtr<UMovieSceneSection>       InWeakSection,
		TWeakPtr<ISequencer>                     InWeakSequencer,
		TWeakPtr<FTrackInstancePropertyBindings> InWeakPropertyBindings,
		TFunction<TOptional<ValueType>(UObject&, FTrackInstancePropertyBindings*)> InOnGetExternalValue
	)
		: ObjectBindingID(InObjectBindingID)
		, ChannelHandle(InChannelHandle)
		, WeakSection(InWeakSection)
		, WeakSequencer(InWeakSequencer)
		, WeakPropertyBindings(InWeakPropertyBindings)
		, OnGetExternalValue(InOnGetExternalValue)
	{}

	static TOptional<ValueType> Get(const FGuid& ObjectBindingID, ISequencer* Sequencer, FTrackInstancePropertyBindings* PropertyBindings, const TFunction<TOptional<ValueType>(UObject&, FTrackInstancePropertyBindings*)>& OnGetExternalValue)
	{
		if (!Sequencer || !ObjectBindingID.IsValid() || !OnGetExternalValue)
		{
			return TOptional<ValueType>();
		}

		for (TWeakObjectPtr<> WeakObject : Sequencer->FindBoundObjects(ObjectBindingID, Sequencer->GetFocusedTemplateID()))
		{
			if (UObject* Object = WeakObject.Get())
			{
				TOptional<ValueType> ExternalValue = OnGetExternalValue(*Object, PropertyBindings);
				if (ExternalValue.IsSet())
				{
					return ExternalValue;
				}
			}
		}

		return TOptional<ValueType>();
	}

	TOptional<ValueType> GetExternalValue() const
	{
		return Get(ObjectBindingID, WeakSequencer.Pin().Get(), WeakPropertyBindings.Pin().Get(), OnGetExternalValue);
	}

	ValueType GetCurrentValue() const
	{
		using namespace MovieScene;

		ChannelType* Channel = ChannelHandle.Get();
		ISequencer* Sequencer = WeakSequencer.Pin().Get();
		UMovieSceneSection* OwningSection = WeakSection.Get();

		ValueType Result{};

		if (Channel && Sequencer && OwningSection)
		{
			const FFrameTime CurrentTime = MovieScene::ClampToDiscreteRange(Sequencer->GetLocalTime().Time, OwningSection->GetRange());
			//If we have no keys and no default, key with the external value if it exists
			if (!EvaluateChannel(Channel, CurrentTime, Result))
			{
				if (TOptional<ValueType> ExternalValue = GetExternalValue())
				{
					if (ExternalValue.IsSet())
					{
						Result = ExternalValue.GetValue();
					}
				}
			}
		}

		return Result;
	}

	void SetValue(const ValueType& InValue)
	{
		using namespace MovieScene;
		using namespace Sequencer;

		UMovieSceneSection* OwningSection = WeakSection.Get();
		if (!OwningSection)
		{
			return;
		}

		OwningSection->SetFlags(RF_Transactional);

		ChannelType* Channel = ChannelHandle.Get();
		ISequencer* Sequencer = WeakSequencer.Pin().Get();

		if (!OwningSection->TryModify() || !Channel || !Sequencer)
		{
			return;
		}

		const FFrameNumber CurrentTime = Sequencer->GetLocalTime().Time.FloorToFrame();
		const bool  bAutoSetTrackDefaults = Sequencer->GetAutoSetTrackDefaults();

		EMovieSceneKeyInterpolation Interpolation = Sequencer->GetKeyInterpolation();

		TArray<FKeyHandle> KeysAtCurrentTime;
		Channel->GetKeys(TRange<FFrameNumber>(CurrentTime), nullptr, &KeysAtCurrentTime);

		if (KeysAtCurrentTime.Num() > 0)
		{
			AssignValue(Channel, KeysAtCurrentTime[0], InValue);
		}
		else
		{
			bool bHasAnyKeys = Channel->GetNumKeys() != 0;

			if (bHasAnyKeys || bAutoSetTrackDefaults == false)
			{
				// When auto setting track defaults are disabled, add a key even when it's empty so that the changed
				// value is saved and is propagated to the property.
				AddKeyToChannel(Channel, CurrentTime, InValue, Interpolation);
				bHasAnyKeys = Channel->GetNumKeys() != 0;
			}

			if (bHasAnyKeys)
			{
				TRange<FFrameNumber> KeyRange = TRange<FFrameNumber>(CurrentTime);
				TRange<FFrameNumber> SectionRange = OwningSection->GetRange();

				if (!SectionRange.Contains(KeyRange))
				{
					OwningSection->SetRange(TRange<FFrameNumber>::Hull(KeyRange, SectionRange));
				}
			}
		}

		// Always update the default value when auto-set default values is enabled so that the last changes
		// are always saved to the track.
		if (bAutoSetTrackDefaults)
		{
			SetChannelDefault(Channel, InValue);
		}
	}

	void SetValueWithNotify(const ValueType& InValue, EMovieSceneDataChangeType NotifyType = EMovieSceneDataChangeType::TrackValueChanged)
	{
		SetValue(InValue);
		if (ISequencer* Sequencer = WeakSequencer.Pin().Get())
		{
			Sequencer->NotifyMovieSceneDataChanged(NotifyType);
		}
	}

	const FGuid& GetObjectBindingID() const
	{
		return ObjectBindingID;
	}

	ISequencer* GetSequencer() const
	{
		return WeakSequencer.Pin().Get();
	}

	FTrackInstancePropertyBindings* GetPropertyBindings() const
	{
		return WeakPropertyBindings.Pin().Get();
	}

private:

	FGuid ObjectBindingID;
	TMovieSceneChannelHandle<ChannelType> ChannelHandle;
	TWeakObjectPtr<UMovieSceneSection> WeakSection;
	TWeakPtr<ISequencer> WeakSequencer;
	TWeakPtr<FTrackInstancePropertyBindings> WeakPropertyBindings;
	TFunction<TOptional<ValueType>(UObject&, FTrackInstancePropertyBindings*)> OnGetExternalValue;
};

using TextKeyEditor = TSequencerKeyEditor<FMovieSceneTextChannel, FText>;

class SEQUENOEEXTENSIONS_API STextCurveKeyEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STextCurveKeyEditor) {}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs, const TextKeyEditor& InKeyEditor);
};

struct SEQUENOEEXTENSIONS_API MovieSceneTextChannelEditor : TSequencerChannelInterface<FMovieSceneTextChannel>
{
	bool CanCreateKeyEditor_Raw(const FMovieSceneChannel* InChannel) const override
	{
		return true;
	}

	TSharedRef<SWidget> CreateKeyEditor_Raw(const FMovieSceneChannelHandle& RawChannel, UMovieSceneSection* Section, const FGuid& InObjectBindingID, TWeakPtr<FTrackInstancePropertyBindings> PropertyBindings, TWeakPtr<ISequencer> Sequencer) const override
	{
		const TMovieSceneChannelHandle<FMovieSceneTextChannel>& Channel = RawChannel.Cast<FMovieSceneTextChannel>();
		const TMovieSceneExternalValue<FText>* ExternalValue = Channel.GetExtendedEditorData();
		if (!ExternalValue)
		{
			return SNullWidget::NullWidget;
		}

		TextKeyEditor KeyEditor(
			InObjectBindingID, Channel,
			Section, Sequencer, PropertyBindings, ExternalValue->OnGetExternalValue
		);

		return SNew(STextCurveKeyEditor, KeyEditor);
	}
};

using NameKeyEditor = TSequencerKeyEditor<FMovieSceneNameChannel, FName>;

class SEQUENOEEXTENSIONS_API SNameCurveKeyEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SNameCurveKeyEditor) {}
	SLATE_END_ARGS();

	void Construct(const FArguments& InArgs, const NameKeyEditor& InKeyEditor);

private:

	FText GetText() const;
	void OnTextCommitted(const FText& InText, ETextCommit::Type InCommitType);

	NameKeyEditor KeyEditor;
};

struct SEQUENOEEXTENSIONS_API MovieSceneNameChannelEditor : TSequencerChannelInterface<FMovieSceneNameChannel>
{
	bool CanCreateKeyEditor_Raw(const FMovieSceneChannel* InChannel) const override
	{
		return true;
	}

	TSharedRef<SWidget> CreateKeyEditor_Raw(const FMovieSceneChannelHandle& RawChannel, UMovieSceneSection* Section, const FGuid& InObjectBindingID, TWeakPtr<FTrackInstancePropertyBindings> PropertyBindings, TWeakPtr<ISequencer> Sequencer) const override
	{
		const TMovieSceneChannelHandle<FMovieSceneNameChannel>& Channel = RawChannel.Cast<FMovieSceneNameChannel>();
		const TMovieSceneExternalValue<FName>* ExternalValue = Channel.GetExtendedEditorData();
		if (!ExternalValue)
		{
			return SNullWidget::NullWidget;
		}

		NameKeyEditor KeyEditor(
			InObjectBindingID, Channel,
			Section, Sequencer, PropertyBindings, ExternalValue->OnGetExternalValue
		);

		return SNew(SNameCurveKeyEditor, KeyEditor);
	}
};