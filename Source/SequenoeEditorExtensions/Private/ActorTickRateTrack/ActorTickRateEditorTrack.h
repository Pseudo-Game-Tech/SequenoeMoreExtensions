// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "ISequencer.h"
#include "MovieSceneTrack.h"
#include "ISequencerTrackEditor.h"
#include "FloatPropertyTrackEditor.h"
#include "ActorTickRateTrack.h"

class FActorTickRateEditorTrack
	: public FKeyframeTrackEditor<UActorTickRateTrack>
{
public:

	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> InSequencer);

public:

	FActorTickRateEditorTrack(TSharedRef<ISequencer> InSequencer);

public:

	// ISequencerTrackEditor interface

	virtual void BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;

private:
	void AddActorTickRateTrack(FGuid ObjectBinding);
};
