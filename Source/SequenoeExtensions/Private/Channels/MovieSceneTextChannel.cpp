// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Channels/MovieSceneTextChannel.h"
#include "Curves/TextCurve.h"
#include "MovieSceneFwd.h"
#include "Channels/MovieSceneChannelProxy.h"
#include "MovieSceneFrameMigration.h"

const FText* FMovieSceneTextChannel::Evaluate(FFrameTime InTime) const
{
	if (Times.Num())
	{
		const int32 Index = FMath::Max(0, Algo::UpperBound(Times, InTime.FrameNumber) - 1);
		return &Values[Index];
	}

	return bHasDefaultValue ? &DefaultValue : nullptr;
}

bool FMovieSceneTextChannel::SerializeFromMismatchedTag(const FPropertyTag& Tag, FStructuredArchive::FSlot Slot)
{
	static const FName StringCurveName("StringCurve");
	if (Tag.Type == NAME_StructProperty && Tag.StructName == StringCurveName)
	{
		FTextCurve StringCurve;
		FTextCurve::StaticStruct()->SerializeItem(Slot, &StringCurve, nullptr);

		FText NewDefault = FText::GetEmpty();
		if (true)
		{
			bHasDefaultValue = true;
			DefaultValue = MoveTemp(NewDefault);
		}

		FFrameRate LegacyFrameRate = GetLegacyConversionFrameRate();

		Times.Reserve(StringCurve.GetNumKeys());
		Values.Reserve(StringCurve.GetNumKeys());
		int32 Index = 0;
		for (const FTextCurveKey& Key : StringCurve.GetKeys())
		{
			FFrameNumber KeyTime = UpgradeLegacyMovieSceneTime(nullptr, LegacyFrameRate, Key.Time);

			FText Val(Key.Value);
			ConvertInsertAndSort<FText>(Index++, KeyTime, Val, Times, Values);
		}
		return true;
	}

	return false;
}

void FMovieSceneTextChannel::GetKeys(const TRange<FFrameNumber>& WithinRange, TArray<FFrameNumber>* OutKeyTimes, TArray<FKeyHandle>* OutKeyHandles)
{
	GetData().GetKeys(WithinRange, OutKeyTimes, OutKeyHandles);
}

void FMovieSceneTextChannel::GetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<FFrameNumber> OutKeyTimes)
{
	GetData().GetKeyTimes(InHandles, OutKeyTimes);
}

void FMovieSceneTextChannel::SetKeyTimes(TArrayView<const FKeyHandle> InHandles, TArrayView<const FFrameNumber> InKeyTimes)
{
	GetData().SetKeyTimes(InHandles, InKeyTimes);
}

void FMovieSceneTextChannel::DuplicateKeys(TArrayView<const FKeyHandle> InHandles, TArrayView<FKeyHandle> OutNewHandles)
{
	GetData().DuplicateKeys(InHandles, OutNewHandles);
}

void FMovieSceneTextChannel::DeleteKeys(TArrayView<const FKeyHandle> InHandles)
{
	GetData().DeleteKeys(InHandles);
}

void FMovieSceneTextChannel::ChangeFrameResolution(FFrameRate SourceRate, FFrameRate DestinationRate)
{
	GetData().ChangeFrameResolution(SourceRate, DestinationRate);
}

TRange<FFrameNumber> FMovieSceneTextChannel::ComputeEffectiveRange() const
{
	return GetData().GetTotalRange();
}

int32 FMovieSceneTextChannel::GetNumKeys() const
{
	return Times.Num();
}

void FMovieSceneTextChannel::Reset()
{
	Times.Reset();
	Values.Reset();
	KeyHandles.Reset();
	bHasDefaultValue = false;
}

void FMovieSceneTextChannel::Optimize(const FKeyDataOptimizationParams& InParameters)
{
	MovieScene::Optimize(this, InParameters);
}

void FMovieSceneTextChannel::Offset(FFrameNumber DeltaPosition)
{
	GetData().Offset(DeltaPosition);
}

void FMovieSceneTextChannel::ClearDefault()
{
	bHasDefaultValue = false;
}