#pragma once


class USs6Project;


//
// Component, Widget などの、連動するプロパティの同期処理を行う 
//
class FSsPlayPropertySync
{
protected:
	FSsPlayPropertySync();
	FSsPlayPropertySync(
		USs6Project** InSsProject,
		FName* InAutoPlayAnimPackName,
		FName* InAutoPlayAnimationName,
		int32* InAutoPlayAnimPackIndex,
		int32* InAutoPlayAnimationIndex
		);

	void OnSerialize(FArchive& Ar);
	void OnPostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent);

	void SyncAutoPlayAnimation_NameToIndex();
	void SyncAutoPlayAnimation_IndexToName();

private:
	USs6Project** RefSsProject;
	FName* RefAutoPlayAnimPackName;
	FName* RefAutoPlayAnimationName;
	int32* RefAutoPlayAnimPackIndex;
	int32* RefAutoPlayAnimationIndex;
};
