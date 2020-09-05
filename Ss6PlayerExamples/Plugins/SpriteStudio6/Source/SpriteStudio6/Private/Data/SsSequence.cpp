#include "SsSequence.h"

#include "Ss6Project.h"


bool FSsSequence::CalcSequenceFpsAndFrameCount(const USs6Project* OwnerProject, int32& OutFPS, int32& OutFrameCount)
{
	if(nullptr == OwnerProject)
	{
		return false;
	}

	// まずFPSを求める. シーケンス内に含まれるアニメーションで数値が最大のもの. 
	OutFPS = 0;
	for(auto ItItem = List.CreateConstIterator(); ItItem; ++ItItem)
	{
		const FSsAnimation* Animation = OwnerProject->FindAnimation(ItItem->RefAnimPack, ItItem->RefAnime);
		if(nullptr == Animation)
		{
			continue;
		}
		OutFPS = FMath::Max(OutFPS, Animation->Settings.Fps);
	}
	if(0 == OutFPS)
	{
		return false;
	}

	// 求めたFPS換算でのフレームを合計する 
	// FPS換算時の端数はFSsSequence単位で切捨て. ループ数分は先に合算するので注意. 
	OutFrameCount = 0;
	for(auto ItItem = List.CreateConstIterator(); ItItem; ++ItItem)
	{
		const FSsAnimation* Animation = OwnerProject->FindAnimation(ItItem->RefAnimPack, ItItem->RefAnime);
		if(nullptr == Animation)
		{
			continue;
		}

		int32 ItemFrameCount = ItItem->RepeatCount * Animation->GetFrameCount();
		int32 ItemSeqFrameCount = (int32)(ItemFrameCount * ((float)OutFPS / (float)Animation->Settings.Fps));

		OutFrameCount += ItemSeqFrameCount;
	}

	return true;
}
bool FSsSequence::GetAnimationBySequenceFrame(
	const USs6Project* OwnerProject,
	int32 SequenceFrame,
	int32& OutAnimePackIndex,
	int32& OutAnimationIndex,
	int32& OutItemIndex,
	int32& OutRepeatCount,
	int32& OutAnimationFrame
	)
{
	if((nullptr == OwnerProject) || (0 == List.Num()))
	{
		return false;
	}

	// 0の場合は色々検索が不要なので最適化 
	if(0 == SequenceFrame)
	{
		OutItemIndex      = 0;
		OutRepeatCount    = 0;
		OutAnimationFrame = 0;
		return OwnerProject->FindAnimationIndex(List[0].RefAnimPack, List[0].RefAnime, OutAnimePackIndex, OutAnimationIndex);
	}


	// シーケンスFPS換算でのフレーム数で検索していく 
	for(auto ItItem = List.CreateConstIterator(); ItItem; ++ItItem)
	{
		const FSsAnimation* Animation = OwnerProject->FindAnimation(ItItem->RefAnimPack, ItItem->RefAnime);
		if(nullptr == Animation)
		{
			continue;
		}

		int32 ItemFrameCount = ItItem->RepeatCount * Animation->GetFrameCount();
		int32 ItemSeqFrameCount = (int32)(ItemFrameCount * ((float)SequenceFPS / (float)Animation->Settings.Fps));
		if(SequenceFrame < ItemSeqFrameCount)
		{
			if(OwnerProject->FindAnimationIndex(ItItem->RefAnimPack, ItItem->RefAnime, OutAnimePackIndex, OutAnimationIndex))
			{
				OutItemIndex = ItItem.GetIndex();
				
				float AnimSeqFrameCount = Animation->GetFrameCount() * ((float)SequenceFPS / (float)Animation->Settings.Fps);	// シーケンサFPS換算でのアニメーションフレーム数 
				OutRepeatCount = (int32)(SequenceFrame / AnimSeqFrameCount);
				OutAnimationFrame = (int32)((SequenceFrame - (OutRepeatCount * AnimSeqFrameCount)) * ((float)Animation->Settings.Fps / (float)SequenceFPS));

				return true;
			}
		}

		SequenceFrame -= ItemSeqFrameCount;
	}

	// 範囲外であれば最後のアニメーションの最終フレームを指定しておく 
	if(OwnerProject->FindAnimationIndex(List[List.Num() - 1].RefAnimPack, List[List.Num() - 1].RefAnime, OutAnimePackIndex, OutAnimationIndex))
	{
		const FSsAnimation* Animation = OwnerProject->FindAnimation(OutAnimePackIndex, OutAnimationIndex);
		if(nullptr != Animation)
		{
			OutItemIndex = List.Num() - 1;
			OutRepeatCount = List.Last().RepeatCount;
			OutAnimationFrame = Animation->GetFrameCount();
			return true;
		}
	}

	return false;
}

const FSsSequence* FSsSequencePack::FindSequence(const FName& InName) const
{
	for(int32 i = 0; i < SequenceList.Num(); ++i)
	{
		if(InName == SequenceList[i].SequenceName)
		{
			return &SequenceList[i];
		}
	}
	return nullptr;
}
int32 FSsSequencePack::FindSequenceIndex(const FName& InName) const
{
	for(int32 i = 0; i < SequenceList.Num(); ++i)
	{
		if(InName == SequenceList[i].SequenceName)
		{
			return i;
		}
	}
	return -1;
}
