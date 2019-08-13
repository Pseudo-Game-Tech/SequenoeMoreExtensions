#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Misc/Guid.h"
#include "Curves/KeyHandle.h"
#include "MovieSceneSection.h"
#include "MovieSceneObjectBindingID.h"
#include "StaringSceneSection.generated.h"

UCLASS(MinimalAPI)
class UStaringSceneSection : public UMovieSceneSection
{
public:
	GENERATED_BODY()

	UStaringSceneSection();
public:
	UFUNCTION(BlueprintCallable, Category = "Movie Scene Section")
		const FMovieSceneObjectBindingID& GetStaringTargetBindingID() const
	{
		return StaringTargetBindingID;
	};

	UFUNCTION(BlueprintPure, Category = "Movie Scene Section")
		void SetStaringTargetBindingID(const FMovieSceneObjectBindingID& InStaringTargetBindingID)
	{
		StaringTargetBindingID = InStaringTargetBindingID;
	};

public:

	/** UMovieSceneSection interface */
	
	void OnBindingsUpdated(const TMap<FGuid, FGuid>& OldGuidToNewGuidMap) override;

	void GetReferencedBindings(TArray<FGuid>& OutBindings) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Section")
	FMovieSceneObjectBindingID StaringTargetBindingID;
};