#include "SpriteStudio6EdPrivatePCH.h"
#include "SspjFactory.h"

#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

#include "SsImportSettings.h"
#include "ReimportSspjFactory.h"
#include "Ss6Project.h"
#include "SsCellMap.h"
#include "SsAnimePack.h"
#include "SsLoader.h"


namespace
{
	FString GetFilePath(const FString& CurPath, const FString& BaseDir, const FString& FileName)
	{
		bool bBaseDirRelative = FPaths::IsRelative(BaseDir);
		bool bFileNameRelative = FPaths::IsRelative(FileName);
		if(bBaseDirRelative && bFileNameRelative)
		{
			return CurPath / BaseDir / FileName;
		}
		if(bFileNameRelative)
		{
			return BaseDir / FileName;
		}
		return FileName;
	}
}


USspjFactory::USspjFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Formats.Add(TEXT("sspj;SpriteStudio Project"));

	SupportedClass = USs6Project::StaticClass();
	bCreateNew = false;
	bEditAfterNew = false;
	bEditorImport = true;
	bText = false;
}

bool USspjFactory::DoesSupportClass(UClass * Class)
{
	return (Class == USs6Project::StaticClass());
}

UClass* USspjFactory::ResolveSupportedClass()
{
	return USs6Project::StaticClass();
}

UObject* USspjFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* InBufferEnd, FFeedbackContext* Warn)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	// インポート設定の取得 
	const USsImportSettings* ImportSettings = GetDefault<USsImportSettings>();

	// 再インポートかどうか 
	bool bReimport = false;
	TMap<FString, UTexture*>* ExistImages = NULL;
	if(this->IsA(UReimportSspjFactory::StaticClass()))
	{
		UReimportSspjFactory* ReimportFactory = Cast<UReimportSspjFactory>(this);
		bReimport = ReimportFactory->bReimporting;
		if(bReimport)
		{
			ExistImages = &(ReimportFactory->ExistImages);
		}
	}

	FString ProjectNameStr = InName.ToString();
	FName ProjectName = InName;

	UPackage* InParentPackage = Cast<UPackage>(InParent);
	if(InParentPackage && !bReimport)
	{
		FString ProjectPackageName;
		FString BasePackageName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName()) / ProjectNameStr;
		if(ImportSettings->bCreateSspjFolder)
		{
			BasePackageName = BasePackageName / ProjectNameStr;
		}
		AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, TEXT(""), ProjectPackageName, ProjectNameStr);
		BasePackageName = ProjectPackageName;
		int32 i = 0;
		while(!InParentPackage->Rename(*ProjectPackageName, nullptr, REN_Test))
		{
			ProjectPackageName = BasePackageName + FString::FromInt(i++);
		}
		InParentPackage->Rename(*ProjectPackageName);
	}

	// インポート開始 
	FEditorDelegates::OnAssetPreImport.Broadcast(this, InClass, InParent, ProjectName, Type);

	// sspj
	USs6Project* NewProject = FSsLoader::LoadSsProject(InParent, ProjectName, Flags, Buffer, (InBufferEnd - Buffer) + 1);
	if(NewProject)
	{
		NewProject->SetFilepath(GetCurrentFilename());

		if(NewProject->AssetImportData == nullptr)
		{
			NewProject->AssetImportData = NewObject<UAssetImportData>(NewProject);
		}
		NewProject->AssetImportData->Update(CurrentFilename);

		FString CurPath = FPaths::GetPath(GetCurrentFilename());

		TArray<FString> ImagePaths;
		TArray<SsTexWrapMode::Type> ImageWrapModes;
		TArray<SsTexFilterMode::Type> ImageFilterModes;

		// ssce
		NewProject->CellmapList.Empty();
		NewProject->CellmapList.AddZeroed(NewProject->CellmapNames.Num());
		for(int i = 0; i < NewProject->CellmapNames.Num(); ++i)
		{
			FString FileName = GetFilePath(CurPath, NewProject->Settings.CellMapBaseDirectory, NewProject->CellmapNames[i].ToString());

			TArray<uint8> Data;
			if(FFileHelper::LoadFileToArray(Data, *FileName))
			{
				const uint8* BufferBegin = Data.GetData();
				const uint8* BufferEnd = BufferBegin + Data.Num() - 1;
				if(FSsLoader::LoadSsCellMap(&(NewProject->CellmapList[i]), BufferBegin, (BufferEnd - BufferBegin) + 1))
				{
					NewProject->CellmapList[i].FileName = NewProject->CellmapNames[i];
					if(0 < NewProject->CellmapList[i].ImagePath.Len())
					{
						if(INDEX_NONE == ImagePaths.Find(NewProject->CellmapList[i].ImagePath))
						{
							ImagePaths.Add(NewProject->CellmapList[i].ImagePath);
							if(NewProject->CellmapList[i].OverrideTexSettings)
							{
								ImageWrapModes.Add(NewProject->CellmapList[i].WrapMode);
								ImageFilterModes.Add(NewProject->CellmapList[i].FilterMode);
							}
							else
							{
								ImageWrapModes.Add(NewProject->Settings.WrapMode);
								ImageFilterModes.Add(NewProject->Settings.FilterMode);
							}
						}
					}
				}
			}
		}

		// ssae
		NewProject->AnimeList.Empty();
		NewProject->AnimeList.AddZeroed(NewProject->AnimepackNames.Num());
		for(int i = 0; i < NewProject->AnimepackNames.Num(); ++i)
		{
			FString FileName = GetFilePath(CurPath, NewProject->Settings.AnimeBaseDirectory, NewProject->AnimepackNames[i].ToString());

			TArray<uint8> Data;
			if(FFileHelper::LoadFileToArray(Data, *FileName))
			{
				const uint8* BufferBegin = Data.GetData();
				const uint8* BufferEnd = BufferBegin + Data.Num() - 1;
				FSsLoader::LoadSsAnimePack(&(NewProject->AnimeList[i]), BufferBegin, (BufferEnd - BufferBegin) + 1);
			}
		}

		// ssee
		NewProject->EffectList.Empty();
		NewProject->EffectList.AddZeroed(NewProject->EffectFileNames.Num());
		for(int i = 0; i < NewProject->EffectFileNames.Num(); ++i)
		{
			FString FileName = GetFilePath(CurPath, NewProject->Settings.EffectBaseDirectory, NewProject->EffectFileNames[i].ToString());

			TArray<uint8> Data;
			if(FFileHelper::LoadFileToArray(Data, *FileName))
			{
				const uint8* BufferBegin = Data.GetData();
				const uint8* BufferEnd = BufferBegin + Data.Num() - 1;
				FSsLoader::LoadSsEffectFile(&(NewProject->EffectList[i]), BufferBegin, (BufferEnd - BufferBegin) + 1);
			}
		}

		// texture
		{
			UTextureFactory* TextureFact = NewObject<UTextureFactory>();
			TextureFact->AddToRoot();

			for(int i = 0; i < ImagePaths.Num(); ++i)
			{
				FString FileName = GetFilePath(CurPath, NewProject->Settings.ImageBaseDirectory, ImagePaths[i]);

				UTexture* ImportedTexture = NULL;
				if(ExistImages && ExistImages->Contains(ImagePaths[i]))
				{
					ImportedTexture = ExistImages->FindChecked(ImagePaths[i]);
				}

				TArray<uint8> Data;
				if(FFileHelper::LoadFileToArray(Data, *FileName))
				{
					FString TextureName = FPaths::GetBaseFilename(ImagePaths[i]);

					UPackage* TexturePackage = NULL;
					if(ImportedTexture)
					{
						TexturePackage = ImportedTexture->GetOutermost();
					}
					else
					{
						FString TexturePackageName;
						FString BasePackageName = FPackageName::GetLongPackagePath(InParent->GetOutermost()->GetName()) / TextureName;
						AssetToolsModule.Get().CreateUniqueAssetName(BasePackageName, TEXT(""), TexturePackageName, TextureName);
						TexturePackage = CreatePackage(NULL, *TexturePackageName);
					}

					const uint8* BufferBegin = Data.GetData();
					const uint8* BufferEnd = BufferBegin + Data.Num();
					UTexture2D* NewTexture = (UTexture2D*)TextureFact->FactoryCreateBinary(
						UTexture2D::StaticClass(),
						TexturePackage,
						FName(*TextureName),
						Flags,
						NULL,
						*FPaths::GetExtension(ImagePaths[i]),
						BufferBegin, BufferEnd,
						Warn
						);
					if(NewTexture)
					{
						if(ImportSettings->bOverwriteMipGenSettings)
						{
							NewTexture->MipGenSettings = TMGS_NoMipmaps;
						}
						if(ImportSettings->bOverwriteTextureGroup)
						{
							NewTexture->LODGroup = ImportSettings->TextureGroup;
						}
						if(ImportSettings->bOverwriteCompressionSettings)
						{
							NewTexture->CompressionSettings = TextureCompressionSettings::TC_EditorIcon;
						}
						if(ImportSettings->bOverwriteTilingMethodFromSspj)
						{
							switch(ImageWrapModes[i])
							{
								case SsTexWrapMode::Clamp:
									{
										NewTexture->AddressX = NewTexture->AddressY = TA_Clamp;
									} break;
								case SsTexWrapMode::Repeat:
									{
										NewTexture->AddressX = NewTexture->AddressY = TA_Wrap;
									} break;
								case SsTexWrapMode::Mirror:
									{
										NewTexture->AddressX = NewTexture->AddressY = TA_Mirror;
									} break;
							}
						}
						if(ImportSettings->bOverwriteNeverStream)
						{
							NewTexture->NeverStream = true;
						}
						if(ImportSettings->bOverwriteFilterFromSspj)
						{
							switch(ImageFilterModes[i])
							{
								case SsTexFilterMode::Nearest:
									{
										NewTexture->Filter = TF_Nearest;
									} break;
								case SsTexFilterMode::Linear:
									{
										NewTexture->Filter = TF_Bilinear;
									} break;
							}
						}

						NewTexture->UpdateResource();

						FAssetRegistryModule::AssetCreated(NewTexture);
						TexturePackage->SetDirtyFlag(true);

						ImportedTexture = NewTexture;
					}
				}

				if(ImportedTexture)
				{
					for(int ii = 0; ii < NewProject->CellmapList.Num(); ++ii)
					{
						if(NewProject->CellmapList[ii].ImagePath == ImagePaths[i])
						{
							NewProject->CellmapList[ii].Texture = ImportedTexture;
						}
					}
				}
			}

			TextureFact->RemoveFromRoot();
		}
	}

	// インポート終了
	FEditorDelegates::OnAssetPostImport.Broadcast(this, NewProject);
	return NewProject;
}