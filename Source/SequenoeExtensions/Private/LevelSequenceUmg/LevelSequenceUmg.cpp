// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelSequenceUmg/LevelSequenceUmg.h"
#include "Engine.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Slate/SceneViewport.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

#define LOCTEXT_NAMESPACE "LevelSequenceUmg"

void ULevelSequenceUmg::AddToEditorViewport(int32 ZOrder)
{
#if WITH_EDITOR
	if (!EditorPreviewWidget.IsValid())
	{
		TSharedRef<SConstraintCanvas> FullScreenCanvas = SNew(SConstraintCanvas);
		EditorPreviewWidget = FullScreenCanvas;

		TSharedRef<SWidget> UserSlateWidget = TakeWidget();

		FullScreenCanvas->AddSlot()
			.Offset(BIND_UOBJECT_ATTRIBUTE(FMargin, GetFullScreenOffset))
			.Anchors(BIND_UOBJECT_ATTRIBUTE(FAnchors, GetAnchorsInViewport))
			.Alignment(BIND_UOBJECT_ATTRIBUTE(FVector2D, GetAlignmentInViewport))
			[
				UserSlateWidget
			];

		
		if (UEditorEngine * UEEngine = Cast<UEditorEngine>(GEngine))
		{
			if (FSceneViewport * EditorViewport = (FSceneViewport*)UEEngine->GetActiveViewport())
			{
				if (SOverlay * Overlay = (SOverlay*)EditorViewport->GetViewportWidget().Pin().Get()->GetContent().Get())
				{
					FChildren* Children = Overlay->GetChildren();
					int32 Num = Children->Num();
					SWidget* GameLayerManager = nullptr;
					for (int32 i = 0; i < Num; ++i)
					{
						SWidget& Child = Children->GetChildAt(i).Get();
						if (Child.GetType() == FName(TEXT("SGameLayerManager")))
						{
							GameLayerManager = &Child;
							break;
						}
					}
					SWidget& DPIScaler = GameLayerManager->GetChildren()->GetChildAt(0).Get();
					SWidget& VerticalBox = DPIScaler.GetChildren()->GetChildAt(0).Get();

					Children = VerticalBox.GetChildren();
					Num = Children->Num();
					for (int32 i = 0; i < Num; ++i)
					{
						SWidget& Child = Children->GetChildAt(i).Get();
						if (Child.GetType() == FName(TEXT("SOverlay")))
						{
							EditorPreviewOverlay = Children->GetChildAt(i);
							Overlay = (SOverlay*)& Child;
							break;
						}
					}
					Overlay->AddSlot()
						[
							FullScreenCanvas
						];
				}

			}
		}
	}
#endif
}

void ULevelSequenceUmg::RemoveFromParent()
{
#if WITH_EDITOR
	if (!HasAnyFlags(RF_BeginDestroyed))
	{
		if (EditorPreviewWidget.IsValid() && EditorPreviewOverlay.IsValid())
		{
			SOverlay* Overlay = (SOverlay*)EditorPreviewOverlay.Pin().Get();
			Overlay->RemoveSlot(EditorPreviewWidget.Pin().ToSharedRef());

			EditorPreviewWidget.Reset();
			EditorPreviewOverlay.Reset();
		}
	}
#endif
	Super::RemoveFromParent();
}

#undef LOCTEXT_NAMESPACE

