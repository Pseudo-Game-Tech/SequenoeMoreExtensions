// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "SkeletalAnimationRateTrackEditor.h"
#include "Rendering/DrawElements.h"
#include "Widgets/SBoxPanel.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GameFramework/Actor.h"
#include "AssetData.h"
#include "Animation/AnimSequenceBase.h"
#include "Modules/ModuleManager.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/MenuStack.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Layout/SBox.h"
#include "SequencerSectionPainter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "SkeletalAnimationRateTrack/MovieSceneSkeletalAnimationRateTrack.h"
#include "SkeletalAnimationRateTrack/MovieSceneSkeletalAnimationRateSection.h"
#include "CommonMovieSceneTools.h"
#include "AssetRegistryModule.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "MatineeImportTools.h"
#include "Matinee/InterpTrackAnimControl.h"
#include "SequencerUtilities.h"
#include "ISectionLayoutBuilder.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequence.h"
#include "Animation/PoseAsset.h"
#include "EditorStyleSet.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "MovieSceneTimeHelpers.h"
#include "Fonts/FontMeasure.h"
#include "SequencerTimeSliderController.h"
#include "AnimationEditorUtils.h"
#include "Factories/PoseAssetFactory.h"
#include "Misc/MessageDialog.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Toolkits/AssetEditorManager.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/Blueprint.h"
#include "Matinee/InterpTrackAnimControl.h"
#include "Channels/MovieSceneFloatChannel.h"

namespace SkeletalAnimationRateEditorConstants
{
	// @todo Sequencer Allow this to be customizable
	const uint32 AnimationTrackHeight = 20;
}

#define LOCTEXT_NAMESPACE "FSkeletalAnimationRateTrackEditor"

USkeletalMeshComponent* AcquireSkeletalMeshFromObjectGuid(const FGuid& Guid, TSharedPtr<ISequencer> SequencerPtr)
{
	UObject* BoundObject = SequencerPtr.IsValid() ? SequencerPtr->FindSpawnedObjectOrTemplate(Guid) : nullptr;

	if (AActor * Actor = Cast<AActor>(BoundObject))
	{
		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (USkeletalMeshComponent * SkeletalMeshComp = Cast<USkeletalMeshComponent>(Component))
			{
				return SkeletalMeshComp;
			}
		}
	}
	else if (USkeletalMeshComponent * SkeletalMeshComponent = Cast<USkeletalMeshComponent>(BoundObject))
	{
		if (SkeletalMeshComponent->SkeletalMesh)
		{
			return SkeletalMeshComponent;
		}
	}

	return nullptr;
}

USkeleton* GetSkeletonFromComponent(UActorComponent* InComponent)
{
	USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(InComponent);
	if (SkeletalMeshComp && SkeletalMeshComp->SkeletalMesh && SkeletalMeshComp->SkeletalMesh->Skeleton)
	{
		// @todo Multiple actors, multiple components
		return SkeletalMeshComp->SkeletalMesh->Skeleton;
	}

	return nullptr;
}

USkeleton* AcquireSkeletonFromObjectGuid(const FGuid& Guid, TSharedPtr<ISequencer> SequencerPtr)
{
	UObject* BoundObject = SequencerPtr.IsValid() ? SequencerPtr->FindSpawnedObjectOrTemplate(Guid) : nullptr;

	if (AActor * Actor = Cast<AActor>(BoundObject))
	{
		for (UActorComponent* Component : Actor->GetComponents())
		{
			if (USkeleton * Skeleton = GetSkeletonFromComponent(Component))
			{
				return Skeleton;
			}
		}

		AActor* ActorCDO = Cast<AActor>(Actor->GetClass()->GetDefaultObject());
		if (ActorCDO)
		{
			for (UActorComponent* Component : ActorCDO->GetComponents())
			{
				if (USkeleton * Skeleton = GetSkeletonFromComponent(Component))
				{
					return Skeleton;
				}
			}
		}

		UBlueprintGeneratedClass* ActorBlueprintGeneratedClass = Cast<UBlueprintGeneratedClass>(Actor->GetClass());
		if (ActorBlueprintGeneratedClass)
		{
			const TArray<USCS_Node*>& ActorBlueprintNodes = ActorBlueprintGeneratedClass->SimpleConstructionScript->GetAllNodes();

			for (USCS_Node* Node : ActorBlueprintNodes)
			{
				if (Node->ComponentClass->IsChildOf(USkeletalMeshComponent::StaticClass()))
				{
					if (USkeleton * Skeleton = GetSkeletonFromComponent(Node->GetActualComponentTemplate(ActorBlueprintGeneratedClass)))
					{
						return Skeleton;
					}
				}
			}
		}
	}
	else if (USkeletalMeshComponent * SkeletalMeshComponent = Cast<USkeletalMeshComponent>(BoundObject))
	{
		if (USkeleton * Skeleton = GetSkeletonFromComponent(SkeletalMeshComponent))
		{
			return Skeleton;
		}
	}

	return nullptr;
}


FSkeletalAnimationRateSection::FSkeletalAnimationRateSection(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer)
	: Section(*CastChecked<UMovieSceneSkeletalAnimationRateSection>(&InSection))
	, Sequencer(InSequencer)
	, InitialStartOffsetDuringResize(0)
	, InitialStartTimeDuringResize(0)
{
	UpdateSectionData();
	OnMovieSceneDataChangedDelegateHandle = Sequencer.Pin().Get()->OnMovieSceneDataChanged().AddLambda([this](EMovieSceneDataChangeType ChangeType)
		{
			if (ChangeType == EMovieSceneDataChangeType::TrackValueChanged || ChangeType == EMovieSceneDataChangeType::TrackValueChangedRefreshImmediately)
			{
				UpdateSectionData();
			}
		});
}

FSkeletalAnimationRateSection::~FSkeletalAnimationRateSection()
{
	if (Sequencer.Pin().IsValid())
	{
		Sequencer.Pin().Get()->OnMovieSceneDataChanged().Remove(OnMovieSceneDataChangedDelegateHandle);
	}
}


UMovieSceneSection* FSkeletalAnimationRateSection::GetSectionObject()
{
	return &Section;
}


FText FSkeletalAnimationRateSection::GetSectionTitle() const
{
	if (Section.Params.Animation != nullptr)
	{
		return FText::FromString(Section.Params.Animation->GetName());
	}
	return LOCTEXT("NoAnimationSection", "No Animation");
}


float FSkeletalAnimationRateSection::GetSectionHeight() const
{
	return (float)SkeletalAnimationRateEditorConstants::AnimationTrackHeight;
}


FMargin FSkeletalAnimationRateSection::GetContentPadding() const
{
	return FMargin(8.0f, 8.0f);
}


int32 FSkeletalAnimationRateSection::OnPaintSection(FSequencerSectionPainter& Painter) const
{
	const ESlateDrawEffect DrawEffects = Painter.bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

	const FTimeToPixel& TimeToPixelConverter = Painter.GetTimeConverter();

	int32 LayerId = Painter.PaintSectionBackground();

	static const FSlateBrush* GenericDivider = FEditorStyle::GetBrush("Sequencer.GenericDivider");

	if (!Section.HasStartFrame() || !Section.HasEndFrame())
	{
		return LayerId;
	}

	FFrameRate TickResolution = TimeToPixelConverter.GetTickResolution();

	float SeqLength = Section.Params.GetSequenceLength() - (TickResolution.AsSeconds(Section.Params.StartFrameOffset + Section.Params.EndFrameOffset));

	if (!FMath::IsNearlyZero(SeqLength, KINDA_SMALL_NUMBER) && SeqLength > 0)
	{
		float MaxOffset = Section.GetRange().Size<FFrameTime>() / TickResolution;
		float OffsetTime = SeqLength;
		float StartTime = Section.GetInclusiveStartFrame() / TickResolution;

		while (OffsetTime < MaxOffset)
		{
			float OffsetPixel = TimeToPixelConverter.SecondsToPixel(StartTime + OffsetTime) - TimeToPixelConverter.SecondsToPixel(StartTime);

			FSlateDrawElement::MakeBox(
				Painter.DrawElements,
				LayerId,
				Painter.SectionGeometry.MakeChild(
					FVector2D(2.f, Painter.SectionGeometry.Size.Y - 2.f),
					FSlateLayoutTransform(FVector2D(OffsetPixel, 1.f))
				).ToPaintGeometry(),
				GenericDivider,
				DrawEffects
			);

			OffsetTime += SeqLength;
		}
	}

	TSharedPtr<ISequencer> SequencerPtr = Sequencer.Pin();
	if (Painter.bIsSelected && SequencerPtr.IsValid())
	{
		FFrameTime CurrentTime = SequencerPtr->GetLocalTime().Time;
		if (Section.GetRange().Contains(CurrentTime.FrameNumber) && Section.Params.Animation != nullptr)
		{
			const float Time = TimeToPixelConverter.FrameToPixel(CurrentTime);

			// Draw the current time next to the scrub handle
			const float AnimTime = Section.MapTimeToAnimation(CurrentTime, TickResolution);
			int32 FrameTime = Section.Params.Animation->GetFrameAtTime(AnimTime);
			FString FrameString = FString::FromInt(FrameTime);

			const FSlateFontInfo SmallLayoutFont = FCoreStyle::GetDefaultFontStyle("Bold", 10);
			const TSharedRef< FSlateFontMeasure > FontMeasureService = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			FVector2D TextSize = FontMeasureService->Measure(FrameString, SmallLayoutFont);

			// Flip the text position if getting near the end of the view range
			static const float TextOffsetPx = 10.f;
			bool  bDrawLeft = (Painter.SectionGeometry.Size.X - Time) < (TextSize.X + 22.f) - TextOffsetPx;
			float TextPosition = bDrawLeft ? Time - TextSize.X - TextOffsetPx : Time + TextOffsetPx;
			//handle mirrored labels
			const float MajorTickHeight = 9.0f;
			FVector2D TextOffset(TextPosition, Painter.SectionGeometry.Size.Y - (MajorTickHeight + TextSize.Y));

			const FLinearColor DrawColor = FEditorStyle::GetSlateColor("SelectionColor").GetColor(FWidgetStyle());
			const FVector2D BoxPadding = FVector2D(4.0f, 2.0f);
			// draw time string

			FSlateDrawElement::MakeBox(
				Painter.DrawElements,
				LayerId + 5,
				Painter.SectionGeometry.ToPaintGeometry(TextOffset - BoxPadding, TextSize + 2.0f * BoxPadding),
				FEditorStyle::GetBrush("WhiteBrush"),
				ESlateDrawEffect::None,
				FLinearColor::Black.CopyWithNewOpacity(0.5f)
			);

			FSlateDrawElement::MakeText(
				Painter.DrawElements,
				LayerId + 6,
				Painter.SectionGeometry.ToPaintGeometry(TextOffset, TextSize),
				FrameString,
				SmallLayoutFont,
				DrawEffects,
				DrawColor
			);
		}
	}

	return LayerId;
}

void FSkeletalAnimationRateSection::BeginResizeSection()
{
	InitialStartOffsetDuringResize = Section.Params.StartFrameOffset;
	InitialStartTimeDuringResize = Section.HasStartFrame() ? Section.GetInclusiveStartFrame() : 0;
}

void FSkeletalAnimationRateSection::ResizeSection(ESequencerSectionResizeMode ResizeMode, FFrameNumber ResizeTime)
{
	// Adjust the start offset when resizing from the beginning
	if (ResizeMode == SSRM_LeadingEdge)
	{
		FFrameRate FrameRate = Section.GetTypedOuter<UMovieScene>()->GetTickResolution();
		FFrameNumber StartOffset = FrameRate.AsFrameNumber((ResizeTime - InitialStartTimeDuringResize) / FrameRate);

		StartOffset += InitialStartOffsetDuringResize;

		// Ensure start offset is not less than 0 and adjust ResizeTime
		if (StartOffset < 0)
		{
			ResizeTime = ResizeTime - StartOffset;

			StartOffset = FFrameNumber(0);
		}

		Section.Params.StartFrameOffset = StartOffset;
	}

	ISequencerSection::ResizeSection(ResizeMode, ResizeTime);
	UpdateSectionData();
}

void FSkeletalAnimationRateSection::BeginSlipSection()
{
	BeginResizeSection();
}

void FSkeletalAnimationRateSection::SlipSection(FFrameNumber SlipTime)
{
	FFrameRate FrameRate = Section.GetTypedOuter<UMovieScene>()->GetTickResolution();
	FFrameNumber StartOffset = FrameRate.AsFrameNumber((SlipTime - InitialStartTimeDuringResize) / FrameRate);

	StartOffset += InitialStartOffsetDuringResize;

	// Ensure start offset is not less than 0 and adjust ResizeTime
	if (StartOffset < 0)
	{
		SlipTime = SlipTime - StartOffset;

		StartOffset = FFrameNumber(0);
	}

	Section.Params.StartFrameOffset = StartOffset;

	ISequencerSection::SlipSection(SlipTime);
}

void FSkeletalAnimationRateSection::UpdateSectionData()
{
	if (!&Section || !IsValid(&Section)) return;

	const FFrameRate CompressFrameRate = UMovieSceneSkeletalAnimationRateSection::CompressFrameRate;
	const FFrameRate FrameRate = Section.GetTypedOuter<UMovieScene>()->GetTickResolution();

	const FFrameNumber StartFrame = FFrameRate::TransformTime(Section.GetInclusiveStartFrame(), FrameRate, CompressFrameRate).GetFrame();
	const FFrameNumber EndFrame = FFrameRate::TransformTime(Section.GetExclusiveEndFrame(), FrameRate, CompressFrameRate).GetFrame()-1;
	
	const float AnimationStartPosition = FrameRate.AsSeconds(Section.Params.StartFrameOffset);
	const float AnimationLength = Section.Params.GetSequenceLength();
	const float SeqLengthSeconds = AnimationLength - FrameRate.AsSeconds(Section.Params.StartFrameOffset + Section.Params.EndFrameOffset);

	const FFrameTime SequenceFrameLength = SeqLengthSeconds * CompressFrameRate;
	const double FrameIntervalTime = CompressFrameRate.AsInterval();

	if (SequenceFrameLength.FrameNumber > 1)
	{
		Section.Params.PlayPosition.Empty(SequenceFrameLength.FrameNumber.Value);

		// 跳过第一帧
		FFrameNumber CurrentFrameNumber = StartFrame + 1;
		float CurrentAnimationPosition = 0;
		Section.Params.PlayPosition.Add(AnimationStartPosition);
		// 遍历每一帧
		while (CurrentFrameNumber <= EndFrame)
		{
			float CurrentPlayRate;
			Section.Params.PlayRate.Evaluate(FFrameRate::TransformTime(CurrentFrameNumber, CompressFrameRate, FrameRate), CurrentPlayRate);
			CurrentAnimationPosition = FMath::Max(0.f, FMath::Fmod(CurrentAnimationPosition + FrameIntervalTime * CurrentPlayRate, SeqLengthSeconds));

			Section.Params.PlayPosition.Add(AnimationStartPosition + CurrentAnimationPosition);

			++CurrentFrameNumber;
		}
	}
}

bool FSkeletalAnimationRateSection::CreatePoseAsset(const TArray<UObject*> NewAssets, FGuid InObjectBinding)
{
	USkeletalMeshComponent* SkeletalMeshComponent = AcquireSkeletalMeshFromObjectGuid(InObjectBinding, Sequencer.Pin());

	bool bResult = false;
	if (NewAssets.Num() > 0)
	{
		for (auto NewAsset : NewAssets)
		{
			UPoseAsset* NewPoseAsset = Cast<UPoseAsset>(NewAsset);
			if (NewPoseAsset)
			{
				NewPoseAsset->AddOrUpdatePoseWithUniqueName(SkeletalMeshComponent);
				bResult = true;
			}
		}

		// if it contains error, warn them
		if (bResult)
		{
			FText NotificationText;
			if (NewAssets.Num() == 1)
			{
				NotificationText = FText::Format(LOCTEXT("NumPoseAssetsCreated", "{0} Pose assets created."), NewAssets.Num());
			}
			else
			{
				NotificationText = FText::Format(LOCTEXT("PoseAssetsCreated", "Pose asset created: '{0}'."), FText::FromString(NewAssets[0]->GetName()));
			}

			FNotificationInfo Info(NotificationText);
			Info.ExpireDuration = 8.0f;
			Info.bUseLargeFont = false;
			Info.Hyperlink = FSimpleDelegate::CreateLambda([NewAssets]()
				{
					FAssetEditorManager::Get().OpenEditorForAssets(NewAssets);
				});
			Info.HyperlinkText = FText::Format(LOCTEXT("OpenNewPoseAssetHyperlink", "Open {0}"), FText::FromString(NewAssets[0]->GetName()));

			TSharedPtr<SNotificationItem> Notification = FSlateNotificationManager::Get().AddNotification(Info);
			if (Notification.IsValid())
			{
				Notification->SetCompletionState(SNotificationItem::CS_Success);
			}
		}
		else
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FailedToCreateAsset", "Failed to create asset"));
		}
	}
	return bResult;
}


void FSkeletalAnimationRateSection::HandleCreatePoseAsset(FGuid InObjectBinding)
{
	USkeleton* Skeleton = AcquireSkeletonFromObjectGuid(InObjectBinding, Sequencer.Pin());
	if (Skeleton)
	{
		TArray<TWeakObjectPtr<UObject>> Skeletons;
		Skeletons.Add(Skeleton);
		AnimationEditorUtils::ExecuteNewAnimAsset<UPoseAssetFactory, UPoseAsset>(Skeletons, FString("_PoseAsset"), FAnimAssetCreated::CreateSP(this, &FSkeletalAnimationRateSection::CreatePoseAsset, InObjectBinding), false);
	}
}

void FSkeletalAnimationRateSection::BuildSectionContextMenu(FMenuBuilder& MenuBuilder, const FGuid& InObjectBinding)
{
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("SkeletonMenuText", "Skeleton"));

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CreatePoseAsset", "Create Pose Asset"),
		LOCTEXT("CreatePoseAsset_ToolTip", "Create Animation from current pose"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FSkeletalAnimationRateSection::HandleCreatePoseAsset, InObjectBinding)),
		NAME_None,
		EUserInterfaceActionType::Button);

	MenuBuilder.EndSection();
}

FSkeletalAnimationRateTrackEditor::FSkeletalAnimationRateTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{ }


TSharedRef<ISequencerTrackEditor> FSkeletalAnimationRateTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FSkeletalAnimationRateTrackEditor(InSequencer));
}


bool FSkeletalAnimationRateTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return Type == UMovieSceneSkeletalAnimationRateTrack::StaticClass();
}


TSharedRef<ISequencerSection> FSkeletalAnimationRateTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	check(SupportsType(SectionObject.GetOuter()->GetClass()));

	return MakeShareable(new FSkeletalAnimationRateSection(SectionObject, GetSequencer()));
}


void FSkeletalAnimationRateTrackEditor::AddKey(const FGuid& ObjectGuid)
{
	USkeleton* Skeleton = AcquireSkeletonFromObjectGuid(ObjectGuid, GetSequencer());

	if (Skeleton)
	{
		// Load the asset registry module
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		// Collect a full list of assets with the specified class
		TArray<FAssetData> AssetDataList;
		AssetRegistryModule.Get().GetAssetsByClass(UAnimSequenceBase::StaticClass()->GetFName(), AssetDataList, true);

		if (AssetDataList.Num())
		{
			TSharedPtr< SWindow > Parent = FSlateApplication::Get().GetActiveTopLevelWindow();
			if (Parent.IsValid())
			{
				FSlateApplication::Get().PushMenu(
					Parent.ToSharedRef(),
					FWidgetPath(),
					BuildAnimationSubMenu(ObjectGuid, Skeleton, nullptr),
					FSlateApplication::Get().GetCursorPos(),
					FPopupTransitionEffect(FPopupTransitionEffect::TypeInPopup)
				);
			}
		}
	}
}


bool FSkeletalAnimationRateTrackEditor::HandleAssetAdded(UObject* Asset, const FGuid& TargetObjectGuid)
{
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();

	if (Asset->IsA<UAnimSequenceBase>() && SequencerPtr.IsValid())
	{
		UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(Asset);

		if (TargetObjectGuid.IsValid() && AnimSequence->CanBeUsedInComposition())
		{
			USkeleton* Skeleton = AcquireSkeletonFromObjectGuid(TargetObjectGuid, GetSequencer());

			if (Skeleton && Skeleton == AnimSequence->GetSkeleton())
			{
				UObject* Object = SequencerPtr->FindSpawnedObjectOrTemplate(TargetObjectGuid);

				UMovieSceneTrack* Track = nullptr;

				const FScopedTransaction Transaction(LOCTEXT("AddAnimation_Transaction", "Add Animation"));

				int32 RowIndex = INDEX_NONE;
				AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FSkeletalAnimationRateTrackEditor::AddKeyInternal, Object, AnimSequence, Track, RowIndex));

				return true;
			}
		}
	}
	return false;
}


void FSkeletalAnimationRateTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass)
{
	if (ObjectClass->IsChildOf(USkeletalMeshComponent::StaticClass()) || ObjectClass->IsChildOf(AActor::StaticClass()))
	{
		const TSharedPtr<ISequencer> ParentSequencer = GetSequencer();

		USkeleton* Skeleton = AcquireSkeletonFromObjectGuid(ObjectBinding, GetSequencer());

		if (Skeleton)
		{
			// Load the asset registry module
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

			// Collect a full list of assets with the specified class
			TArray<FAssetData> AssetDataList;
			AssetRegistryModule.Get().GetAssetsByClass(UAnimSequenceBase::StaticClass()->GetFName(), AssetDataList, true);

			if (AssetDataList.Num())
			{
				UMovieSceneTrack* Track = nullptr;

				MenuBuilder.AddSubMenu(
					LOCTEXT("AddAnimation", "AnimationRate"), NSLOCTEXT("Sequencer", "AddAnimationTooltip", "Adds an animation track."),
					FNewMenuDelegate::CreateRaw(this, &FSkeletalAnimationRateTrackEditor::AddAnimationSubMenu, ObjectBinding, Skeleton, Track)
				);
			}
		}
	}
}

TSharedRef<SWidget> FSkeletalAnimationRateTrackEditor::BuildAnimationSubMenu(FGuid ObjectBinding, USkeleton* Skeleton, UMovieSceneTrack* Track)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	AddAnimationSubMenu(MenuBuilder, ObjectBinding, Skeleton, Track);

	return MenuBuilder.MakeWidget();
}

bool FSkeletalAnimationRateTrackEditor::ShouldFilterAsset(const FAssetData& AssetData)
{
	// we don't want montage
	if (AssetData.AssetClass == UAnimMontage::StaticClass()->GetFName())
	{
		return true;
	}

	const FString EnumString = AssetData.GetTagValueRef<FString>(GET_MEMBER_NAME_CHECKED(UAnimSequence, AdditiveAnimType));
	if (EnumString.IsEmpty())
	{
		return false;
	}

	UEnum* AdditiveTypeEnum = StaticEnum<EAdditiveAnimationType>();
	return ((EAdditiveAnimationType)AdditiveTypeEnum->GetValueByName(*EnumString) == AAT_RotationOffsetMeshSpace);
}

void FSkeletalAnimationRateTrackEditor::AddAnimationSubMenu(FMenuBuilder& MenuBuilder, FGuid ObjectBinding, USkeleton* Skeleton, UMovieSceneTrack* Track)
{
	FAssetPickerConfig AssetPickerConfig;
	{
		AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateRaw(this, &FSkeletalAnimationRateTrackEditor::OnAnimationAssetSelected, ObjectBinding, Track);
		AssetPickerConfig.OnAssetEnterPressed = FOnAssetEnterPressed::CreateRaw(this, &FSkeletalAnimationRateTrackEditor::OnAnimationAssetEnterPressed, ObjectBinding, Track);
		AssetPickerConfig.bAllowNullSelection = false;
		AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
		AssetPickerConfig.OnShouldFilterAsset = FOnShouldFilterAsset::CreateRaw(this, &FSkeletalAnimationRateTrackEditor::ShouldFilterAsset);
		AssetPickerConfig.Filter.bRecursiveClasses = true;
		AssetPickerConfig.Filter.ClassNames.Add(UAnimSequenceBase::StaticClass()->GetFName());
		AssetPickerConfig.Filter.TagsAndValues.Add(TEXT("Skeleton"), FAssetData(Skeleton).GetExportTextName());
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	TSharedPtr<SBox> MenuEntry = SNew(SBox)
		.WidthOverride(300.0f)
		.HeightOverride(300.f)
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		];

	MenuBuilder.AddWidget(MenuEntry.ToSharedRef(), FText::GetEmpty(), true);
}


void FSkeletalAnimationRateTrackEditor::OnAnimationAssetSelected(const FAssetData& AssetData, FGuid ObjectBinding, UMovieSceneTrack* Track)
{
	FSlateApplication::Get().DismissAllMenus();

	UObject* SelectedObject = AssetData.GetAsset();
	TSharedPtr<ISequencer> SequencerPtr = GetSequencer();

	if (SelectedObject && SelectedObject->IsA(UAnimSequenceBase::StaticClass()) && SequencerPtr.IsValid())
	{
		UAnimSequenceBase* AnimSequence = CastChecked<UAnimSequenceBase>(AssetData.GetAsset());

		UObject* Object = SequencerPtr->FindSpawnedObjectOrTemplate(ObjectBinding);
		int32 RowIndex = INDEX_NONE;
		AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FSkeletalAnimationRateTrackEditor::AddKeyInternal, Object, AnimSequence, Track, RowIndex));
	}
}

void FSkeletalAnimationRateTrackEditor::OnAnimationAssetEnterPressed(const TArray<FAssetData>& AssetData, FGuid ObjectBinding, UMovieSceneTrack* Track)
{
	if (AssetData.Num() > 0)
	{
		OnAnimationAssetSelected(AssetData[0].GetAsset(), ObjectBinding, Track);
	}
}


FKeyPropertyResult FSkeletalAnimationRateTrackEditor::AddKeyInternal(FFrameNumber KeyTime, UObject* Object, class UAnimSequenceBase* AnimSequence, UMovieSceneTrack* Track, int32 RowIndex)
{
	FKeyPropertyResult KeyPropertyResult;

	FFindOrCreateHandleResult HandleResult = FindOrCreateHandleToObject(Object);
	FGuid ObjectHandle = HandleResult.Handle;
	KeyPropertyResult.bHandleCreated |= HandleResult.bWasCreated;
	if (ObjectHandle.IsValid())
	{
		if (!Track)
		{
			Track = AddTrack(GetSequencer()->GetFocusedMovieSceneSequence()->GetMovieScene(), ObjectHandle, UMovieSceneSkeletalAnimationRateTrack::StaticClass(), NAME_None);
			KeyPropertyResult.bTrackCreated = true;
		}

		if (ensure(Track))
		{
			Track->Modify();

			UMovieSceneSection* NewSection = Cast<UMovieSceneSkeletalAnimationRateTrack>(Track)->AddNewAnimationOnRow(KeyTime, AnimSequence, RowIndex);
			KeyPropertyResult.bTrackModified = true;

			GetSequencer()->EmptySelection();
			GetSequencer()->SelectSection(NewSection);
			GetSequencer()->ThrobSectionSelection();
		}
	}

	return KeyPropertyResult;
}

//  FMatineeImportTools 方法
//bool CopyInterpAnimControlTrack(UInterpTrackAnimControl* MatineeAnimControlTrack, UMovieSceneSkeletalAnimationRateTrack* SkeletalAnimationTrack, FFrameNumber EndPlaybackRange)
//{
//	// @todo - Sequencer - Add support for slot names once they are implemented.
//	const FScopedTransaction Transaction(NSLOCTEXT("Sequencer", "PasteMatineeAnimTrack", "Paste Matinee Anim Track"));
//	bool bSectionCreated = false;
//
//	FFrameRate FrameRate = SkeletalAnimationTrack->GetTypedOuter<UMovieScene>()->GetTickResolution();
//
//	SkeletalAnimationTrack->Modify();
//	SkeletalAnimationTrack->RemoveAllAnimationData();
//
//	for (int32 i = 0; i < MatineeAnimControlTrack->AnimSeqs.Num(); i++)
//	{
//		const auto& AnimSeq = MatineeAnimControlTrack->AnimSeqs[i];
//
//		float EndTime;
//		if (AnimSeq.bLooping)
//		{
//			if (i < MatineeAnimControlTrack->AnimSeqs.Num() - 1)
//			{
//				EndTime = MatineeAnimControlTrack->AnimSeqs[i + 1].StartTime;
//			}
//			else
//			{
//				EndTime = EndPlaybackRange / FrameRate;
//			}
//		}
//		else
//		{
//			EndTime = AnimSeq.StartTime + (((AnimSeq.AnimSeq->SequenceLength - AnimSeq.AnimEndOffset) - AnimSeq.AnimStartOffset) / AnimSeq.AnimPlayRate);
//
//			// Clamp to next clip's start time
//			if (i + 1 < MatineeAnimControlTrack->AnimSeqs.Num())
//			{
//				float NextStartTime = MatineeAnimControlTrack->AnimSeqs[i + 1].StartTime;
//				EndTime = FMath::Min(NextStartTime, EndTime);
//			}
//		}
//
//		UMovieSceneSkeletalAnimationRateSection* NewSection = Cast<UMovieSceneSkeletalAnimationRateSection>(SkeletalAnimationTrack->CreateNewSection());
//		NewSection->SetRange(TRange<FFrameNumber>((AnimSeq.StartTime * FrameRate).RoundToFrame(), (EndTime * FrameRate).RoundToFrame() + 1));
//		NewSection->Params.StartFrameOffset = FrameRate.AsFrameNumber(AnimSeq.AnimStartOffset);
//		NewSection->Params.EndFrameOffset = FrameRate.AsFrameNumber(AnimSeq.AnimEndOffset);
//		NewSection->Params.PlayRate = AnimSeq.AnimPlayRate;
//		NewSection->Params.Animation = AnimSeq.AnimSeq;
//		NewSection->Params.SlotName = MatineeAnimControlTrack->SlotName;
//
//		SkeletalAnimationTrack->AddSection(*NewSection);
//		bSectionCreated = true;
//	}
//
//	return bSectionCreated;
//}

//void CopyInterpAnimControlTrack2(TSharedRef<ISequencer> Sequencer, UInterpTrackAnimControl* MatineeAnimControlTrack, UMovieSceneSkeletalAnimationRateTrack* SkeletalAnimationRateTrack)
//{
//	FFrameNumber EndPlaybackRange = MovieScene::DiscreteExclusiveUpper(Sequencer.Get().GetFocusedMovieSceneSequence()->GetMovieScene()->GetPlaybackRange());
//
//	if (CopyInterpAnimControlTrack(MatineeAnimControlTrack, SkeletalAnimationRateTrack, EndPlaybackRange))
//	{
//		Sequencer.Get().NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
//	}
//}

//void FSkeletalAnimationRateTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
//{
//	UInterpTrackAnimControl* MatineeAnimControlTrack = nullptr;
//	for (UObject* CopyPasteObject : GUnrealEd->MatineeCopyPasteBuffer)
//	{
//		MatineeAnimControlTrack = Cast<UInterpTrackAnimControl>(CopyPasteObject);
//		if (MatineeAnimControlTrack != nullptr)
//		{
//			break;
//		}
//	}
//	UMovieSceneSkeletalAnimationRateTrack* SkeletalAnimationRateTrack = Cast<UMovieSceneSkeletalAnimationRateTrack>(Track);
//	MenuBuilder.AddMenuEntry(
//		NSLOCTEXT("Sequencer", "PasteMatineeAnimControlTrack", "Paste Matinee SkeletalAnimationRate Track"),
//		NSLOCTEXT("Sequencer", "PasteMatineeAnimControlTrackTooltip", "Pastes keys from a Matinee float track into this track."),
//		FSlateIcon(),
//		FUIAction(
//			FExecuteAction::CreateStatic(&CopyInterpAnimControlTrack2, GetSequencer().ToSharedRef(), MatineeAnimControlTrack, SkeletalAnimationRateTrack),
//			FCanExecuteAction::CreateLambda([=]()->bool { return MatineeAnimControlTrack != nullptr && MatineeAnimControlTrack->AnimSeqs.Num() > 0 && SkeletalAnimationRateTrack != nullptr; })));
//}

TSharedPtr<SWidget> FSkeletalAnimationRateTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	USkeleton* Skeleton = AcquireSkeletonFromObjectGuid(ObjectBinding, GetSequencer());

	if (Skeleton)
	{
		// Create a container edit box
		return SNew(SHorizontalBox)

			// Add the animation combo box
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				FSequencerUtilities::MakeAddButton(LOCTEXT("AnimationText", "Animation"), FOnGetContent::CreateSP(this, &FSkeletalAnimationRateTrackEditor::BuildAnimationSubMenu, ObjectBinding, Skeleton, Track), Params.NodeIsHovered, GetSequencer())
			];
	}

	else
	{
		return TSharedPtr<SWidget>();
	}
}

bool FSkeletalAnimationRateTrackEditor::OnAllowDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid)
{
	if (!Track->IsA(UMovieSceneSkeletalAnimationRateTrack::StaticClass()))
	{
		return false;
	}

	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();

	if (!Operation.IsValid() || !Operation->IsOfType<FAssetDragDropOp>())
	{
		return false;
	}

	if (!TargetObjectGuid.IsValid())
	{
		return false;
	}

	USkeleton* Skeleton = AcquireSkeletonFromObjectGuid(TargetObjectGuid, GetSequencer());

	TSharedPtr<FAssetDragDropOp> DragDropOp = StaticCastSharedPtr<FAssetDragDropOp>(Operation);

	for (const FAssetData& AssetData : DragDropOp->GetAssets())
	{
		UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(AssetData.GetAsset());

		const bool bValidAnimSequence = AnimSequence && AnimSequence->CanBeUsedInComposition();
		if (bValidAnimSequence && Skeleton && Skeleton == AnimSequence->GetSkeleton())
		{
			return true;
		}
	}

	return false;
}


FReply FSkeletalAnimationRateTrackEditor::OnDrop(const FDragDropEvent& DragDropEvent, UMovieSceneTrack* Track, int32 RowIndex, const FGuid& TargetObjectGuid)
{
	if (!Track->IsA(UMovieSceneSkeletalAnimationRateTrack::StaticClass()))
	{
		return FReply::Unhandled();
	}

	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();

	if (!Operation.IsValid() || !Operation->IsOfType<FAssetDragDropOp>())
	{
		return FReply::Unhandled();
	}

	if (!TargetObjectGuid.IsValid())
	{
		return FReply::Unhandled();
	}

	USkeleton* Skeleton = AcquireSkeletonFromObjectGuid(TargetObjectGuid, GetSequencer());

	TSharedPtr<FAssetDragDropOp> DragDropOp = StaticCastSharedPtr<FAssetDragDropOp>(Operation);

	bool bAnyDropped = false;
	for (const FAssetData& AssetData : DragDropOp->GetAssets())
	{
		UAnimSequenceBase* AnimSequence = Cast<UAnimSequenceBase>(AssetData.GetAsset());
		const bool bValidAnimSequence = AnimSequence && AnimSequence->CanBeUsedInComposition();
		if (bValidAnimSequence && Skeleton && Skeleton == AnimSequence->GetSkeleton())
		{
			UObject* Object = GetSequencer()->FindSpawnedObjectOrTemplate(TargetObjectGuid);

			AnimatablePropertyChanged(FOnKeyProperty::CreateRaw(this, &FSkeletalAnimationRateTrackEditor::AddKeyInternal, Object, AnimSequence, Track, RowIndex));

			bAnyDropped = true;
		}
	}

	return bAnyDropped ? FReply::Handled() : FReply::Unhandled();
}

#undef LOCTEXT_NAMESPACE
