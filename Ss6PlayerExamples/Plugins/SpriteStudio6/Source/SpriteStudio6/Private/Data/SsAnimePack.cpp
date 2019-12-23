#include "SsAnimePack.h"


void FSsPartAnime::Serialize(FArchive& Ar)
{
	for(int32 i = 0; i < Attributes.Num(); ++i)
	{
		Attributes[i].Serialize(Ar);
	}
}

void FSsAnimation::Serialize(FArchive& Ar)
{
	for(int32 i = 0; i < PartAnimes.Num(); ++i)
	{
		PartAnimes[i].Serialize(Ar);
	}
}

void FSsAnimePack::Serialize(FArchive& Ar)
{
	if(Ar.IsLoading() || Ar.IsSaving())
	{
		for(int32 i = 0; i < AnimeList.Num(); ++i)
		{
			AnimeList[i].Serialize(Ar);
		}
	}
	if(Ar.IsLoading())
	{
		for(auto It = AnimeList.CreateIterator(); It; ++It)
		{
			if(It->IsSetup)
			{
				Model.SetupAnimation = &(*It);
				break;
			}
		}
	}
}

int32 FSsAnimePack::FindAnimationIndex(const FName& Name) const
{
	for(int32 i = 0; i < AnimeList.Num(); ++i)
	{
		if(Name == AnimeList[i].AnimationName)
		{
			return i;
		}
	}
	return -1;
}
const FSsAnimation* FSsAnimePack::FindAnimation(const FName& Name) const
{
	int32 Index = FindAnimationIndex(Name);
	if(0 <= Index)
	{
		return &AnimeList[Index];
	}
	return nullptr;
}
