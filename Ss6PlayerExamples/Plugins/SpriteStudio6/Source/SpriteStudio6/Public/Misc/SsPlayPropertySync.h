#pragma once


class USsProject;


//
// Component, Widget などの、連動するプロパティの同期処理を行う 
//
class FSsPlayPropertySync
{
protected:
	FSsPlayPropertySync();
	FSsPlayPropertySync(
		USsProject** InSsProject,
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
	USsProject** RefSsProject;
	FName* RefAutoPlayAnimPackName;
	FName* RefAutoPlayAnimationName;
	int32* RefAutoPlayAnimPackIndex;
	int32* RefAutoPlayAnimationIndex;
};
