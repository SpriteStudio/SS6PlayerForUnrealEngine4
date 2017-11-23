#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayer.h"

#include "Ss6Project.h"
#include "SsAnimePack.h"
#include "SsString_uty.h"

#include "ssplayer_animedecode.h"
#include "SsPlayer_PartState.h"

// コンストラクタ
FSsPlayer::FSsPlayer()
	: PlayRate(1.f)
	, LoopCount(-1)
	, bRoundTrip(false)
	, bFlipH(false)
	, bFlipV(false)
	, SsProject(nullptr)
	, Decoder(nullptr)
	, bPlaying(false)
	, bFirstTick(false)
	, AnimPivot(0.f,0.f)
	, PlayingAnimPackIndex(-1)
	, PlayingAnimationIndex(-1)
	, bCalcHideParts(false)
{}

// デストラクタ
FSsPlayer::~FSsPlayer()
{
	if(nullptr != Decoder)
	{
		delete Decoder;
	}
}

// SsProjectアセットをセット
void FSsPlayer::SetSsProject(TWeakObjectPtr<USs6Project> InSsProject)
{
	SsProject = InSsProject;
	if(!SsProject.IsValid())
	{
		return;
	}

	if(nullptr != Decoder)
	{
		delete Decoder;
	}
	Decoder = new SsAnimeDecoder();
	CellMapList = MakeShareable(new FSsCellMapList());

	PlayingAnimPackIndex = -1;
	PlayingAnimationIndex = -1;

	//TODO: 旧DecoderのCreateRenderPart()を移植する際に考慮するコト 
//	Decoder->SetCalcHideParts(bCalcHideParts);
}

// 更新
FSsPlayerTickResult FSsPlayer::Tick(float DeltaSeconds)
{
	FSsPlayerTickResult Result;

	if(bPlaying)
	{
		TickAnimation(DeltaSeconds, Result);
	}

	return Result;
}

// アニメーションの更新
void FSsPlayer::TickAnimation(float DeltaSeconds, FSsPlayerTickResult& Result)
{
	if(nullptr == Decoder)
	{
		return;
	}

	// 更新後のフレーム
	float BkAnimeFrame = Decoder->nowPlatTime;
	float AnimeFrame = BkAnimeFrame + (PlayRate * DeltaSeconds * Decoder->getAnimeFPS());

	// 再生開始フレームと同フレームに設定されたユーザーデータを拾うため、初回更新時のみBkAnimeFrameを誤魔化す
	if(bFirstTick)
	{
		BkAnimeFrame -= (.0f <= PlayRate) ? .1f : -.1f;
	}

	// 0フレームを跨いだか 
	bool bBeyondZeroFrame = false;

	// 最終フレーム以降で順方向再生
	if((Decoder->getAnimeEndFrame() <= AnimeFrame) && (0.f < PlayRate))
	{
		if(0 < LoopCount)
		{
			// ループ回数更新
			--LoopCount;

			// ループ回数の終了
			if(0 == LoopCount)
			{
				AnimeFrame = (float)Decoder->getAnimeEndFrame();
				Pause();
				Result.bEndPlay = true;
			}
		}
		FindUserDataInInterval(Result, BkAnimeFrame, (float)Decoder->getAnimeEndFrame());

		if(bPlaying)
		{
			float AnimeFrameSurplus = AnimeFrame - Decoder->getAnimeEndFrame();		// 極端に大きなDeltaSecondsは考慮しない
			// 往復
			if(bRoundTrip)
			{
				AnimeFrame = Decoder->getAnimeEndFrame() - AnimeFrameSurplus;
				PlayRate *= -1.f;
				FindUserDataInInterval(Result, (float)Decoder->getAnimeEndFrame(), AnimeFrame);
			}
			// ループ
			else
			{
				AnimeFrame = AnimeFrameSurplus;
				FindUserDataInInterval(Result, -.1f, AnimeFrame);
			}
		}
		bBeyondZeroFrame = true;
	}
	// 0フレーム以前で逆方向再生
	else if((AnimeFrame < 0.f) && (PlayRate < 0.f))
	{
		if(0 < LoopCount)
		{
			// ループ回数更新
			--LoopCount;

			// ループ回数の終了
			if(0 == LoopCount)
			{
				AnimeFrame = 0.f;
				Pause();
				Result.bEndPlay = true;
			}
		}
		FindUserDataInInterval(Result, BkAnimeFrame, 0.f);

		if(bPlaying)
		{
			// 往復
			if(bRoundTrip)
			{
				AnimeFrame = -AnimeFrame;
				PlayRate *= -1.f;
				FindUserDataInInterval(Result, 0.f, AnimeFrame);
			}
			// ループ
			else
			{
				AnimeFrame = Decoder->getAnimeEndFrame() + AnimeFrame;
				FindUserDataInInterval(Result, (float)Decoder->getAnimeEndFrame()+.1f, AnimeFrame);
			}
		}
		bBeyondZeroFrame = true;
	}
	else
	{
		FindUserDataInInterval(Result, BkAnimeFrame, AnimeFrame);
	}

	Decoder->setPlayFrame( AnimeFrame );

/*
	Decoder->SetDeltaForIndependentInstance( DeltaSeconds );
	if(bBeyondZeroFrame)
	{
		Decoder->ReloadEffects();	// コレは中身が #if 0 で消されてるので削除でOK 
	}
	Decoder->Update();

	RenderParts.Empty();
	Decoder->CreateRenderParts(RenderParts);
*/
	//TODO:	更新手順を再確認 
	//		CreateRenderParts相当の処理はPlayer側で実装する 
	Decoder->update(DeltaSeconds * Decoder->getAnimeFPS());

	// 水平反転 
	if(bFlipH)
	{
		for(int32 i = 0; i < RenderParts.Num(); ++i)
		{
			for(int32 v = 0; v < 4; ++v)
			{
				RenderParts[i].Vertices[v].Position.X = 1.f - RenderParts[i].Vertices[v].Position.X;
			}
		}
	}
	// 垂直反転 
	if(bFlipV)
	{
		for(int32 i = 0; i < RenderParts.Num(); ++i)
		{
			for(int32 v = 0; v < 4; ++v)
			{
				RenderParts[i].Vertices[v].Position.Y = 1.f - RenderParts[i].Vertices[v].Position.Y;
			}
		}
	}

	// テクスチャ差し替え 
	if(0 < TextureReplacements.Num())
	{
		for(int32 i = 0; i < RenderParts.Num(); ++i)
		{
			TWeakObjectPtr<UTexture>* ReplaceTexture = TextureReplacements.Find(RenderParts[i].PartIndex);
			if(ReplaceTexture && (*ReplaceTexture).IsValid())
			{
				RenderParts[i].Texture = (*ReplaceTexture).Get();
			}
		}
	}

	Result.bUpdate = true;

	bFirstTick = false;
}

// 指定区間内のUserDataキーをResultに格納する
//   Start < Key <= End
void FSsPlayer::FindUserDataInInterval(FSsPlayerTickResult& Result, float Start, float End)
{
	float IntervalMin = FMath::Min(Start, End);
	float IntervalMax = FMath::Max(Start, End);

	const TArray<SsPartAndAnime>& PartAnime = Decoder->getPartAnime();
	for(int32 i = 0; i < PartAnime.Num(); ++i)
	{
		FSsPart* Part = PartAnime[i].Key;
		FSsPartAnime* Anime = PartAnime[i].Value;
		if((NULL == Part) || (NULL == Anime))
		{
			continue;
		}

		for(int32 j = 0; j < Anime->Attributes.Num(); ++j)
		{
			FSsAttribute* Attr = &(Anime->Attributes[j]);
			if(Attr->Tag != SsAttributeKind::User)
			{
				continue;
			}

			for(int32 k = 0; k < Attr->Key.Num(); ++k)
			{
				float FloatTime = (float)Attr->Key[k].Time;
				if(	   ((IntervalMin < FloatTime) && (FloatTime < IntervalMax))
					|| (FloatTime == End)
					)
				{
					FSsUserData NewUserData;
					NewUserData.PartName  = Part->PartName;
					NewUserData.PartIndex = i;
					NewUserData.KeyFrame  = Attr->Key[k].Time;

					FSsValue& Value = Attr->Key[k].Value;
					if((Value.Type == SsValueType::HashType) && (Value._Hash))
					{
						for(int32 l = 0; l < Value._Hash->Num(); ++l)
						{
							if(Value._Hash->Contains(TEXT("integer")))
							{
								NewUserData.Value.bUseInteger = true;
								NewUserData.Value.Integer = (*Value._Hash)[TEXT("integer")].get<int>();
							}
							if(Value._Hash->Contains(TEXT("rect")))
							{
								NewUserData.Value.bUseRect = true;
								FString temp = (*Value._Hash)[TEXT("rect")].get<FString>();
								TArray<FString> tempArr;
								tempArr.Empty(4);
								split_string(temp, ' ', tempArr);
								if(4 == tempArr.Num())
								{
									NewUserData.Value.Rect.Left   = (float)FCString::Atof(*tempArr[0]);
									NewUserData.Value.Rect.Top    = (float)FCString::Atof(*tempArr[1]);
									NewUserData.Value.Rect.Right  = (float)FCString::Atof(*tempArr[2]);
									NewUserData.Value.Rect.Bottom = (float)FCString::Atof(*tempArr[3]);
								}
							}
							if(Value._Hash->Contains(TEXT("point")))
							{
								NewUserData.Value.bUsePoint = true;
								FString temp = (*Value._Hash)[TEXT("point")].get<FString>();
								TArray<FString> tempArr;
								tempArr.Empty(2);
								split_string(temp, ' ', tempArr);
								if(2 == tempArr.Num())
								{
									NewUserData.Value.Point.X = (float)FCString::Atof(*tempArr[0]);
									NewUserData.Value.Point.Y = (float)FCString::Atof(*tempArr[1]);
								}
							}
							if(Value._Hash->Contains(TEXT("string")))
							{
								NewUserData.Value.bUseString = true;
								NewUserData.Value.String = (*Value._Hash)[TEXT("string")].get<FString>();
							}
						}
					}

					Result.UserData.Add(NewUserData);
				}
			}
		}
	}
}

const FVector2D FSsPlayer::GetAnimCanvasSize() const
{
	return (nullptr != Decoder) && (nullptr != Decoder->curAnimation) ? Decoder->curAnimation->Settings.CanvasSize : FVector2D(0,0);
}

// 再生
bool FSsPlayer::Play(int32 InAnimPackIndex, int32 InAnimationIndex, int32 StartFrame, float InPlayRate, int32 InLoopCount, bool bInRoundTrip)
{
	if(NULL == SsProject){ return false; }

	FSsAnimePack* AnimPack = (InAnimPackIndex < SsProject->AnimeList.Num()) ? &(SsProject->AnimeList[InAnimPackIndex]) : NULL;
	if(NULL == AnimPack){ return false; }

	FSsAnimation* Animation = (InAnimationIndex < AnimPack->AnimeList.Num()) ? &(AnimPack->AnimeList[InAnimationIndex]) : NULL;
	if(NULL == Animation){ return false; }

	CellMapList->set(SsProject.Get(), AnimPack);
	Decoder->setAnimation(&AnimPack->Model, Animation, CellMapList.Get(), SsProject.Get());
	Decoder->setPlayFrame( (float)StartFrame );

	bPlaying = true;
	bFirstTick = true;
	PlayRate = InPlayRate;
	LoopCount = InLoopCount;
	bRoundTrip = bInRoundTrip;
	AnimPivot = Animation->Settings.Pivot;
	PlayingAnimPackIndex = InAnimPackIndex;
	PlayingAnimationIndex = InAnimationIndex;

	return true;
}

// 再開
bool FSsPlayer::Resume()
{
	if((nullptr != Decoder) && (nullptr != Decoder->curAnimation))
	{
		bPlaying = true;
		return true;
	}
	return false;
}

// アニメーション名からインデックスを取得
bool FSsPlayer::GetAnimationIndex(const FName& InAnimPackName, const FName& InAnimationName, int32& OutAnimPackIndex, int32& OutAnimationIndex)
{
	if(SsProject.IsValid())
	{
		return SsProject->FindAnimationIndex(InAnimPackName, InAnimationName, OutAnimPackIndex, OutAnimationIndex);
	}
	return false;
}

// 指定フレーム送り
void FSsPlayer::SetPlayFrame(float Frame)
{
	if(nullptr != Decoder)
	{
		Decoder->setPlayFrame( Frame );
	}
}

// 現在フレーム取得
float FSsPlayer::GetPlayFrame() const
{
	if(nullptr != Decoder)
	{
		return Decoder->nowPlatTime;
	}
	return 0.f;
}

// 最終フレーム取得
float FSsPlayer::GetAnimeEndFrame() const
{
	if(nullptr != Decoder)
	{
		return Decoder->getAnimeEndFrame();
	}
	return 0.f;
}

// パーツ名からインデックスを取得
int32 FSsPlayer::GetPartIndexFromName(FName PartName) const
{
	if(nullptr != Decoder)
	{
		for(auto It = Decoder->partAnime.CreateConstIterator(); It; ++It)
		{
			if(It->Key == PartName)
			{
				return It.GetIndex();
			}
		}
	}
	return -1;
}

// パーツのTransformを取得
bool FSsPlayer::GetPartTransform(int32 PartIndex, FVector2D& OutPosition, float& OutRotate, FVector2D& OutScale) const
{
	if(nullptr != Decoder)
	{
		//TODO: 再実装 
		//return Decoder->GetPartTransform(PartIndex, OutPosition, OutRotate, OutScale);
	}
	return false;
}

// パーツのColorLabelを取得 
FName FSsPlayer::GetPartColorLabel(int32 PartIndex)
{
	if((nullptr != Decoder) && (0 <= PartIndex) && (PartIndex < Decoder->partAnime.Num()))
	{
		return Decoder->partAnime[PartIndex].Key->ColorLabel;
	}
	return FName();
}

// 非表示パーツの計算を行うかを設定 
void FSsPlayer::SetCalcHideParts(bool bInCalcHideParts)
{
	bCalcHideParts = bInCalcHideParts;
}
