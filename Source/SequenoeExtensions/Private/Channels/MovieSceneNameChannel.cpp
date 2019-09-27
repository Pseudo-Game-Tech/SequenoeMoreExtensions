// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Channels/MovieSceneNameChannel.h"
#include "Curves/NameCurve.h"
#include "MovieSceneFwd.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "MovieSceneFrameMigration.h"

const FName* FMovieSceneNameChannel::Evaluate(FFrameTime InTime) const
{
	if (Times.Num())
	{
		const int32 Index = FMath::Max(0, Algo::UpperBound(Times, InTime.FrameNumber) - 1);
		return &Values[Index];
	}

	return bHasDefaultValue ? &DefaultValue : nullptr;
}

bool FMovieSceneNameChannel::SerializeFromMismatchedTag(const FPropertyTag& Tag, FStructuredArchive::FSlot Slot)
{
	static const FName StringCurveName("StringCurve");
	if (Tag.Type == NAME_StructProperty && Tag.StructName == StringCurveName)
	{
		FNameCurve StringCurve;
		FNameCurve::StaticStruct()->SerializeItem(Slot, &StringCurve, nullptr);

		FName NewDefault = NAME_None;
		if (true)
		{
			bHasDefaultValue = true;
			DefaultValue = MoveTemp(NewDefault);
		}

		FFrameRate LegacyFrameRate = GetLegacyConversionFrameRate();

		Times.Reserve(StringCurve.GetNumKeys());
		Values.Reserve(StringCurve.GetNumKeys());
		int32 Index = 0;
		for (const FNameCurveKey& Key : StringCurve.GetKeys())
		{
			FFrameNumber KeyTime = UpgradeLegacyMovieSceneTime(nullptr, LegacyFrameRate, Key.Time);

			FName Val(Key.Value);
			ConvertInsertAndSort<FName>(Index++, KeyTime, Val, Times, Values);
		}
		return true;
	}

	return false;
}

void FMovieSceneNameChannel::GetKeys(const TRange<FFrameNumber>& WithinRange, TArray<FFrameNumber>* OutKeyTimes, TArray<FKeyHandle>* OutKeyHandles)
{
	GetData().GetKeys(WithinRange, OutKeyTimes, OutKeyHandles);
}

void FMovieSceneNameChannel::GetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<FFrameNumber> OutKeyTimes)
{
	GetData().GetKeyTimes(InHandles, OutKeyTimes);
}

void FMovieSceneNameChannel::SetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<const FFrameNumber> InKeyTimes)
{
	GetData().SetKeyTimes(InHandles, InKeyTimes);
}

void FMovieSceneNameChannel::DuplicateKeys(TArrayView<const FKeyHandle> InHandles, TArrayView<FKeyHandle> OutNewHandles)
{
	GetData().DuplicateKeys(InHandles, OutNewHandles);
}

void FMovieSceneNameChannel::DeleteKeys(TArrayView<const FKeyHandle> InHandles)
{
	GetData().DeleteKeys(InHandles);
}

void FMovieSceneNameChannel::ChangeFrameResolution(FFrameRate SourceRate, FFrameRate DestinationRate)
{
	GetData().ChangeFrameResolution(SourceRate, DestinationRate);
}

TRange<FFrameNumber> FMovieSceneNameChannel::ComputeEffectiveRange() const
{
	return GetData().GetTotalRange();
}

int32 FMovieSceneNameChannel::GetNumKeys() const
{
	return Times.Num();
}

void FMovieSceneNameChannel::Reset()
{
	Times.Reset();
	Values.Reset();
	KeyHandles.Reset();
	bHasDefaultValue = false;
}

void FMovieSceneNameChannel::Optimize(const FKeyDataOptimizationParams& InParameters)
{
	MovieScene::Optimize(this, InParameters);
}

void FMovieSceneNameChannel::Offset(FFrameNumber DeltaPosition)
{
	GetData().Offset(DeltaPosition);
}

void FMovieSceneNameChannel::ClearDefault()
{
	bHasDefaultValue = false;
}