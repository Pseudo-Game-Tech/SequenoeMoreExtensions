#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MovieSceneSection.h"
#include "MovieSceneTextChannel.h"
#include "MovieSceneTextSection.generated.h"

/**
 * A single string section
 */
UCLASS(MinimalAPI)
class UMovieSceneTextSection
	: public UMovieSceneSection
{
	GENERATED_UCLASS_BODY()

public:

	/**
	 * Public access to this section's internal data function
	 */
	const FMovieSceneTextChannel& GetChannel() const { return TextCurve; }

private:

	/** Curve data */
	UPROPERTY()
		FMovieSceneTextChannel TextCurve;
};
