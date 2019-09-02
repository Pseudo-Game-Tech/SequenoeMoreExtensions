// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MovieSceneSection.h"
#include "Animation/AnimSequenceBase.h"
#include "Channels/MovieSceneFloatChannel.h"
#include "MovieSceneSkeletalAnimationRateSection.generated.h"

USTRUCT(BlueprintType)
struct FMovieSceneSkeletalAnimationRateParams
{
	GENERATED_BODY()

		FMovieSceneSkeletalAnimationRateParams();

	/** Gets the animation sequence length, not modified by play rate */
	float GetSequenceLength() const { return Animation != nullptr ? Animation->SequenceLength : 0.f; }

	/** The animation this section plays */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation", meta = (AllowedClasses = "AnimSequence, AnimComposite"))
		UAnimSequenceBase* Animation;

	/** The offset into the beginning of the animation clip */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		FFrameNumber StartFrameOffset;

	/** The offset into the end of the animation clip */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		FFrameNumber EndFrameOffset;

	/** The playback rate of the animation clip */
	UPROPERTY()
		FMovieSceneFloatChannel PlayRate;

	/** 片段中每帧动画播放得位置 (需要在关键帧或片段长度变化时重新计算更新) */
	UPROPERTY()
		TArray<float> PlayPosition;

	/** Reverse the playback of the animation clip */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		uint32 bReverse : 1;

	/** The slot name to use for the animation */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		FName SlotName;

	/** The weight curve for this animation section */
	UPROPERTY()
		FMovieSceneFloatChannel Weight;

	/** If on will skip sending animation notifies */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
		bool bSkipAnimNotifiers;

	/** If on animation sequence will always play when active even if the animation is controlled by a Blueprint or Anim Instance Class*/
	UPROPERTY(EditAnywhere, Category = "Animation")
		bool bForceCustomMode;

	UPROPERTY()
		float StartOffset_DEPRECATED;

	UPROPERTY()
		float EndOffset_DEPRECATED;
};

/**
 * Movie scene section that control skeletal animation
 */
UCLASS(MinimalAPI)
class UMovieSceneSkeletalAnimationRateSection
	: public UMovieSceneSection
{
	GENERATED_UCLASS_BODY()

public:

	/* 实际Params.PlayPosition中保存得帧率,减少数据量 */
	SEQUENOEEXTENSIONS_API const static FFrameRate CompressFrameRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation", meta = (ShowOnlyInnerProperties))
		FMovieSceneSkeletalAnimationRateParams Params;

	/** Get Frame Time as Animation Time*/
	SEQUENOEEXTENSIONS_API float MapTimeToAnimation(FFrameTime InPosition, FFrameRate InFrameRate) const;

protected:

	//~ UMovieSceneSection interface
	virtual TOptional<TRange<FFrameNumber> > GetAutoSizeRange() const override;
	virtual void TrimSection(FQualifiedFrameTime TrimTime, bool bTrimLeft) override;
	virtual UMovieSceneSection* SplitSection(FQualifiedFrameTime SplitTime) override;
	virtual void GetSnapTimes(TArray<FFrameNumber>& OutSnapTimes, bool bGetSectionBorders) const override;
	virtual TOptional<FFrameTime> GetOffsetTime() const override;
	virtual FMovieSceneEvalTemplatePtr GenerateTemplate() const override;
	virtual float GetTotalWeightValue(FFrameTime InTime) const override;
	/** ~UObject interface */
	virtual void PostLoad() override;
	virtual void Serialize(FArchive& Ar) override;

private:

	UPROPERTY()
		class UAnimSequence* AnimSequence_DEPRECATED;

	UPROPERTY()
		UAnimSequenceBase* Animation_DEPRECATED;

	UPROPERTY()
		float StartOffset_DEPRECATED;

	UPROPERTY()
		float EndOffset_DEPRECATED;

	UPROPERTY()
		float PlayRate_DEPRECATED;

	UPROPERTY()
		uint32 bReverse_DEPRECATED : 1;

	UPROPERTY()
		FName SlotName_DEPRECATED;
};
