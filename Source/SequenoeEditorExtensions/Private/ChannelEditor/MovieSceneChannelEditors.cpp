#include "ChannelEditor/MovieSceneChannelEditors.h"
#include "Widgets/Input/SEditableText.h"
#include "ScopedTransaction.h"
#include "PropertyTracks/MovieSceneTextSection.h"
#include "PropertyTracks/MovieSceneTextSection.h"
#include "STextPropertyEditableTextBox.h"
#include "ScopedTransaction.h"

#define LOCTEXT_NAMESPACE "TextCurveKeyEditor"

namespace
{
	class EditableTextCurve : public IEditableTextProperty
	{
	public:
		EditableTextCurve(const TextKeyEditor& Editor) 
			: KeyEditor(Editor)
		{}

		virtual bool IsMultiLineText() const override
		{
			return false;
		}

		virtual bool IsPassword() const override
		{
			return false;
		}

		virtual bool IsReadOnly() const override
		{
			return false;
		}

		virtual bool IsDefaultValue() const override
		{
			return false;
		}

		virtual FText GetToolTipText() const override
		{
			return FText::GetEmpty();
		}

		virtual int32 GetNumTexts() const override
		{
			return 1;
		}

		virtual FText GetText(const int32 InIndex) const override
		{
			check(InIndex == 0);
			return KeyEditor.GetCurrentValue();
		}

		virtual void SetText(const int32 InIndex, const FText& InText) override
		{
			check(InIndex == 0);
			FScopedTransaction Transaction(LOCTEXT("SetStringKey", "Set String Key Value"));
			KeyEditor.SetValueWithNotify(InText, EMovieSceneDataChangeType::TrackValueChangedRefreshImmediately);
		}

		virtual bool IsValidText(const FText& InText, FText& OutErrorMsg) const override
		{
			return true;
		}

#if USE_STABLE_LOCALIZATION_KEYS
		virtual void GetStableTextId(const int32 InIndex, const ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey) const override
		{
			check(InIndex == 0);
			return StaticStableTextId(KeyEditor.GetSequencer()->AsUObject(), InEditAction, InTextSource, InProposedNamespace, InProposedKey, OutStableNamespace, OutStableKey);
		}
#endif // USE_STABLE_LOCALIZATION_KEYS

		virtual void RequestRefresh() override
		{
		}
	private:
		TextKeyEditor KeyEditor;
	};
}

void STextCurveKeyEditor::Construct(const FArguments& InArgs, const TSequencerKeyEditor<FMovieSceneTextChannel, FText>& InKeyEditor)
{
	ChildSlot
		[
			SNew(STextPropertyEditableTextBox, MakeShared<EditableTextCurve>(InKeyEditor))
		];
}

#undef LOCTEXT_NAMESPACE


#define LOCTEXT_NAMESPACE "NameCurveKeyEditor"

void SNameCurveKeyEditor::Construct(const FArguments& InArgs, const TSequencerKeyEditor<FMovieSceneNameChannel, FName>& InKeyEditor)
{
	KeyEditor = InKeyEditor;

	ChildSlot
		[
			SNew(SEditableText)
			.SelectAllTextWhenFocused(true)
		.Text(this, &SNameCurveKeyEditor::GetText)
		.OnTextCommitted(this, &SNameCurveKeyEditor::OnTextCommitted)
		];
}

FText SNameCurveKeyEditor::GetText() const
{
	return FText::FromName(KeyEditor.GetCurrentValue());
}

void SNameCurveKeyEditor::OnTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	FScopedTransaction Transaction(LOCTEXT("SetStringKey", "Set String Key Value"));
	KeyEditor.SetValueWithNotify(FName(*InText.ToString()), EMovieSceneDataChangeType::TrackValueChangedRefreshImmediately);
}

#undef LOCTEXT_NAMESPACE