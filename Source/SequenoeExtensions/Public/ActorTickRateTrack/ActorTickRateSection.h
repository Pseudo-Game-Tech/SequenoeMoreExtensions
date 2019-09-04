// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Sections/MovieSceneFloatSection.h"
#include "ActorTickRateSection.generated.h"


/**
 * A single floating point section.
 */
UCLASS(MinimalAPI)
class UActorTickRateSection
	: public UMovieSceneFloatSection
{
	GENERATED_BODY()

		/** Default constructor. */
		UActorTickRateSection();
};
