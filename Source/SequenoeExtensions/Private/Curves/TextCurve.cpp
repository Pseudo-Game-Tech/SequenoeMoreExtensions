#include "Curves/TextCurve.h"


/* FKeyHandleMap interface
 *****************************************************************************/


void FMyKeyHandleMap::Add(const FKeyHandle& InHandle, int32 InIndex)
{
	for (auto It = KeyHandlesToIndices.CreateIterator(); It; ++It)
	{
		int32& KeyIndex = It.Value();
		if (KeyIndex >= InIndex) { ++KeyIndex; }
	}

	if (InIndex > KeyHandles.Num())
	{
		KeyHandles.Reserve(InIndex + 1);
		for (int32 NewIndex = KeyHandles.Num(); NewIndex < InIndex; ++NewIndex)
		{
			KeyHandles.AddDefaulted();
			KeyHandlesToIndices.Add(KeyHandles.Last(), NewIndex);
		}
		KeyHandles.Add(InHandle);
	}
	else
	{
		KeyHandles.Insert(InHandle, InIndex);
	}

	KeyHandlesToIndices.Add(InHandle, InIndex);
}


void FMyKeyHandleMap::Empty()
{
	KeyHandlesToIndices.Empty();
	KeyHandles.Empty();
}


void FMyKeyHandleMap::Remove(const FKeyHandle& InHandle)
{
	int32 Index = INDEX_NONE;
	if (KeyHandlesToIndices.RemoveAndCopyValue(InHandle, Index))
	{
		// update key indices
		for (auto It = KeyHandlesToIndices.CreateIterator(); It; ++It)
		{
			int32& KeyIndex = It.Value();
			if (KeyIndex >= Index) { --KeyIndex; }
		}

		KeyHandles.RemoveAt(Index);
	}
}


/* FTextCurveKey interface
 *****************************************************************************/

bool FTextCurveKey::operator==(const FTextCurveKey& Curve) const
{
	return ((Time == Curve.Time) && (Value.CompareTo(Curve.Value) == 0));
}


bool FTextCurveKey::operator!=(const FTextCurveKey& Other) const
{
	return !(*this == Other);
}


bool FTextCurveKey::Serialize(FArchive& Ar)
{
	Ar << Time << Value;
	return true;
}


/* FTextCurve interface
 *****************************************************************************/

FKeyHandle FTextCurve::AddKey(float InTime, const FText& InValue, FKeyHandle KeyHandle)
{
	int32 Index = 0;

	// insert key
	for(; Index < Keys.Num() && Keys[Index].Time < InTime; ++Index);
	Keys.Insert(FTextCurveKey(InTime, InValue), Index);

	FMyKeyHandleMap& Map = reinterpret_cast<FMyKeyHandleMap&>(KeyHandlesToIndices);
	Map.Add(KeyHandle, Index);

	return GetKeyHandle(Index);
}


void FTextCurve::DeleteKey(FKeyHandle KeyHandle)
{
	// remove key
	int32 Index = GetIndex(KeyHandle);
	Keys.RemoveAt(Index);


	FMyKeyHandleMap& Map = reinterpret_cast<FMyKeyHandleMap&>(KeyHandlesToIndices);
	Map.Remove(KeyHandle);
}


FKeyHandle FTextCurve::FindKey(float KeyTime, float KeyTimeTolerance) const
{
	int32 Start = 0;
	int32 End = Keys.Num()-1;

	// Binary search since the keys are in sorted order
	while (Start <= End)
	{
		int32 TestPos = Start + (End-Start) / 2;
		float TestKeyTime = Keys[TestPos].Time;

		if (FMath::IsNearlyEqual(TestKeyTime, KeyTime, KeyTimeTolerance))
		{
			return GetKeyHandle(TestPos);
		}

		if (TestKeyTime < KeyTime)
		{
			Start = TestPos+1;
		}
		else
		{
			End = TestPos-1;
		}
	}

	return FKeyHandle();
}


FTextCurveKey& FTextCurve::GetKey(FKeyHandle KeyHandle)
{
	EnsureAllIndicesHaveHandles();
	return Keys[GetIndex(KeyHandle)];
}


FTextCurveKey FTextCurve::GetKey(FKeyHandle KeyHandle) const
{
	EnsureAllIndicesHaveHandles();
	return Keys[GetIndex(KeyHandle)];
}


float FTextCurve::GetKeyTime(FKeyHandle KeyHandle) const
{
	if (!IsKeyHandleValid(KeyHandle))
	{
		return 0.f;
	}

	return GetKey(KeyHandle).Time;
}


void FTextCurve::SetKeyTime(FKeyHandle KeyHandle, float NewTime)
{
	if (IsKeyHandleValid(KeyHandle))
	{
		const FTextCurveKey OldKey = GetKey(KeyHandle);

		DeleteKey(KeyHandle);
		AddKey(NewTime, OldKey.Value, KeyHandle);

		// Copy all properties from old key, but then fix time to be the new time
		FTextCurveKey& NewKey = GetKey(KeyHandle);
		NewKey = OldKey;
		NewKey.Time = NewTime;
	}
}

FKeyHandle FTextCurve::UpdateOrAddKey(float InTime, const FText& InValue, float KeyTimeTolerance)
{
	// Search for a key that already exists at the time and if found, update its value
	for (int32 KeyIndex = 0; KeyIndex < Keys.Num(); ++KeyIndex)
	{
		float KeyTime = Keys[KeyIndex].Time;

		if (FMath::IsNearlyEqual(KeyTime, InTime, KeyTimeTolerance))
		{
			Keys[KeyIndex].Value = InValue;

			return GetKeyHandle(KeyIndex);
		}

		if (KeyTime > InTime)
		{
			// All the rest of the keys exist after the key we want to add
			// so there is no point in searching
			break;
		}
	}

	// A key wasnt found, add it now
	return AddKey(InTime, InValue);
}
