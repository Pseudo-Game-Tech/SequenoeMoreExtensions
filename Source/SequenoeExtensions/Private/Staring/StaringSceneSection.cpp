#include "Staring/StaringSceneSection.h"

UStaringSceneSection::UStaringSceneSection()
{
	EvalOptions.CompletionMode = EMovieSceneCompletionMode::RestoreState;
}

void UStaringSceneSection::OnBindingsUpdated(const TMap<FGuid, FGuid>& OldGuidToNewGuidMap)
{
	if (OldGuidToNewGuidMap.Contains(StaringTargetBindingID.GetGuid()))
	{
		StaringTargetBindingID.SetGuid(OldGuidToNewGuidMap[StaringTargetBindingID.GetGuid()]);
	}
}

void UStaringSceneSection::GetReferencedBindings(TArray<FGuid>& OutBindings)
{
	OutBindings.Add(StaringTargetBindingID.GetGuid());
}