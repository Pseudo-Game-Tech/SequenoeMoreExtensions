#include "PropertyTrackEditors/NamePropertyTrackEditor.h"


TSharedRef<ISequencerTrackEditor> FNamePropertyTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
{
	return MakeShareable(new FNamePropertyTrackEditor(OwningSequencer));
}


void FNamePropertyTrackEditor::GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, FGeneratedTrackKeys& OutGeneratedKeys)
{
	void* CurrentObject = PropertyChangedParams.ObjectsThatChanged[0];
	void* PropertyValue = nullptr;
	for (int32 i = 0; i < PropertyChangedParams.PropertyPath.GetNumProperties(); i++)
	{
		if (UProperty* Property = PropertyChangedParams.PropertyPath.GetPropertyInfo(i).Property.Get())
		{
			CurrentObject = Property->ContainerPtrToValuePtr<FName>(CurrentObject, 0);
		}
	}

	const UNameProperty* NameProperty = Cast<const UNameProperty>(PropertyChangedParams.PropertyPath.GetLeafMostProperty().Property.Get());
	if (NameProperty)
	{
		FName StrPropertyValue = NameProperty->GetPropertyValue(CurrentObject);
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneNameChannel>(0, MoveTemp(StrPropertyValue), true));
	}
}
