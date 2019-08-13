#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "MovieSceneTrack.h"
#include "MovieSceneSection.h"
#include "MovieSceneObjectBindingID.h"
#include "StaringTrack.generated.h"

UCLASS(MinimalAPI)
class UStaringTrack : public UMovieSceneTrack
{
public:
	GENERATED_BODY()

	virtual UMovieSceneSection* AddStaring(FFrameNumber Time, int32 Duration, const FMovieSceneObjectBindingID& StaringBindingID);

public:

	/* UMovieSceneTrack interface */

	virtual void RemoveAllAnimationData() override;
	virtual bool HasSection(const UMovieSceneSection& Section) const override;
	virtual void AddSection(UMovieSceneSection& Section) override;
	virtual void RemoveSection(UMovieSceneSection& Section) override;
	virtual bool IsEmpty() const override;
	virtual const TArray<UMovieSceneSection*>& GetAllSections() const override;

	virtual bool SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const override;
	virtual class UMovieSceneSection* CreateNewSection() override;

	virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection& InSection) const override;

#if WITH_EDITORONLY_DATA
	virtual FText GetDisplayName() const override;
#endif

protected:

	/** 轨迹中所有得截面 */
	UPROPERTY()
		TArray<UMovieSceneSection*> StaringSections;
};