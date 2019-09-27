// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Containers/UnrealString.h"
#include "Misc/FrameNumber.h"
#include "UObject/ObjectMacros.h"
#include "Channels/MovieSceneChannel.h"
#include "Channels/MovieSceneChannelData.h"
#include "Channels/MovieSceneChannelTraits.h"
#include "MovieSceneNameChannel.generated.h"

USTRUCT()
struct SEQUENOEEXTENSIONS_API FMovieSceneNameChannel : public FMovieSceneChannel
{
	GENERATED_BODY()

		FMovieSceneNameChannel()
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
	FORCEINLINE TMovieSceneChannelData<FName> GetData()
	{
		return TMovieSceneChannelData<FName>(&Times, &Values, &KeyHandles);
	}

	/**
	 * Access a constant interface for this channel's data
	 *
	 * @return An object that is able to interrogate this channel's data
	 */
	FORCEINLINE TMovieSceneChannelData<const FName> GetData() const
	{
		return TMovieSceneChannelData<const FName>(&Times, &Values);
	}

	/**
	 * Evaluate this channel
	 *
	 * @param InTime     The time to evaluate at
	 * @return A pointer to the string, or nullptr
	 */
	const FName* Evaluate(FFrameTime InTime) const;

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
	FORCEINLINE void SetDefault(FName InDefaultValue)
	{
		bHasDefaultValue = true;
		DefaultValue = InDefaultValue;
	}

	/**
	 * Get this channel's default value that will be used when no keys are present
	 *
	 * @return (Optional) The channel's default value
	 */
	FORCEINLINE TOptional<FName> GetDefault() const
	{
		return bHasDefaultValue ? TOptional<FName>(DefaultValue) : TOptional<FName>();
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
		TArray<FName> Values;

	/** Default value used when there are no keys */
	UPROPERTY()
		FName DefaultValue;

	/** */
	UPROPERTY()
		bool bHasDefaultValue;

	FMovieSceneKeyHandleMap KeyHandles;
};

template<>
struct TStructOpsTypeTraits<FMovieSceneNameChannel> : public TStructOpsTypeTraitsBase2<FMovieSceneNameChannel>
{
	enum { WithStructuredSerializeFromMismatchedTag = true };
};


template<>
struct TMovieSceneChannelTraits<FMovieSceneNameChannel> : TMovieSceneChannelTraitsBase<FMovieSceneNameChannel>
{
#if WITH_EDITOR

	/** String channels can have external values (ie, they can get their values from external objects for UI purposes) */
	typedef TMovieSceneExternalValue<FName> ExtendedEditorDataType;

#endif
};

inline bool EvaluateChannel(const FMovieSceneNameChannel* InChannel, FFrameTime InTime, FName& OutValue)
{
	if (const FName* Result = InChannel->Evaluate(InTime))
	{
		OutValue = *Result;
		return true;
	}
	return false;
}

inline bool ValueExistsAtTime(const FMovieSceneNameChannel* Channel, FFrameNumber InFrameNumber, const FName& Value)
{
	const FFrameTime FrameTime(InFrameNumber);

	const FName* ExistingValue = Channel->Evaluate(FrameTime);
	return ExistingValue && Value == *ExistingValue;
}

