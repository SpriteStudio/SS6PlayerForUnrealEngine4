#include "SspjFactory.h"

#include "AssetRegistry/AssetRegistryModule.h"
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
	bool bError = false;

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
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPreImport(this, InClass, InParent, ProjectName, Type);

	// sspj
	TArray<FString> Warnings;
	USs6Project* NewProject = FSsLoader::LoadSsProject(InParent, ProjectName, Flags, Buffer, (InBufferEnd - Buffer) + 1, Warnings);
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
				if(FSsLoader::LoadSsCellMap(&(NewProject->CellmapList[i]), BufferBegin, (BufferEnd - BufferBegin) + 1, Warnings))
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
			else
			{
				// ssceファイルのロード失敗. ファイルが無いか、日本語ファイル名が置換されているとココへ来る. 
				UE_LOG(LogSpriteStudioEd, Warning, TEXT("Failed Load ssce file. Check if the file exist, or the previous warning."));
			}
		}

		// ssae
		TArray<FSsAnimePack> TempAnimeList;
		TempAnimeList.AddZeroed(NewProject->AnimepackNames.Num());
		TArray<int32> TempSortOrder;
		TempSortOrder.AddZeroed(NewProject->AnimepackNames.Num());
		for(int i = 0; i < NewProject->AnimepackNames.Num(); ++i)
		{
			FString FileName = GetFilePath(CurPath, NewProject->Settings.AnimeBaseDirectory, NewProject->AnimepackNames[i].ToString());

			TArray<uint8> Data;
			if(FFileHelper::LoadFileToArray(Data, *FileName))
			{
				const uint8* BufferBegin = Data.GetData();
				const uint8* BufferEnd = BufferBegin + Data.Num() - 1;
				FSsLoader::LoadSsAnimePack(&(TempAnimeList[i]), BufferBegin, (BufferEnd - BufferBegin) + 1, TempSortOrder[i], Warnings);
			}
		}

		// SpriteStudio上での並び替えを反映 
		TArray<FName> TempAnimepackNames = NewProject->AnimepackNames;
		TArray<int32> SortOrder;
		NewProject->AnimepackNames.Empty();
		NewProject->AnimeList.Empty();
		for(int32 i = 0; i < TempAnimeList.Num(); ++i)
		{
			bool bInsert = false;
			for(int32 j = 0; j < NewProject->AnimeList.Num(); ++j)
			{
				if((0 != TempSortOrder[i]) && (TempSortOrder[i] < SortOrder[j]))
				{
					NewProject->AnimepackNames.Insert(TempAnimepackNames[i], j);
					NewProject->AnimeList.Insert(TempAnimeList[i], j);
					SortOrder.Insert(TempSortOrder[i], j);
					bInsert = true;
					break;
				}
			}
			if(!bInsert)
			{
				NewProject->AnimepackNames.Add(TempAnimepackNames[i]);
				NewProject->AnimeList.Add(TempAnimeList[i]);
				SortOrder.Add(TempSortOrder[i]);
			}
		}
		for(auto ItAnimePack = NewProject->AnimeList.CreateIterator(); ItAnimePack; ++ItAnimePack)
		{
			ItAnimePack->Model.SetupAnimation = nullptr;
			for(auto ItAnime = ItAnimePack->AnimeList.CreateIterator(); ItAnime; ++ItAnime)
			{
				if(ItAnime->IsSetup)
				{
					ItAnimePack->Model.SetupAnimation = &(*ItAnime);
					break;
				}
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
				FSsLoader::LoadSsEffectFile(&(NewProject->EffectList[i]), BufferBegin, (BufferEnd - BufferBegin) + 1, Warnings);
			}
		}

		// ssqe
		NewProject->SequenceList.Empty();
		NewProject->SequenceList.AddZeroed(NewProject->SequencePackNames.Num());
		for(int i = 0; i < NewProject->SequencePackNames.Num(); ++i)
		{
			FString FileName = GetFilePath(CurPath, NewProject->Settings.AnimeBaseDirectory, NewProject->SequencePackNames[i].ToString());

			TArray<uint8> Data;
			if(FFileHelper::LoadFileToArray(Data, *FileName))
			{
				const uint8* BufferBegin = Data.GetData();
				const uint8* BufferEnd = BufferBegin + Data.Num() - 1;
				FSsLoader::LoadSsSequence(&(NewProject->SequenceList[i]), BufferBegin, (BufferEnd - BufferBegin) + 1, Warnings);

				// シーケンス毎にFPSと総フレーム数を計算しておく 
				for(auto ItSequencePack = NewProject->SequenceList.CreateIterator(); ItSequencePack; ++ItSequencePack)
				{
					for(auto ItSequence = ItSequencePack->SequenceList.CreateIterator(); ItSequence; ++ItSequence)
					{
						int32 FPS, FrameCount;
						ItSequence->CalcSequenceFpsAndFrameCount(NewProject, FPS, FrameCount);
						ItSequence->SequenceFPS = FPS;
						ItSequence->SequenceFrameCount = FrameCount;
					}
				}
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
					FString TextureName = (nullptr == ImportedTexture) ? FPaths::GetBaseFilename(ImagePaths[i]) : ImportedTexture->GetName();

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
						TexturePackage = CreatePackage(*TexturePackageName);
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
				else
				{
					if(FileName.Contains(FString("Replaced")))
					{
						UE_LOG(LogSpriteStudioEd, Error, TEXT("Failed Load Texture (テクスチャファイルパスに日本語が含まれるためロード出来ませんでした)"));
					}
					else
					{
						UE_LOG(LogSpriteStudioEd, Error, TEXT("Failed Load Texture : %s"), *FileName);
					}
					bError = true;
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

		NewProject->PostLoadInternal();
	}

	if(0 < Warnings.Num())
	{
		FString Msg(TEXT("SSPJ内で日本語文字が使用されており、正常に動作しない可能性があります。\n\n"));
		for(auto It = Warnings.CreateConstIterator(); It; ++It)
		{
			Msg += (*It) + TEXT("\n");
		}
		FMessageDialog::Open(
			EAppMsgType::Ok,
			FText::FromString(Msg),
			FText::FromString(TEXT("WARNING"))
			);
	}

	// インポート終了
	GEditor->GetEditorSubsystem<UImportSubsystem>()->BroadcastAssetPostImport(this, NewProject);
	return bError ? nullptr : NewProject;
}