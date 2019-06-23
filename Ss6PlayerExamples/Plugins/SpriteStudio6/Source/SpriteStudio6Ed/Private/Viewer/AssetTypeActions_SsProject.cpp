#include "SpriteStudio6EdPrivatePCH.h"
#include "AssetTypeActions_SsProject.h"
#include "Ss6Project.h"
#include "SsProjectViewer.h"

#define LOCTEXT_NAMESPACE "SpriteStudio6Ed"


UClass* FAssetTypeActions_SsProject::GetSupportedClass() const
{
	return USs6Project::StaticClass();
}

void FAssetTypeActions_SsProject::OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor )
{
	for(int i = 0; i < InObjects.Num(); ++i)
	{
		FSsProjectViewer::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects[i]);
	}
}

void FAssetTypeActions_SsProject::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for(auto& Asset : TypeAssets)
	{
		const USs6Project* SsProject = CastChecked<USs6Project>(Asset);
		if(SsProject->AssetImportData)
		{
			SsProject->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

#undef LOCTEXT_NAMESPACE
