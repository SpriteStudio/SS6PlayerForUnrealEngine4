#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayPropertySync.h"

#include "SsProject.h"


// コンストラクタ
FSsPlayPropertySync::FSsPlayPropertySync()
	: RefSsProject(NULL)
	, RefAutoPlayAnimPackName(NULL)
	, RefAutoPlayAnimationName(NULL)
	, RefAutoPlayAnimPackIndex(NULL)
	, RefAutoPlayAnimationIndex(NULL)
{
}
FSsPlayPropertySync::FSsPlayPropertySync(
	USsProject** InSsProject,
	FName* InAutoPlayAnimPackName,
	FName* InAutoPlayAnimationName,
	int32* InAutoPlayAnimPackIndex,
	int32* InAutoPlayAnimationIndex
	)
	: RefSsProject(InSsProject)
	, RefAutoPlayAnimPackName(InAutoPlayAnimPackName)
	, RefAutoPlayAnimationName(InAutoPlayAnimationName)
	, RefAutoPlayAnimPackIndex(InAutoPlayAnimPackIndex)
	, RefAutoPlayAnimationIndex(InAutoPlayAnimationIndex)
{
}

// シリアライズ時の呼び出し
void FSsPlayPropertySync::OnSerialize(FArchive& Ar)
{
	if(Ar.IsLoading())
	{
		SyncAutoPlayAnimation_NameToIndex();
	}
}

// プロパティ編集時の呼び出し
void FSsPlayPropertySync::OnPostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if(PropertyChangedEvent.Property)
	{
		FString PropertyName = PropertyChangedEvent.Property->GetNameCPP();
		if(    (0 == PropertyName.Compare(TEXT("SsProject")))
			|| (0 == PropertyName.Compare(TEXT("AutoPlayAnimPackName")))
			|| (0 == PropertyName.Compare(TEXT("AutoPlayAnimationName")))
			)
		{
			SyncAutoPlayAnimation_NameToIndex();
		}
		else if((0 == PropertyName.Compare(TEXT("AutoPlayAnimPackIndex")))
			||  (0 == PropertyName.Compare(TEXT("AutoPlayAnimationIndex")))
			)
		{
			SyncAutoPlayAnimation_IndexToName();
		}
	}
}


// 名前->インデックスへ、アニメーション指定を同期
void FSsPlayPropertySync::SyncAutoPlayAnimation_NameToIndex()
{
	check((NULL != RefSsProject) || (NULL != RefAutoPlayAnimPackName) || (NULL != RefAutoPlayAnimationName) || (NULL != RefAutoPlayAnimPackIndex) || (NULL != RefAutoPlayAnimationIndex));
	if(NULL == (*RefSsProject))
	{
		return;
	}
	if(0 == (*RefSsProject)->AnimeList.Num())	// エディタ環境で、SsProject側がロードされる前にココへ来てしまう場合がある 
	{
		return;
	}

	*RefAutoPlayAnimPackIndex  = -1;
	*RefAutoPlayAnimationIndex = -1;
	(*RefSsProject)->FindAnimationIndex(*RefAutoPlayAnimPackName, *RefAutoPlayAnimationName, *RefAutoPlayAnimPackIndex, *RefAutoPlayAnimationIndex);

	if(*RefAutoPlayAnimPackIndex < 0)
	{
		if(0 < (*RefSsProject)->AnimeList.Num())
		{
			*RefAutoPlayAnimPackIndex = 0;
			*RefAutoPlayAnimPackName  = (*RefSsProject)->AnimeList[*RefAutoPlayAnimPackIndex].AnimePackName;
		}
		else
		{
			*RefAutoPlayAnimPackIndex = -1;
			*RefAutoPlayAnimPackName  = FName();
		}
	}
	if(*RefAutoPlayAnimationIndex < 0)
	{
		if((0 <= *RefAutoPlayAnimPackIndex) && (0 < (*RefSsProject)->AnimeList[*RefAutoPlayAnimPackIndex].AnimeList.Num()))
		{
			*RefAutoPlayAnimationIndex = 0;
			*RefAutoPlayAnimationName  = (*RefSsProject)->AnimeList[*RefAutoPlayAnimPackIndex].AnimeList[*RefAutoPlayAnimationIndex].AnimationName;
		}
		else
		{
			*RefAutoPlayAnimationIndex = -1;
			*RefAutoPlayAnimationName  = FName();
		}
	}
}
// インデックス->名前へ、アニメーション指定を同期
void FSsPlayPropertySync::SyncAutoPlayAnimation_IndexToName()
{
	check((NULL != RefSsProject) || (NULL != RefAutoPlayAnimPackName) || (NULL != RefAutoPlayAnimationName) || (NULL != RefAutoPlayAnimPackIndex) || (NULL != RefAutoPlayAnimationIndex));
	if(NULL == (*RefSsProject))
	{
		return;
	}

	if((0 <= *RefAutoPlayAnimPackIndex) && (*RefAutoPlayAnimPackIndex < (*RefSsProject)->AnimeList.Num()))
	{
		*RefAutoPlayAnimPackName = (*RefSsProject)->AnimeList[*RefAutoPlayAnimPackIndex].AnimePackName;

		const FSsAnimation* Animation = (*RefSsProject)->FindAnimation(*RefAutoPlayAnimPackIndex, *RefAutoPlayAnimationIndex);
		if(Animation)
		{
			*RefAutoPlayAnimationName = Animation->AnimationName;
		}
		else
		{
			*RefAutoPlayAnimationIndex = 0;
			*RefAutoPlayAnimationName  = (*RefSsProject)->AnimeList[*RefAutoPlayAnimPackIndex].AnimeList[*RefAutoPlayAnimationIndex].AnimationName;
		}
	}
	else
	{
		*RefAutoPlayAnimPackIndex  = 0;
		*RefAutoPlayAnimationIndex = 0;
		*RefAutoPlayAnimPackName   = (*RefSsProject)->AnimeList[*RefAutoPlayAnimPackIndex].AnimePackName;
		*RefAutoPlayAnimationName  = (*RefSsProject)->AnimeList[*RefAutoPlayAnimPackIndex].AnimeList[*RefAutoPlayAnimationIndex].AnimationName;
	}
}
