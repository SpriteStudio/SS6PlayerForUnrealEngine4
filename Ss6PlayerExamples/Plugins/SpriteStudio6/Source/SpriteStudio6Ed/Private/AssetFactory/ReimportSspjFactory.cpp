#include "ReimportSspjFactory.h"
#include "Ss6Project.h"


UReimportSspjFactory::UReimportSspjFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bReimporting(false)
{
}


bool UReimportSspjFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	USs6Project* SsProject = Cast<USs6Project>(Obj);
	if(SsProject && SsProject->AssetImportData)
	{
		SsProject->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UReimportSspjFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	USs6Project* SsProject = Cast<USs6Project>(Obj);
	if(SsProject && ensure(NewReimportPaths.Num() == 1))
	{
		SsProject->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

EReimportResult::Type UReimportSspjFactory::Reimport(UObject* Obj)
{
	USs6Project* SsProject = Cast<USs6Project>(Obj);
	if(!SsProject)
	{
		return EReimportResult::Failed;
	}

	ExistImages.Empty();
	for(int i = 0; i < SsProject->CellmapList.Num(); ++i)
	{
		if(SsProject->CellmapList[i].Texture)
		{
			ExistImages.Add(SsProject->CellmapList[i].ImagePath, SsProject->CellmapList[i].Texture);
		}
	}

	const FString Filename = SsProject->AssetImportData->GetFirstFilename();
	if(!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
	{
		return EReimportResult::Failed;
	}

	bReimporting = true;

	EReimportResult::Type Result = EReimportResult::Failed;
	if(UFactory::StaticImportObject(SsProject->GetClass(), SsProject->GetOuter(), *SsProject->GetName(), RF_Public | RF_Standalone, *Filename, NULL, this))
	{
		// Try to find the outer package so we can dirty it up
		if (SsProject->GetOuter())
		{
			SsProject->GetOuter()->MarkPackageDirty();
		}
		else
		{
			SsProject->MarkPackageDirty();
		}
		Result = EReimportResult::Succeeded;
	}
	else
	{
		Result = EReimportResult::Failed;
	}

	bReimporting = false;
	return Result;
}
