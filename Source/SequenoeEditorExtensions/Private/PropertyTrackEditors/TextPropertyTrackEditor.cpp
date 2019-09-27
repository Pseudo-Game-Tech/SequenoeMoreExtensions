#include "PropertyTrackEditors/TextPropertyTrackEditor.h"
#include "UObject/TextProperty.h"


TSharedRef<ISequencerTrackEditor> FTextPropertyTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> OwningSequencer)
{
	return MakeShareable(new FTextPropertyTrackEditor(OwningSequencer));
}


void FTextPropertyTrackEditor::GenerateKeysFromPropertyChanged(const FPropertyChangedParams& PropertyChangedParams, FGeneratedTrackKeys& OutGeneratedKeys)
{
	void* CurrentObject = PropertyChangedParams.ObjectsThatChanged[0];
	void* PropertyValue = nullptr;
	for (int32 i = 0; i < PropertyChangedParams.PropertyPath.GetNumProperties(); i++)
	{
		if (UProperty* Property = PropertyChangedParams.PropertyPath.GetPropertyInfo(i).Property.Get())
		{
			CurrentObject = Property->ContainerPtrToValuePtr<FText>(CurrentObject, 0);
		}
	}

	const UTextProperty* TextProperty = Cast<const UTextProperty>(PropertyChangedParams.PropertyPath.GetLeafMostProperty().Property.Get());
	if (TextProperty)
	{
		FText StrPropertyValue = TextProperty->GetPropertyValue(CurrentObject);
		OutGeneratedKeys.Add(FMovieSceneChannelValueSetter::Create<FMovieSceneTextChannel>(0, MoveTemp(StrPropertyValue), true));
	}
}
