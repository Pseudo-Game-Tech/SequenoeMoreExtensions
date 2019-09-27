#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Class.h"
#include "Curves/KeyHandle.h"
#include "Curves/IndexedCurve.h"
#include "TextCurve.generated.h"

USTRUCT()
struct FMyKeyHandleMap
{
	GENERATED_USTRUCT_BODY()
public:
	FMyKeyHandleMap() {}

	// This struct is not copyable.  This must be public or because derived classes are allowed to be copied
	FMyKeyHandleMap(const FMyKeyHandleMap& Other) {}
	void operator=(const FMyKeyHandleMap& Other) {}

	/** TMap functionality */
	void Add(const FKeyHandle& InHandle, int32 InIndex);
	void Empty();
	void Remove(const FKeyHandle& InHandle);
	const int32* Find(const FKeyHandle& InHandle) const { return KeyHandlesToIndices.Find(InHandle); }
	const FKeyHandle* FindKey(int32 KeyIndex) const;
	int32 Num() const { return KeyHandlesToIndices.Num(); }
	TArray<FKeyHandle>::TConstIterator CreateConstIterator() const { return KeyHandles.CreateConstIterator(); }
	const TMap<FKeyHandle, int32>& GetMap() const { return KeyHandlesToIndices; }

	/** ICPPStructOps implementation */
	bool Serialize(FArchive& Ar);
	bool operator==(const FMyKeyHandleMap& Other) const { return KeyHandles == Other.KeyHandles; }
	bool operator!=(const FMyKeyHandleMap& Other) const { return !(*this == Other); }

	friend FArchive& operator<<(FArchive& Ar, FMyKeyHandleMap& P)
	{
		P.Serialize(Ar);
		return Ar;
	}

	// Ensures that all indices have a valid handle and that there are no handles left to invalid indices
	void EnsureAllIndicesHaveHandles(int32 NumIndices);

	// Ensures a handle exists for the specified Index
	void EnsureIndexHasAHandle(int32 KeyIndex);

private:

	TMap<FKeyHandle, int32> KeyHandlesToIndices;
	TArray<FKeyHandle> KeyHandles;
};

/**
 * One key in a curve of FTexts.
 */
USTRUCT()
struct SEQUENOEEXTENSIONS_API FTextCurveKey
{
	GENERATED_USTRUCT_BODY()

	/** Time at this key */
	UPROPERTY(EditAnywhere, Category="Key")
	float Time;

	/** Value at this key */
	UPROPERTY(EditAnywhere, Category="Key")
	FText Value;

	/** Default constructor. */
	FTextCurveKey()
		: Time(0.0f)
		, Value()
	{ }

	/** Creates and initializes a new instance. */
	FTextCurveKey(float InTime, const FText& InValue)
		: Time(InTime)
		, Value(InValue)
	{ }

public:

	// TStructOpsTypeTraits interface

	bool operator==(const FTextCurveKey& Other) const;
	bool operator!=(const FTextCurveKey& Other) const;
	bool Serialize(FArchive& Ar);

	/** Serializes a name curve key from or into an archive. */
	friend FArchive& operator<<(FArchive& Ar, FTextCurveKey& Key)
	{
		Key.Serialize(Ar);
		return Ar;
	}
};


template<>
struct TIsPODType<FTextCurveKey>
{
	enum { Value = true };
};


template<>
struct TStructOpsTypeTraits<FTextCurveKey>
	: public TStructOpsTypeTraitsBase2<FTextCurveKey>
{
	enum
	{
		WithSerializer = true,
		WithCopy = false,
		WithIdenticalViaEquality = true,
	};
};


/**
 * Implements a curve of FTexts.
 */
USTRUCT()
struct SEQUENOEEXTENSIONS_API FTextCurve
	: public FIndexedCurve
{
	GENERATED_USTRUCT_BODY()

	/** Virtual destructor. */
	virtual ~FTextCurve() { }

public:

	/**
	  * Add a new key to the curve with the supplied Time and Value. Returns the handle of the new key.
	  * 
	  * @param InTime The time at which to add the key.
	  * @param InValue The value of the key.
	  * @param KeyHandle Optional handle for the new key.
	  */
	FKeyHandle AddKey(float InTime, const FText& InValue, FKeyHandle KeyHandle = FKeyHandle());

	/**
	 * Remove the specified key from the curve.
	 *
	 * @param KeyHandle Handle to the key to remove.
	 */
	void DeleteKey(FKeyHandle KeyHandle);

	/**
	 * Finds a key a the specified time.
	 *
	 * @param KeyTime The time at which to find the key.
	 * @param KeyTimeTolerance The key time tolerance to use for equality.
	 * @return A handle to the key, or invalid key handle if not found.
	 */
	FKeyHandle FindKey(float KeyTime, float KeyTimeTolerance = KINDA_SMALL_NUMBER) const;

	/**
	 * Get a key.
	 *
	 * @param KeyHandle The handle of the key to get.
	 * @return The key.
	 */
	FTextCurveKey& GetKey(FKeyHandle KeyHandle);
	FTextCurveKey GetKey(FKeyHandle KeyHandle) const;

	/**
	 * Read-only access to the key collection.
	 *
	 * @return Collection of keys.
	 */
	const TArray<FTextCurveKey>& GetKeys() const
	{
		return Keys;
	}

	/**
	 * Get the time for the Key with the specified index.
	 *
	 * @param KeyHandle Handle to the key whose time to get.
	 * @return The key's time.
	 */
	virtual float GetKeyTime(FKeyHandle KeyHandle) const override final;

	/**
	 * Move a key to a new time.
	 *
	 * @param KeyHandle The handle of the key to change.
	 * @param NewTime The new time to set on the key.
	 */
	virtual void SetKeyTime(FKeyHandle KeyHandle, float NewTime) override final;

	/**
	 * Finds the key at InTime, and updates its value. If it can't find the key within the KeyTimeTolerance, it adds one at that time.
	 *
	 * @param InTime The time at which the key should be added or updated.
	 * @param InValue The value of the key.
	 * @param KeyTimeTolerance The tolerance used for key time equality.
	 */
	FKeyHandle UpdateOrAddKey(float InTime, const FText& InValue, float KeyTimeTolerance = KINDA_SMALL_NUMBER);

public:

	// FIndexedCurve interface

	virtual int32 GetNumKeys() const override final { return Keys.Num(); }

	/** Allocates a duplicate of the curve */
	virtual FIndexedCurve* Duplicate() const final { return new FTextCurve(*this); }

public:

	/** Sorted array of keys */
	UPROPERTY(EditAnywhere, EditFixedSize, Category="Curve")
	TArray<FTextCurveKey> Keys;
};
