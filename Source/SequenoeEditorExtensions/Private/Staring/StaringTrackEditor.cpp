#include "StaringTrackEditor.h"
#include "CineCameraActor.h"
#include "GameFramework/Actor.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SButton.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/WorldSettings.h"
#include "SequencerSectionPainter.h"
#include "ActorPickerMode.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "ActorEditorUtils.h"
#include "SceneOutlinerPublicTypes.h"
#include "SceneOutlinerModule.h"
#include "MovieSceneObjectBindingIDPicker.h"
#include "MovieSceneToolHelpers.h"
#include "MovieSceneSection.h"
#include "LevelEditor.h"
#include "Staring/StaringSceneSection.h"
#include "Staring/StaringTrack.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "Templates/SharedPointer.h"

#define LOCTEXT_NAMESPACE "FStaringTrackEditor"

class FStaringSection : public ISequencerSection
{
public:

	FStaringSection(UMovieSceneSection& InSection, FStaringTrackEditor* InStaringTrackEditor)
		: Section(InSection)
		, StaringTrackEditor(InStaringTrackEditor)
	{ }

	/** ISequencerSection interface */
	virtual UMovieSceneSection* GetSectionObject() override
	{
		return &Section;
	}

	virtual FText GetSectionTitle() const override
	{
		UStaringSceneSection* StaringSection = Cast<UStaringSceneSection>(&Section);
		if (StaringSection)
		{
			TSharedPtr<ISequencer> Sequencer = StaringTrackEditor->GetSequencer();
			if (Sequencer.IsValid())
			{
				FMovieSceneSequenceID SequenceID = Sequencer->GetFocusedTemplateID();
				if (StaringSection->GetStaringTargetBindingID().GetSequenceID().IsValid())
				{
					// Ensure that this ID is resolvable from the root, based on the current local sequence ID
					FMovieSceneObjectBindingID RootBindingID = StaringSection->GetStaringTargetBindingID().ResolveLocalToRoot(SequenceID, Sequencer->GetEvaluationTemplate().GetHierarchy());
					SequenceID = RootBindingID.GetSequenceID();
				}

				TArrayView<TWeakObjectPtr<UObject>> RuntimeObjects = Sequencer->FindBoundObjects(StaringSection->GetStaringTargetBindingID().GetGuid(), SequenceID);
				if (RuntimeObjects.Num() == 1 && RuntimeObjects[0].IsValid())
				{
					if (AActor * Actor = Cast<AActor>(RuntimeObjects[0].Get()))
					{
						return FText::FromString(Actor->GetActorLabel());
					}
				}
			}
		}

		return FText::GetEmpty();
	}

	virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override
	{
		return InPainter.PaintSectionBackground();
	}

	virtual void BuildSectionContextMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding) override
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("Add Staring", "Staring"), LOCTEXT("AddStaringTooltip", "Adds an Staring track."),
			FNewMenuDelegate::CreateRaw(StaringTrackEditor, &FStaringTrackEditor::ShowActorMenu, ObjectBinding, &Section));
	}

private:

	/** The section we are visualizing */
	UMovieSceneSection& Section;

	/** The attach track editor */
	FStaringTrackEditor* StaringTrackEditor;
};








FStaringTrackEditor::FStaringTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
}

FStaringTrackEditor::~FStaringTrackEditor()
{
}

TSharedRef<ISequencerTrackEditor> FStaringTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
{
	return MakeShareable(new FStaringTrackEditor(OwningSequencer));
}

void FStaringTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass)
{
	if (ObjectClass != nullptr && ObjectClass->IsChildOf(ACineCameraActor::StaticClass()))
	{
		UMovieSceneSection* DummySection = nullptr;

		MenuBuilder.AddSubMenu(
			LOCTEXT("Add Staring", "Staring"), LOCTEXT("AddStaringTooltip", "Adds an Staring track."),
			FNewMenuDelegate::CreateRaw(this, &FStaringTrackEditor::ShowActorMenu, ObjectBinding, DummySection));
	}
}

TSharedRef<ISequencerSection> FStaringTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	check(SupportsType(SectionObject.GetOuter()->GetClass()));

	return MakeShareable(new FStaringSection(SectionObject, this));
}

bool FStaringTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UStaringTrack::StaticClass();
}

void FStaringTrackEditor::ShowActorMenu(FMenuBuilder& MenuBuilder, FGuid ObjectBinding, UMovieSceneSection* Section)
{
	struct Local
	{
		static FReply OnInteractiveActorPickerClicked(FStaringTrackEditor* ActorPickerTrackEditor, FGuid TheObjectBinding, UMovieSceneSection* TheSection)
		{
			FSlateApplication::Get().DismissAllMenus();
			ActorPickerTrackEditor->PickActorInteractive(TheObjectBinding, TheSection);
			return FReply::Handled();
		}
	};

	auto CreateNewBinding =
		[this, ObjectBinding, Section](FMenuBuilder& SubMenuBuilder)
	{
		using namespace SceneOutliner;

		SceneOutliner::FInitializationOptions InitOptions;
		{
			InitOptions.Mode = ESceneOutlinerMode::ActorPicker;
			InitOptions.bShowHeaderRow = false;
			InitOptions.bFocusSearchBoxWhenOpened = true;
			InitOptions.bShowTransient = true;
			InitOptions.bShowCreateNewFolder = false;
			InitOptions.ColumnMap.Add(FBuiltInColumnTypes::Label(), FColumnInfo(EColumnVisibility::Visible, 0));

			// 过滤可选Actor
			InitOptions.Filters->AddFilterPredicate(SceneOutliner::FActorFilterPredicate::CreateSP(this, &FStaringTrackEditor::IsActorPickable, ObjectBinding, Section));
		}

		FSceneOutlinerModule& SceneOutlinerModule = FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");

		TSharedRef< SWidget > MenuWidget =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.MaxDesiredHeight(400.0f)
				.WidthOverride(300.0f)
				[
					SceneOutlinerModule.CreateSceneOutliner(
						InitOptions,
						FOnActorPicked::CreateSP(this, &FStaringTrackEditor::ActorPicked, ObjectBinding, Section)
					)
				]
			]

		+ SHorizontalBox::Slot()
			.VAlign(VAlign_Top)
			.AutoWidth()
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(4.0f, 0.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
					.ToolTipText(LOCTEXT("PickButtonLabel", "Choose a gaze target"))
					.ButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
					.OnClicked(FOnClicked::CreateStatic(&Local::OnInteractiveActorPickerClicked, this, ObjectBinding, Section))
					.ContentPadding(4.0f)
					.ForegroundColor(FSlateColor::UseForeground())
					.IsFocusable(false)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("PropertyWindow.Button_PickActorInteractive"))
					.ColorAndOpacity(FSlateColor::UseForeground())
					]
			]
			];

		SubMenuBuilder.AddWidget(MenuWidget, FText::GetEmpty(), false);
	};

	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();

	// 始终重新创建绑定ID选择器,保证是最新结果
	BindingIDPicker = MakeShared<FTrackEditorBindingIDPicker>(SequencerPtr->GetFocusedTemplateID(), SequencerPtr);
	BindingIDPicker->OnBindingPicked().AddRaw(this, &FStaringTrackEditor::ExistingBindingPicked, ObjectBinding, Section);

	FText ExistingBindingText = LOCTEXT("ExistingBinding", "Existing Binding");
	FText NewBindingText = LOCTEXT("NewBinding", "New Binding");

	const bool bHasExistingBindings = !BindingIDPicker->IsEmpty();
	if (bHasExistingBindings)
	{
		MenuBuilder.AddSubMenu(
			NewBindingText,
			LOCTEXT("NewBinding_Tip", "Add a new section by creating a new binding to an object in the world."),
			FNewMenuDelegate::CreateLambda(CreateNewBinding)
		);

		MenuBuilder.BeginSection(NAME_None, ExistingBindingText);
		{
			BindingIDPicker->GetPickerMenu(MenuBuilder);
		}
		MenuBuilder.EndSection();
	}
	else
	{
		MenuBuilder.BeginSection(NAME_None, NewBindingText);
		{
			CreateNewBinding(MenuBuilder);
		}
		MenuBuilder.EndSection();
	}
}

void FStaringTrackEditor::PickActorInteractive(FGuid ObjectBinding, UMovieSceneSection* Section)
{
	if (GUnrealEd->GetSelectedActorCount())
	{
		FActorPickerModeModule& ActorPickerMode = FModuleManager::Get().GetModuleChecked<FActorPickerModeModule>("ActorPickerMode");

		ActorPickerMode.BeginActorPickingMode(
			FOnGetAllowedClasses(),
			FOnShouldFilterActor::CreateSP(this, &FStaringTrackEditor::IsActorPickable, ObjectBinding, Section),
			FOnActorSelected::CreateSP(this, &FStaringTrackEditor::ActorPicked, ObjectBinding, Section)
		);
	}
}

bool FStaringTrackEditor::IsActorPickable(const AActor* const TargetActor, FGuid ObjectBinding, UMovieSceneSection* InSection)
{
	TArrayView<TWeakObjectPtr<>> Objects = GetSequencer()->FindObjectsInCurrentSequence(ObjectBinding);
	if (Objects.Contains(TargetActor) && Objects.Num()) // 排除当前对象
	{
		return false;
	}

	if (
		TargetActor->IsListedInSceneOutliner() &&
		!FActorEditorUtils::IsABuilderBrush(TargetActor) &&
		!TargetActor->IsA(AWorldSettings::StaticClass()) &&
		!TargetActor->IsPendingKill()
		) // 确保是正常对象
	{
		return true;
	}

	return false;
}

void FStaringTrackEditor::ActorPicked(AActor* TargetActor, FGuid ObjectBinding, UMovieSceneSection* Section)
{
	ActorPickerIDPicked(FActorPickerID(TargetActor, FMovieSceneObjectBindingID()), ObjectBinding, Section);
}

void FStaringTrackEditor::ExistingBindingPicked(FMovieSceneObjectBindingID ExistingBindingID, FGuid ObjectBinding, UMovieSceneSection* Section)
{
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();

	TArrayView<TWeakObjectPtr<UObject>> RuntimeObjects = SequencerPtr->FindBoundObjects(ExistingBindingID.GetGuid(), ExistingBindingID.GetSequenceID());
	for (auto RuntimeObject : RuntimeObjects)
	{
		if (RuntimeObject.IsValid())
		{
			AActor* Actor = Cast<AActor>(RuntimeObject.Get());
			if (Actor)
			{
				ActorPickerIDPicked(FActorPickerID(Actor, ExistingBindingID), ObjectBinding, Section);
				return;
			}
		}
	}

	ActorPickerIDPicked(FActorPickerID(nullptr, ExistingBindingID), ObjectBinding, Section);
}

void FStaringTrackEditor::ActorPickerIDPicked(FActorPickerID ActorPickerID, FGuid ObjectBinding, UMovieSceneSection* Section)
{
	if (Section != nullptr)
	{
		const FScopedTransaction Transaction(LOCTEXT("UndoSetStaring", "Set Staring"));

		UStaringSceneSection* StaringSection = (UStaringSceneSection*)(Section);

		FMovieSceneObjectBindingID StaringTargetBindingID;

		if (ActorPickerID.ExistingBindingID.IsValid())
		{
			StaringTargetBindingID = ActorPickerID.ExistingBindingID;
		}
		else if (ActorPickerID.ActorPicked.IsValid())
		{
			FGuid ParentActorId = FindOrCreateHandleToObject(ActorPickerID.ActorPicked.Get()).Handle;
			StaringTargetBindingID = FMovieSceneObjectBindingID(ParentActorId, MovieSceneSequenceID::Root, EMovieSceneObjectBindingSpace::Local);
		}

		if (StaringTargetBindingID.IsValid())
		{
			StaringSection->SetStaringTargetBindingID(StaringTargetBindingID);
		}
	}
	else if (ObjectBinding.IsValid())
	{
		TArray<TWeakObjectPtr<>> OutObjects;
		for (TWeakObjectPtr<> Object : GetSequencer()->FindObjectsInCurrentSequence(ObjectBinding))
		{
			OutObjects.Add(Object);
		}

		AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FStaringTrackEditor::AddKeyInternal, OutObjects, ActorPickerID));
	}
}

FKeyPropertyResult FStaringTrackEditor::AddKeyInternal(FFrameNumber KeyTime, const TArray<TWeakObjectPtr<UObject>> Objects, FActorPickerID ActorPickerID)
{
	FKeyPropertyResult KeyPropertyResult;

	FMovieSceneObjectBindingID StaringTargetBindingID;
	{
		if (ActorPickerID.ExistingBindingID.IsValid())
		{
			StaringTargetBindingID = ActorPickerID.ExistingBindingID;
		}
		else if (ActorPickerID.ActorPicked.IsValid())
		{
			FFindOrCreateHandleResult HandleResult = FindOrCreateHandleToObject(ActorPickerID.ActorPicked.Get());
			FGuid StaringTrackId = HandleResult.Handle;
			KeyPropertyResult.bHandleCreated |= HandleResult.bWasCreated;
			StaringTargetBindingID = FMovieSceneObjectBindingID(StaringTrackId, MovieSceneSequenceID::Root, EMovieSceneObjectBindingSpace::Local);
		}

		if (!StaringTargetBindingID.IsValid())
		{
			return KeyPropertyResult;
		}
	}

	UMovieScene* MovieScene = GetSequencer()->GetFocusedMovieSceneSequence()->GetMovieScene();

	MovieScene->Modify();

	{
		for (int32 ObjectIndex = 0; ObjectIndex < Objects.Num(); ++ObjectIndex)
		{
			UObject* Object = Objects[ObjectIndex].Get();

			FFindOrCreateHandleResult HandleResult = FindOrCreateHandleToObject(Object);
			FGuid ObjectHandle = HandleResult.Handle;
			KeyPropertyResult.bHandleCreated |= HandleResult.bWasCreated;
			if (ObjectHandle.IsValid())
			{
				FFindOrCreateTrackResult TrackResult = FindOrCreateTrackForObject(ObjectHandle, UStaringTrack::StaticClass());
				UMovieSceneTrack* Track = TrackResult.Track;
				Track->Modify();
				KeyPropertyResult.bTrackCreated |= TrackResult.bWasCreated;

				if (ensure(Track))
				{
					// Clamp to next attach section's start time or the end of the current movie scene range
					FFrameNumber StaringEndTime = MovieScene->GetPlaybackRange().GetUpperBoundValue();
					{
						for (UMovieSceneSection* Section : Track->GetAllSections())
						{
							FFrameNumber StartTime = Section->HasStartFrame() ? Section->GetInclusiveStartFrame() : 0;
							if (KeyTime < StartTime)
							{
								if (StaringEndTime > StartTime)
								{
									StaringEndTime = StartTime;
								}
							}
						}
					}

					int32 Duration = FMath::Max(0, (StaringEndTime - KeyTime).Value);
					UMovieSceneSection* NewSection = Cast<UStaringTrack>(Track)->AddStaring(KeyTime, Duration, StaringTargetBindingID);
					KeyPropertyResult.bTrackModified = true;

					GetSequencer()->EmptySelection();
					GetSequencer()->SelectSection(NewSection);
					GetSequencer()->ThrobSectionSelection();
				}
			}
		}
	}


	return KeyPropertyResult;
}

#undef LOCTEXT_NAMESPACE