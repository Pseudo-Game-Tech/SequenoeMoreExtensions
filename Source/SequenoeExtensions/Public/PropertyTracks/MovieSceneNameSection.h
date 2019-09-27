#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MovieSceneSection.h"
#include "MovieSceneNameChannel.h"
#include "MovieSceneNameSection.generated.h"

/**
 * A single string section
 */
UCLASS(MinimalAPI)
class UMovieSceneNameSection
	: public UMovieSceneSection
{
	GENERATED_UCLASS_BODY()

public:

	/**
	 * Public access to this section's internal data function
	 */
	const FMovieSceneNameChannel& GetChannel() const { return NameCurve; }

private:

	/** Curve data */
	UPROPERTY()
		FMovieSceneNameChannel NameCurve;
};
