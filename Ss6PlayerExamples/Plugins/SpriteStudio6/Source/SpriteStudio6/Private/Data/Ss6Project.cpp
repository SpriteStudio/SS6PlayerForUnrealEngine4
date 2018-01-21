#include "SpriteStudio6PrivatePCH.h"
#include "Ss6Project.h"

#include "SsAnimePack.h"
#include "SsCellMap.h"
#include "SsString_uty.h"


USs6Project::USs6Project(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void USs6Project::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	Ar.UsingCustomVersion(SSPJ_GUID);

	if(Ar.IsLoading() || Ar.IsSaving())
	{
		for(int32 i = 0; i < AnimeList.Num(); ++i)
		{
			AnimeList[i].Serialize(Ar);
		}
		for(int32 i = 0; i < EffectList.Num(); ++i)
		{
			EffectList[i].Serialize(Ar);
		}
	}
}

int32 USs6Project::FindAnimePackIndex(const FName& AnimePackName) const
{
	for(int32 i = 0; i < AnimeList.Num(); ++i)
	{
		if(AnimePackName == AnimeList[i].AnimePackName)
		{
			return i;
		}
	}
	return -1;
}

int32 USs6Project::FindCellMapIndex(const FName& CellmapName) const
{
	for(int32 i = 0; i < CellmapList.Num(); ++i)
	{
		if(CellmapName == CellmapList[i].FileName)
		{
			return i;
		}
	}
	return -1;
}

bool USs6Project::FindAnimationIndex(const FName& InAnimPackName, const FName& InAnimationName, int32& OutAnimPackIndex, int32& OutAnimationIndex) const
{
	OutAnimPackIndex = FindAnimePackIndex(InAnimPackName);
	if(OutAnimPackIndex < 0){ return false; }

	OutAnimationIndex = AnimeList[OutAnimPackIndex].FindAnimationIndex(InAnimationName);
	if(OutAnimationIndex < 0){ return false; }

	return true;
}

const FSsAnimation* USs6Project::FindAnimation(int32 AnimPackIndex, int32 AnimationIndex) const
{
	if((0 <= AnimPackIndex) && (AnimPackIndex < AnimeList.Num()))
	{
		if((0 <= AnimationIndex) && (AnimationIndex < AnimeList[AnimPackIndex].AnimeList.Num()))
		{
			return &(AnimeList[AnimPackIndex].AnimeList[AnimationIndex]);
		}
	}
	return NULL;
}

const FSsAnimation* USs6Project::FindAnimation(const FName& InAnimPackName, const FName& InAnimationName) const
{
	const FSsAnimePack* AnimePack = FindAnimationPack(InAnimPackName);
	if(nullptr != AnimePack)
	{
		for(auto It = AnimePack->AnimeList.CreateConstIterator(); It; ++It)
		{
			if(InAnimationName == It->AnimationName)
			{
				return &(*It);
			}
		}
	}
	return nullptr;
}

const FSsCellMap* USs6Project::FindCellMap(const FName& InCellMapName) const
{
	for(auto It = CellmapList.CreateConstIterator(); It; ++It)
	{
		if(InCellMapName == It->FileName)
		{
			return &(*It);
		}
	}
	return nullptr;
}

const FSsAnimePack* USs6Project::FindAnimationPack(const FName& InAnimePackName) const
{
	for(auto It = AnimeList.CreateConstIterator(); It; ++It)
	{
		if(InAnimePackName == It->AnimePackName)
		{
			return &(*It);
		}
	}
	return nullptr;
}

int32 USs6Project::FindEffectIndex(const FName& EffectName) const
{
	for(int32 i = 0; i < EffectList.Num(); ++i)
	{
		if(EffectName == EffectList[i].Name)
		{
			return i;
		}
	}
	return -1;
}

const FSsEffectFile* USs6Project::FindEffect(const FName& EffectName) const
{
	for(auto It = EffectList.CreateConstIterator(); It; ++It)
	{
		if(EffectName == It->Name)
		{
			return &(*It);
		}
	}
	return nullptr;
}


FString USs6Project::GetSsceBasepath() const
{
	return getFullPath(ProjFilepath, Settings.CellMapBaseDirectory);
}
FString USs6Project::GetSsaeBasepath() const
{
	return getFullPath(ProjFilepath, Settings.AnimeBaseDirectory);
}
FString USs6Project::GetImageBasepath() const
{
	return getFullPath(ProjFilepath, Settings.ImageBaseDirectory);
}


namespace
{
	uint32 CalcMaxRenderPartsNum_Effect_Recursice(const FSsEffectFile& Effect, const FSsEffectNode* Node, uint32 DrawCount)
	{
		if (nullptr == Node)
		{
			return 0;
		}

		uint32 Result = 0;
		switch (Node->Type)
		{
			case SsEffectNodeType::Root:
				{} break;
			case SsEffectNodeType::Emmiter:
				{
					for (int32 i = 0; i < Node->Behavior.PList.Num(); ++i)
					{
						if (SsEffectFunctionType::Basic == Node->Behavior.PList[i]->MyType)
						{
							DrawCount *= (uint32)(static_cast<FSsParticleElementBasic*>(Node->Behavior.PList[i].Get())->MaximumParticle);
							break;
						}
					}
				} break;
			case SsEffectNodeType::Particle:
				{
					Result += DrawCount;
				} break;
		}

		for(int32 i = 0; i < Effect.EffectData.NodeList.Num(); ++i)
		{
			if(Node->ArrayIndex == Effect.EffectData.NodeList[i].ParentIndex)
			{
				Result += CalcMaxRenderPartsNum_Effect_Recursice(Effect, &(Effect.EffectData.NodeList[i]), DrawCount);
			}
		}

		return Result;
	}
	uint32 CalcMaxRenderPartsNum_Recursive(const USs6Project& Proj, const FSsAnimePack& AnimePack)
	{
		uint32 Result = AnimePack.Model.PartList.Num();
		for(int32 i = 0; i < AnimePack.Model.PartList.Num(); ++i)
		{
			int32 RefAnimePackIndex = Proj.FindAnimePackIndex(AnimePack.Model.PartList[i].RefAnimePack);
			if(0 <= RefAnimePackIndex)
			{
				Result += CalcMaxRenderPartsNum_Recursive(Proj, Proj.AnimeList[RefAnimePackIndex]);
			}
			int32 RefEffectIndex = Proj.FindEffectIndex(AnimePack.Model.PartList[i].RefEffectName);
			if(0 <= RefEffectIndex)
			{
				Result += CalcMaxRenderPartsNum_Effect_Recursice(Proj.EffectList[RefEffectIndex], Proj.EffectList[RefEffectIndex].EffectData.Root, 1);
			}
		}
		return Result;
	}
}

void USs6Project::CalcMaxVertexAndIndexNum(uint32& OutVertexNum, uint32& OutIndexNum) const
{
	uint32 PartsNum = 0;
	for(int32 i = 0; i < AnimeList.Num(); ++i)
	{
		for(int32 j = 0; j < AnimeList[i].AnimeList.Num(); ++j)
		{
			PartsNum = FMath::Max(PartsNum, CalcMaxRenderPartsNum_Recursive(*this, AnimeList[i]));
		}
	}
	OutVertexNum = PartsNum * 4;
	OutIndexNum = PartsNum * 6;
}
