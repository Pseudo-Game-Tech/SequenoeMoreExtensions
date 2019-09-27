// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "Misc/FrameNumber.h"
#include "UObject/ObjectMacros.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelData.h"
#include "Channels/MovieSceneChannelTraits.h"
#include "MovieSceneTextChannel.generated.h"

USTRUCT()
struct SEQUENOEEXTENSIONS_API FMovieSceneTextChannel : public FMovieSceneChannel
{
	GENERATED_BODY()

		FMovieSceneTextChannel()
		: DefaultValue(), bHasDefaultValue(false)
	{}

	/**
	 * Serialize this type from another
	 */
	bool SerializeFromMismatchedTag(const FPropertyTag& Tag, FStructuredArchive::FSlot Slot);

	/**
	 * Access a mutable interface for this channel's data
	 *
	 * @return An object that is able to manipulate this channel's data
	 */
	FORCEINLINE TMovieSceneChannelData<FText> GetData()
	{
		return TMovieSceneChannelData<FText>(&Times, &Values, &KeyHandles);
	}

	/**
	 * Access a constant interface for this channel's data
	 *
	 * @return An object that is able to interrogate this channel's data
	 */
	FORCEINLINE TMovieSceneChannelData<const FText> GetData() const
	{
		return TMovieSceneChannelData<const FText>(&Times, &Values);
	}

	/**
	 * Evaluate this channel
	 *
	 * @param InTime     The time to evaluate at
	 * @return A pointer to the string, or nullptr
	 */
	const FText* Evaluate(FFrameTime InTime) const;

public:

	// ~ FMovieSceneChannel Interface
	virtual void GetKeys(const TRange<FFrameNumber>& WithinRange, TArray<FFrameNumber>* OutKeyTimes, TArray<FKeyHandle>* OutKeyHandles) override;
	virtual void GetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<FFrameNumber> OutKeyTimes) override;
	virtual void SetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<const FFrameNumber> InKeyTimes) override;
	virtual void DuplicateKeys(TArrayView<const FKeyHandle> InHandles, TArrayView<FKeyHandle> OutNewHandles) override;
	virtual void DeleteKeys(TArrayView<const FKeyHandle> InHandles) override;
	virtual void ChangeFrameResolution(FFrameRate SourceRate, FFrameRate DestinationRate) override;
	virtual TRange<FFrameNumber> ComputeEffectiveRange() const override;
	virtual int32 GetNumKeys() const override;
	virtual void Reset() override;
	virtual void Offset(FFrameNumber DeltaPosition) override;
	virtual void Optimize(const FKeyDataOptimizationParams& InParameters) override;
	virtual void ClearDefault() override;

public:

	/**
	 * Set this channel's default value that should be used when no keys are present
	 *
	 * @param InDefaultValue The desired default value
	 */
	FORCEINLINE void SetDefault(FText InDefaultValue)
	{
		bHasDefaultValue = true;
		DefaultValue = InDefaultValue;
	}

	/**
	 * Get this channel's default value that will be used when no keys are present
	 *
	 * @return (Optional) The channel's default value
	 */
	FORCEINLINE TOptional<FText> GetDefault() const
	{
		return bHasDefaultValue ? TOptional<FText>(DefaultValue) : TOptional<FText>();
	}

	/**
	 * Remove this channel's default value causing the channel to have no effect where no keys are present
	 */
	FORCEINLINE void RemoveDefault()
	{
		bHasDefaultValue = false;
	}

private:

	UPROPERTY(meta = (KeyTimes))
		TArray<FFrameNumber> Times;

	/** Array of values that correspond to each key time */
	UPROPERTY(meta = (KeyValues))
		TArray<FText> Values;

	/** Default value used when there are no keys */
	UPROPERTY()
		FText DefaultValue;

	/** */
	UPROPERTY()
		bool bHasDefaultValue;

	FMovieSceneKeyHandleMap KeyHandles;
};

template<>
struct TStructOpsTypeTraits<FMovieSceneTextChannel> : public TStructOpsTypeTraitsBase2<FMovieSceneTextChannel>
{
	enum { WithStructuredSerializeFromMismatchedTag = true };
};


template<>
struct TMovieSceneChannelTraits<FMovieSceneTextChannel> : TMovieSceneChannelTraitsBase<FMovieSceneTextChannel>
{
#if WITH_EDITOR

	/** String channels can have external values (ie, they can get their values from external objects for UI purposes) */
	typedef TMovieSceneExternalValue<FText> ExtendedEditorDataType;

#endif
};

inline bool EvaluateChannel(const FMovieSceneTextChannel* InChannel, FFrameTime InTime, FText& OutValue)
{
	if (const FText* Result = InChannel->Evaluate(InTime))
	{
		OutValue = *Result;
		return true;
	}
	return false;
}

inline bool ValueExistsAtTime(const FMovieSceneTextChannel* Channel, FFrameNumber InFrameNumber, const FText& Value)
{
	const FFrameTime FrameTime(InFrameNumber);

	const FText* ExistingValue = Channel->Evaluate(FrameTime);
	return ExistingValue && Value.CompareTo(*ExistingValue) == 0;
}

