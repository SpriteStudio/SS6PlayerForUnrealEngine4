#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayer.h"

#include "Ss6Project.h"
#include "SsAnimePack.h"
#include "SsString_uty.h"

#include "ssplayer_animedecode.h"
#include "ssplayer_PartState.h"

// コンストラクタ
FSsPlayer::FSsPlayer()
	: PlayRate(1.f)
	, LoopCount(-1)
	, bRoundTrip(false)
	, bFlipH(false)
	, bFlipV(false)
	, SsProject(nullptr)
	, Decoder(nullptr)
	, CellMapList(nullptr)
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
	if(nullptr != CellMapList)
	{
		delete CellMapList;
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
	CellMapList = new SsCellMapList();

	PlayingAnimPackIndex = -1;
	PlayingAnimationIndex = -1;
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
		Decoder->ReloadEffects();	// コレは中身が #if 0 で消されてるので削除でOK. 旧エフェクトシステムの残骸かな？ 
	}
	Decoder->Update();

	RenderParts.Empty();
	Decoder->CreateRenderParts(RenderParts);
*/
	//TODO:	更新手順を再確認 
	//		CreateRenderParts相当の処理はPlayer側で実装する 
	Decoder->update(DeltaSeconds * Decoder->getAnimeFPS());

	RenderParts.Empty();
	CreateRenderParts(Decoder, GetAnimCanvasSize(), GetAnimPivot());

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

// 描画用パーツデータの作成 
void FSsPlayer::CreateRenderParts(SsAnimeDecoder* RenderDecoder, const FVector2D& CanvasSize, const FVector2D& Pivot)
{
	for(auto It = RenderDecoder->sortList.CreateConstIterator(); It; ++It)
	{
		SsPartState* State = (*It);

		if(nullptr != State->refAnime)
		{
			if(!State->hide)
			{
				CreateRenderParts(State->refAnime, CanvasSize, Pivot);
			}
		}
		else if(nullptr != State->refEffect)
		{
			if(!State->hide)
			{
				//TODO:
				//CreateEffectRenderParts(State->refEffect, State, CanvasSize, Pivot);
			}
		}
		else
		{
			FSsRenderPart RenderPart;
			if(CreateRenderPart(RenderPart, State, CanvasSize, Pivot))
			{
				RenderParts.Add(RenderPart);
			}
		}
	}
}

// 描画用パーツデータの作成（１パーツ分） 
bool FSsPlayer::CreateRenderPart(FSsRenderPart& OutRenderPart, const SsPartState* State, const FVector2D& CanvasSize, const FVector2D& Pivot)
{
	if(nullptr == State){ return false; }
	if(State->noCells){ return false; }
	if(nullptr == State->cellValue.cell){ return false; }
	if(nullptr == State->cellValue.texture){ return false; }
	float HideAlpha = 1.f;
	if(!bCalcHideParts)
	{
		if(State->hide){ return false; }
		if(0.f == State->alpha){ return false; }
	}
	else
	{
		if(State->hide)
		{
			HideAlpha = 0.f;
		}
	}

	//TODO: 今は一旦Ss5Pのそのまま移植. 
	//		Ss6の新機能が色々あるので、通して見直す. 
	//		LocalScaleとか. 

	// RenderTargetに対する描画基準位置
	float OffX = (float)(CanvasSize.X /2) + (Pivot.X * CanvasSize.X);
	float OffY = (float)(CanvasSize.Y /2) - (Pivot.Y * CanvasSize.Y);

	// 頂点座標
	FMatrix ViewMatrix(
		FVector(State->matrix[ 0], State->matrix[ 1], State->matrix[ 2]),
		FVector(State->matrix[ 4], State->matrix[ 5], State->matrix[ 6]),
		FVector(State->matrix[ 8], State->matrix[ 9], State->matrix[10]),
		FVector(State->matrix[12], State->matrix[13], State->matrix[14])
	);
	FVector2D Vertices2D[4];
	for(int i = 0; i < 4; ++i)
	{
		FVector4 V = ViewMatrix.TransformPosition(FVector(
			State->vertices[i*3 + 0],
			State->vertices[i*3 + 1],
			State->vertices[i*3 + 2]
		));
		Vertices2D[i] = FVector2D(V.X + OffX, -V.Y + OffY);
	}

	// 上下反転，左右反転
	if(State->hFlip)
	{
		FVector2D tmp;
		tmp = Vertices2D[0];
		Vertices2D[0] = Vertices2D[1];
		Vertices2D[1] = tmp;
		tmp = Vertices2D[2];
		Vertices2D[2] = Vertices2D[3];
		Vertices2D[3] = tmp;
	}
	if(State->vFlip)
	{
		FVector2D tmp;
		tmp = Vertices2D[0];
		Vertices2D[0] = Vertices2D[2];
		Vertices2D[2] = tmp;
		tmp = Vertices2D[1];
		Vertices2D[1] = Vertices2D[3];
		Vertices2D[3] = tmp;
	}

	// UV
	FVector2D UVs[4];
	for(int i = 0; i < 4; ++i)
	{
		UVs[i] = FVector2D(State->cellValue.uvs[i].X + State->uvs[i*2 + 0] + State->uvTranslate.X, State->cellValue.uvs[i].Y + State->uvs[i*2 + 1] + State->uvTranslate.Y);
	}
	if(1.f != State->uvScale.X)
	{
		float Center;
		Center = (UVs[1].X - UVs[0].X) / 2.f + UVs[0].X;
		UVs[0].X = Center - ((Center - UVs[0].X) * State->uvScale.X);
		UVs[1].X = Center - ((Center - UVs[1].X) * State->uvScale.X);
		Center = (UVs[3].X - UVs[2].X) / 2.f + UVs[2].X;
		UVs[2].X = Center - ((Center - UVs[2].X) * State->uvScale.X);
		UVs[3].X = Center - ((Center - UVs[3].X) * State->uvScale.X);
	}
	if(0.f != State->uvRotation)
	{
		FVector2D UVCenter((UVs[1].X - UVs[0].X) / 2.f + UVs[0].X, (UVs[2].Y - UVs[0].Y) / 2.f + UVs[0].Y);
		float S = FMath::Sin(FMath::DegreesToRadians(State->uvRotation));
		float C = FMath::Cos(FMath::DegreesToRadians(State->uvRotation));
		for(int i = 0; i < 4; ++i)
		{
			UVs[i] -= UVCenter;
			UVs[i] = FVector2D(
				UVs[i].X * C - UVs[i].Y * S,
				UVs[i].X * S + UVs[i].Y * C
			);
			UVs[i] += UVCenter;
		}
	}
	if(1.f != State->uvScale.Y)
	{
		float Center;
		Center = (UVs[2].Y - UVs[0].Y) / 2.f + UVs[0].Y;
		UVs[0].Y = Center - ((Center - UVs[0].Y) * State->uvScale.Y);
		UVs[2].Y = Center - ((Center - UVs[2].Y) * State->uvScale.Y);
		Center = (UVs[3].Y - UVs[1].Y) / 2.f + UVs[1].Y;
		UVs[1].Y = Center - ((Center - UVs[1].Y) * State->uvScale.Y);
		UVs[3].Y = Center - ((Center - UVs[3].Y) * State->uvScale.Y);
	}

	// イメージ反転
	if(State->imageFlipH)
	{
		FVector2D tmp;
		tmp = UVs[0];
		UVs[0] = UVs[1];
		UVs[1] = tmp;
		tmp = UVs[2];
		UVs[2] = UVs[3];
		UVs[3] = tmp;
	}
	if(State->imageFlipV)
	{
		FVector2D tmp;
		tmp = UVs[0];
		UVs[0] = UVs[2];
		UVs[2] = tmp;
		tmp = UVs[1];
		UVs[1] = UVs[3];
		UVs[3] = tmp;
	}

	// 頂点カラー
	FColor VertexColors[4];
	float ColorBlendRate[4];
	if(State->is_color_blend)
	{
		if(State->colorValue.target == SsColorBlendTarget::Whole)
		{
			const SsColorBlendValue& cbv = State->colorValue.color;
			VertexColors[0].R = cbv.rgba.r;
			VertexColors[0].G = cbv.rgba.g;
			VertexColors[0].B = cbv.rgba.b;
			VertexColors[0].A = (uint8)(cbv.rgba.a * State->alpha * HideAlpha);
			ColorBlendRate[0] = cbv.rate;

			for(int32 i = 1; i < 4; ++i)
			{
				VertexColors[i] = VertexColors[0];
				ColorBlendRate[i] = cbv.rate;
			}
		}
		else
		{
			for(int32 i = 0; i < 4; ++i)
			{
				const SsColorBlendValue& cbv = State->colorValue.colors[i];
				VertexColors[i].R = cbv.rgba.r;
				VertexColors[i].G = cbv.rgba.g;
				VertexColors[i].B = cbv.rgba.b;
				VertexColors[i].A = (uint8)(cbv.rgba.a * State->alpha * HideAlpha);
				ColorBlendRate[i] = cbv.rate;
			}
		}
	}
	else
	{
		const SsColorBlendValue& cbv = State->colorValue.color;
		for(int32 i = 0; i < 4; ++i)
		{
			VertexColors[i] = FColor(255, 255, 255, (uint8)(255 * State->alpha * HideAlpha));
			ColorBlendRate[i] = 1.f;
		}
	}

	OutRenderPart.PartIndex = State->index;
	OutRenderPart.Texture = State->cellValue.texture;
	OutRenderPart.ColorBlendType = State->colorValue.blendType;
	OutRenderPart.AlphaBlendType = State->alphaBlendType;
	for(int32 i = 0; i < 4; ++i)
	{
		OutRenderPart.Vertices[i].Position = FVector2D(Vertices2D[i].X/CanvasSize.X, Vertices2D[i].Y/CanvasSize.Y);
		OutRenderPart.Vertices[i].TexCoord = UVs[i];
		OutRenderPart.Vertices[i].Color = VertexColors[i];
		OutRenderPart.Vertices[i].ColorBlendRate = ColorBlendRate[i];
	}
	return true;
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
	Decoder->setAnimation(&AnimPack->Model, Animation, CellMapList, SsProject.Get());
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
			if(It->Key->PartName == PartName)
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
