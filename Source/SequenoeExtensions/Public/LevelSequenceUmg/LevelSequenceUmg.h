// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LevelSequenceUmg.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class SEQUENOEEXTENSIONS_API ULevelSequenceUmg : public UUserWidget
{
public:
	GENERATED_BODY()


private:
	TWeakPtr<SWidget> EditorPreviewWidget;
	TWeakPtr<SWidget> EditorPreviewOverlay;
public:
	void AddToEditorViewport(int32 ZOrder = 0);

	virtual void RemoveFromParent() override;
};
