﻿#include "SsPlayer.h"

#include "Ss6Project.h"
#include "SsAnimePack.h"
#include "SsString_uty.h"

#include "ssplayer_animedecode.h"
#include "ssplayer_PartState.h"
#include "ssplayer_effect2.h"
#include "ssplayer_matrix.h"
#include "ssplayer_mesh.h"


// コンストラクタ
FSsPlayer::FSsPlayer()
	: PlayRate(1.f)
	, LoopCount(-1)
	, bRoundTrip(false)
	, bFlipH(false)
	, bFlipV(false)
	, MulAlpha(1.f)
	, SsProject(nullptr)
	, Decoder(nullptr)
	, CellMapList(nullptr)
	, bPlaying(false)
	, bFirstTick(false)
	, AnimPivot(0.f,0.f)
	, PlayingAnimPackIndex(-1)
	, PlayingAnimationIndex(-1)
	, bCalcHideParts(false)
	, bPlayingSequence(false)
	, PlayingSequencePackIndex(-1)
	, PlayingSequenceIndex(-1)
	, PlayingSequenceFrame(0.f)
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
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayer_Tick);

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
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayer_TickAnimation);

	if(nullptr == Decoder)
	{
		return;
	}

	// 通常のアニメーション再生 
	if(!bPlayingSequence)
	{
		int32 AnimeStartFrame = Decoder->getAnimeStartFrame();
		int32 AnimeEndFrame   = Decoder->getAnimeEndFrame() + 1;	// データ上は「アニメーションの最後のフレーム」、ココで欲しいのは「ループの基準になる最終フレーム」 

		// 更新後のフレーム
		float BkAnimeFrame = Decoder->nowPlatTime;
		float AnimeFrame = BkAnimeFrame + (PlayRate * DeltaSeconds * Decoder->getAnimeFPS());

		// 再生開始フレームと同フレームに設定されたユーザーデータを拾うため、初回更新時のみBkAnimeFrameを誤魔化す
		if(bFirstTick)
		{
			BkAnimeFrame -= (.0f <= PlayRate) ? .1f : -.1f;
		}

		// ループ/往復処理 
		{
			// 最終フレーム以降で順方向再生
			if((AnimeEndFrame <= AnimeFrame) && (0.f < PlayRate))
			{
				if(0 < LoopCount)
				{
					// ループ回数更新
					--LoopCount;

					// ループ回数の終了
					if(0 == LoopCount)
					{
						AnimeFrame = (float)AnimeEndFrame;
						Pause();
						Result.bEndPlay = true;
					}
				}
				FindUserDataInInterval(Result, BkAnimeFrame, (float)AnimeEndFrame);

				if(bPlaying)
				{
					// アニメ時間より長いDeltaSecondsは考慮しない 
					float AnimeFrameSurplus = AnimeFrame - AnimeEndFrame;
					if((AnimeEndFrame - AnimeStartFrame) < AnimeFrameSurplus)
					{
						AnimeFrameSurplus = (AnimeEndFrame - AnimeStartFrame);
					}

					// 往復
					if(bRoundTrip)
					{
						AnimeFrame = AnimeEndFrame - AnimeFrameSurplus;
						PlayRate *= -1.f;
						FindUserDataInInterval(Result, (float)AnimeEndFrame, AnimeFrame);
					}
					// ループ
					else
					{
						if((AnimeEndFrame - AnimeStartFrame) <= AnimeFrameSurplus)
						{
							AnimeFrameSurplus -= (AnimeEndFrame - AnimeStartFrame);
						}
						AnimeFrame = AnimeStartFrame + AnimeFrameSurplus;
						FindUserDataInInterval(Result, -.1f, AnimeFrame);
					}
				}
			}
			// 開始フレーム以前で逆方向再生
			else if((AnimeFrame < AnimeStartFrame) && (PlayRate < 0.f))
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
					// アニメ時間より長いDeltaSecondsは考慮しない 
					float AnimeFrameSurplus = AnimeStartFrame - AnimeFrame;
					if((AnimeEndFrame - AnimeStartFrame) < AnimeFrameSurplus)
					{
						AnimeFrameSurplus = (AnimeEndFrame - AnimeStartFrame);
					}

					// 往復
					if(bRoundTrip)
					{
						AnimeFrame = AnimeStartFrame + AnimeFrameSurplus;
						PlayRate *= -1.f;
						FindUserDataInInterval(Result, 0.f, AnimeFrame);
					}
					// ループ
					else
					{
						AnimeFrame = AnimeEndFrame - AnimeFrameSurplus;
						FindUserDataInInterval(Result, (float)AnimeEndFrame+.1f, AnimeFrame);
					}
				}
			}
			else
			{
				FindUserDataInInterval(Result, BkAnimeFrame, AnimeFrame);
			}
		}

		// Decoder更新 
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayer_Tick_UpdateDecoder);

			Decoder->setPlayFrame( AnimeFrame );
			Decoder->update(DeltaSeconds * Decoder->getAnimeFPS());
		}
	}
	// シーケンス再生 
	else
	{
		check(0.f <= PlayRate);
		check((0 <= PlayingSequencePackIndex) && (PlayingSequencePackIndex < SsProject->SequenceList.Num()));
		check((0 <= PlayingSequenceIndex) && (PlayingSequenceIndex < SsProject->SequenceList[PlayingSequencePackIndex].SequenceList.Num()));

		FSsSequence* PlayingSequence = &(SsProject->SequenceList[PlayingSequencePackIndex].SequenceList[PlayingSequenceIndex]);
		check(PlayingSequence);

		// シーケンスFPS換算でアニメーション時間を進める 
		float DeltaFrame = PlayRate * DeltaSeconds * PlayingSequence->SequenceFPS;
		PlayingSequenceFrame = FMath::Max(0.f, PlayingSequenceFrame + DeltaFrame);

		// シーケンス再生終了時の処理. 
		if((PlayingSequence->SequenceFrameCount + 1) <= PlayingSequenceFrame)
		{
			switch(PlayingSequence->Type)
			{
				case SsSequenceType::Last:
					{
						// 最後のアイテムを繰り返し再生 
						// シーケンスFPS換算で最後のアニメーションのフレーム数分巻き戻す 
						int32 LastAnimFPS = SsProject->AnimeList[PlayingAnimPackIndex].AnimeList[PlayingAnimationIndex].Settings.Fps;
						int32 LastAnimFrameCount = SsProject->AnimeList[PlayingAnimPackIndex].AnimeList[PlayingAnimationIndex].Settings.FrameCount;
						PlayingSequenceFrame -= LastAnimFrameCount * ((float)PlayingSequence->SequenceFPS / (float)LastAnimFPS);
					} break;
				case SsSequenceType::Keep:
					{
						// 最終フレームを維持 
						// 動かないけれどアニメーション自体は再生状態とみなし、EndPlayも呼び出さない 
						PlayingSequenceFrame = PlayingSequence->SequenceFrameCount;
					} break;
				case SsSequenceType::Top:
					{
						// 全体を繰り返し再生 
						PlayingSequenceFrame -= PlayingSequence->SequenceFrameCount;
					} break;
			}
		}

		// シーケンスによるアニメーション切り替え 
		int32 AnimPackIndex, AnimationIndex, ItemIndex, RepeatCount, AnimationFrame;
		PlayingSequence->GetAnimationBySequenceFrame(SsProject.Get(), (int32)PlayingSequenceFrame, AnimPackIndex, AnimationIndex, ItemIndex, RepeatCount, AnimationFrame);
		if((PlayingAnimPackIndex != AnimPackIndex) || (PlayingAnimationIndex != AnimationIndex))
		{
			bool bBkPlaying = bPlaying;
			PlayInternal(AnimPackIndex, AnimationIndex, AnimationFrame, PlayRate, 0, false);
			bPlaying = bBkPlaying;	// Pause中のSetPlayFrame()によるアニメーション切り替えでポーズが解除されてしまう問題への対処 
		}

		// 再生開始フレームと同フレームに設定されたユーザーデータを拾うため、初回更新時のみBkAnimeFrameを誤魔化す 
		float BkAnimeFrame = Decoder->nowPlatTime;
		if(bFirstTick)
		{
			BkAnimeFrame -= (.0f <= PlayRate) ? .1f : -.1f;
		}
		FindUserDataInInterval(Result, BkAnimeFrame, (float)AnimationFrame);

		// Decoder更新 
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayer_Tick_UpdateDecoder);

			Decoder->setPlayFrame( (float)AnimationFrame );
			Decoder->update(PlayRate * DeltaSeconds * Decoder->getAnimeFPS());	// 再生中のアニメーションFPS換算のフレーム数で渡す 
		}
	}

	// 描画情報更新 
	RenderParts.Reset();
	CreateRenderParts(Decoder, GetAnimCanvasSize(), GetAnimPivot());

	// 水平反転 
	if(bFlipH)
	{
		for(auto ItPart = RenderParts.CreateIterator(); ItPart; ++ItPart)
		{
			for(auto ItVert = ItPart->Vertices.CreateIterator(); ItVert; ++ItVert)
			{
				ItVert->Position.X = 1.f - ItVert->Position.X;
			}
			for(auto ItMesh = ItPart->Mesh.CreateIterator(); ItMesh; ++ItMesh)
			{
				for(auto ItVert = ItMesh->Vertices.CreateIterator(); ItVert; ++ItVert)
				{
					ItVert->Position.X = 1.f - ItVert->Position.X;
				}
			}
		}
	}
	// 垂直反転 
	if(bFlipV)
	{
		for(auto ItPart = RenderParts.CreateIterator(); ItPart; ++ItPart)
		{
			for(auto ItVert = ItPart->Vertices.CreateIterator(); ItVert; ++ItVert)
			{
				ItVert->Position.Y = 1.f - ItVert->Position.Y;
			}
			for(auto ItMesh = ItPart->Mesh.CreateIterator(); ItMesh; ++ItMesh)
			{
				for(auto ItVert = ItMesh->Vertices.CreateIterator(); ItVert; ++ItVert)
				{
					ItVert->Position.Y = 1.f - ItVert->Position.Y;
				}
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
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayer_FindUserDataInInterval);

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
void FSsPlayer::CreateRenderParts(SsAnimeDecoder* RenderDecoder, const FVector2f& CanvasSize, const FVector2f& Pivot, bool bInstance)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayer_CreateRenderParts);

	for(auto It = RenderDecoder->sortList.CreateConstIterator(); It; ++It)
	{
		SsPartState* State = (*It);

		if(nullptr != State->refAnime)
		{
			if(!State->hide)
			{
				CreateRenderParts(State->refAnime, CanvasSize, Pivot, true);
			}
		}
		else if(nullptr != State->refEffect)
		{
			if(!State->hide)
			{
				CreateEffectRenderParts(RenderParts, State, CanvasSize, Pivot);
			}
		}
		else
		{
			FSsRenderPart& RenderPart = RenderParts.AddZeroed_GetRef();
			if(!CreateRenderPart(RenderPart, State, CanvasSize, Pivot, bInstance))
			{
				RenderParts.RemoveAt(RenderParts.Num()-1, 1, false);
			}
		}
	}
}

// 描画用パーツデータの作成（１パーツ分） 
bool FSsPlayer::CreateRenderPart(FSsRenderPart& OutRenderPart, const SsPartState* State, const FVector2f& CanvasSize, const FVector2f& Pivot, bool bInstance)
{
	if(nullptr == State){ return false; }
	float Alpha = (State->is_localAlpha ? State->localalpha : State->alpha) * MulAlpha;
	float HideAlpha = 1.f;
	bool bHideParts = false;
	if(!bCalcHideParts)
	{
		if(State->noCells){ return false; }
		if(nullptr == State->cellValue.cell){ return false; }
		if(nullptr == State->cellValue.texture){ return false; }
		if(State->hide){ return false; }
	}
	else
	{
		if(    (State->noCells)
			|| (nullptr == State->cellValue.cell)
			|| (nullptr == State->cellValue.texture)
			|| (State->hide)
			)
		{
			bHideParts = true;
			if((State->hide))
			{
				HideAlpha = 0.f;
			}
		}
	}

	// 描画対象外のパーツ種別は無視 
	if(    (State->partType == SsPartType::Armature)
		|| (State->partType == SsPartType::MoveNode)
		|| (State->partType == SsPartType::Constraint)
		|| (State->partType == SsPartType::Joint)
		|| (State->partType == SsPartType::BonePoint)
		)
	{
		return false;
	}

	// プログラム指定での非表示 
	if(!bInstance && (0 < HiddenParts.Num()) && HiddenParts.Contains(State->index))
	{
		return false;
	}

	OutRenderPart.PartIndex = State->index;
	OutRenderPart.Texture = bHideParts ? nullptr : State->cellValue.texture;	// 座標計算だけを行う非表示パーツはテクスチャをNULLにしておき、これを基準に描画をスキップする. 
	OutRenderPart.ColorBlendType = State->partsColorValue.blendType;
	OutRenderPart.AlphaBlendType = State->alphaBlendType;
	OutRenderPart.bMaskInfluence = State->maskInfluence;


	// RenderTargetに対する描画基準位置
	float OffX = (float)(CanvasSize.X /2) + (Pivot.X * CanvasSize.X);
	float OffY = (float)(CanvasSize.Y /2) - (Pivot.Y * CanvasSize.Y);

	FMatrix44f ViewMatrix(
		FVector3f(State->matrixLocal[ 0], State->matrixLocal[ 1], State->matrixLocal[ 2]),
		FVector3f(State->matrixLocal[ 4], State->matrixLocal[ 5], State->matrixLocal[ 6]),
		FVector3f(State->matrixLocal[ 8], State->matrixLocal[ 9], State->matrixLocal[10]),
		FVector3f(State->matrixLocal[12], State->matrixLocal[13], State->matrixLocal[14])
	);

	// 通常パーツ/マスクパーツ 
	if(State->partType != SsPartType::Mesh)
	{
		const int32 VertCnt = (State->is_vertex_transform || State->is_parts_color) ? 5 : 4;

		// 頂点座標
		FVector2f Vertices2D[5];
		for(int i = 0; i < VertCnt; ++i)
		{
			FVector4f V = ViewMatrix.TransformPosition(FVector3f(
				State->vertices[i*3 + 0],
				State->vertices[i*3 + 1],
				State->vertices[i*3 + 2]
			));
			Vertices2D[i] = FVector2f(V.X + OffX, -V.Y + OffY);
		}

		// UV
		FVector2f UVs[5];
		for(int i = 0; i < 4; ++i)
		{
			UVs[i] = FVector2f(State->cellValue.uvs[i].X + State->uvs[i*2 + 0] + State->uvTranslate.X, State->cellValue.uvs[i].Y + State->uvs[i*2 + 1] + State->uvTranslate.Y);
		}
		if(5 <= VertCnt)
		{
			UVs[4].X = UVs[4].Y = 0.f;
			for(int32 i = 0; i < 4; ++i)
			{
				UVs[4].X += UVs[i].X;
				UVs[4].Y += UVs[i].Y;
			}
			UVs[4].X /= 4.f;
			UVs[4].Y /= 4.f;
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
			FVector2f UVCenter((UVs[1].X - UVs[0].X) / 2.f + UVs[0].X, (UVs[2].Y - UVs[0].Y) / 2.f + UVs[0].Y);
			float S = FMath::Sin(FMath::DegreesToRadians(State->uvRotation));
			float C = FMath::Cos(FMath::DegreesToRadians(State->uvRotation));
			for(int i = 0; i < VertCnt; ++i)
			{
				UVs[i] -= UVCenter;
				UVs[i] = FVector2f(
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
			FVector2f tmp;
			tmp = UVs[0];
			UVs[0] = UVs[1];
			UVs[1] = tmp;
			tmp = UVs[2];
			UVs[2] = UVs[3];
			UVs[3] = tmp;
		}
		if(State->imageFlipV)
		{
			FVector2f tmp;
			tmp = UVs[0];
			UVs[0] = UVs[2];
			UVs[2] = tmp;
			tmp = UVs[1];
			UVs[1] = UVs[3];
			UVs[3] = tmp;
		}

		// 頂点カラー 
		FColor VertexColors[5];
		float ColorBlendRate[5];
		if(State->is_parts_color)
		{
			if(State->partsColorValue.target == SsColorBlendTarget::Whole)
			{
				const SsColorBlendValue& cbv = State->partsColorValue.color;
				VertexColors[0].R = cbv.rgba.r;
				VertexColors[0].G = cbv.rgba.g;
				VertexColors[0].B = cbv.rgba.b;
				VertexColors[0].A = (uint8)(cbv.rgba.a * Alpha * HideAlpha);
				ColorBlendRate[0] = cbv.rate;

				for(int32 i = 1; i < VertCnt; ++i)
				{
					VertexColors[i] = VertexColors[0];
					ColorBlendRate[i] = cbv.rate;
				}
			}
			else
			{
				if(SsBlendType::Mix == State->partsColorValue.blendType)
				{
					for(int32 i = 0; i < 4; ++i)
					{
						const SsColorBlendValue& cbv = State->partsColorValue.colors[i];
						VertexColors[i].R = cbv.rgba.r;
						VertexColors[i].G = cbv.rgba.g;
						VertexColors[i].B = cbv.rgba.b;
						VertexColors[i].A = (uint8)(255 * Alpha * HideAlpha);
						ColorBlendRate[i] = (float)(cbv.rgba.a / 255.f);
					}
				}
				else
				{
					for(int32 i = 0; i < 4; ++i)
					{
						const SsColorBlendValue& cbv = State->partsColorValue.colors[i];
						VertexColors[i].R = cbv.rgba.r;
						VertexColors[i].G = cbv.rgba.g;
						VertexColors[i].B = cbv.rgba.b;
						VertexColors[i].A = (uint8)(cbv.rgba.a * Alpha * HideAlpha);
						ColorBlendRate[i] = cbv.rate;
					}
				}
				if(5 <= VertCnt)
				{
					uint32 R(0), G(0), B(0), A(0);
					float Rate(0.f);
					for (int i = 0; i < 4; i++)
					{
						R += VertexColors[i].R;
						G += VertexColors[i].G;
						B += VertexColors[i].B;
						A += VertexColors[i].A;
						Rate += ColorBlendRate[i];
					}
					VertexColors[4].R = R / 4;
					VertexColors[4].G = G / 4;
					VertexColors[4].B = B / 4;
					VertexColors[4].A = A / 4;
					ColorBlendRate[4] = Rate / 4.f;
				}
			}
		}
		else
		{
			for(int32 i = 0; i < VertCnt; ++i)
			{
				VertexColors[i] = FColor(255, 255, 255, (uint8)(255 * Alpha * HideAlpha));
				ColorBlendRate[i] = 1.f;
			}
		}

		if(State->partType == SsPartType::Mask)
		{
			OutRenderPart.ColorBlendType = SsBlendType::Mask;
			for(int32 i = 0; i < VertCnt; ++i)
			{
				VertexColors[i].A = State->masklimen;
			}
		}

		OutRenderPart.Vertices.AddUninitialized(VertCnt);
		for(int32 i = 0; i < VertCnt; ++i)
		{
			OutRenderPart.Vertices[i].Position = FVector2f(Vertices2D[i].X/CanvasSize.X, Vertices2D[i].Y/CanvasSize.Y);
			OutRenderPart.Vertices[i].TexCoord = UVs[i];
			OutRenderPart.Vertices[i].Color = VertexColors[i];
			OutRenderPart.Vertices[i].ColorBlendRate = ColorBlendRate[i];
		}
	}
	// メッシュパーツ 
	else
	{
		if((nullptr == State->meshPart) || (nullptr == State->meshPart->vertices))
		{
			return false;
		}
		int32 Idx = OutRenderPart.Mesh.Add(FSsRenderMesh());
		check(0 == Idx);
		FSsRenderMesh& RenderMesh = OutRenderPart.Mesh[Idx];


		FVector2f UVSize(
			(State->cellValue.uvs[3].X + State->cellValue.uvs[0].X) / 2.f,
			(State->cellValue.uvs[3].Y + State->cellValue.uvs[0].Y) / 2.f
		);
		float S = FMath::Sin(FMath::DegreesToRadians(State->uvRotation));
		float C = FMath::Cos(FMath::DegreesToRadians(State->uvRotation));

		RenderMesh.Vertices.AddUninitialized(State->meshPart->ver_size);
		for(int32 i = 0; i < State->meshPart->ver_size; ++i)
		{
			if(State->meshPart->isBind)
			{
				RenderMesh.Vertices[i].Position.X = ( State->meshPart->draw_vertices[i*3 + 0] + OffX) / CanvasSize.X;
				RenderMesh.Vertices[i].Position.Y = (-State->meshPart->draw_vertices[i*3 + 1] + OffY) / CanvasSize.Y;
			}
			else
			{
				bool bDeform = i < State->deformValue.verticeChgList.Num();
				FVector4f V = ViewMatrix.TransformPosition(FVector3f(
					State->meshPart->vertices[i*3 + 0] + (bDeform ? State->deformValue.verticeChgList[i].X : 0.f),
					State->meshPart->vertices[i*3 + 1] + (bDeform ? State->deformValue.verticeChgList[i].Y : 0.f),
					State->meshPart->vertices[i*3 + 2]
				));
				RenderMesh.Vertices[i].Position.X = ( V.X + OffX) / CanvasSize.X;
				RenderMesh.Vertices[i].Position.Y = (-V.Y + OffY) / CanvasSize.Y;
			}

			FVector2f UV(
				State->meshPart->uvs[i*2 + 0],
				State->meshPart->uvs[i*2 + 1]
			);
			UV -= UVSize;
			UV *= State->uvScale;
			if(State->imageFlipH)
			{
				UV.X *= -1.f;
			}
			if(State->imageFlipV)
			{
				UV.Y *= -1.f;
			}
			UV = FVector2f(
				UV.X * C - UV.Y * S,
				UV.X * S + UV.Y * C
			);
			UV += UVSize;
			UV += State->uvTranslate;

			RenderMesh.Vertices[i].TexCoord = UV;
		}

		RenderMesh.Indices.AddUninitialized(State->meshPart->tri_size * 3);
		for(int32 i = 0; i < (State->meshPart->tri_size * 3); ++i)
		{
			RenderMesh.Indices[i] = (uint16)State->meshPart->indices[i];
		}

		RenderMesh.Color.R = State->partsColorValue.color.rgba.r;
		RenderMesh.Color.G = State->partsColorValue.color.rgba.g;
		RenderMesh.Color.B = State->partsColorValue.color.rgba.b;
		RenderMesh.Color.A = (uint8)(255 * Alpha * HideAlpha);
		RenderMesh.ColorBlendRate = State->partsColorValue.color.rgba.a / 255.f;
	}

	return true;
}

// エフェクト描画用パーツデータの作成 
void FSsPlayer::CreateEffectRenderParts(TArray<FSsRenderPart>& OutRenderParts, const SsPartState* State, const FVector2f& CanvasSize, const FVector2f& Pivot)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayer_CreateEffectRenderParts);

	if(nullptr == State){ return; }
	if(nullptr == State->refEffect){ return; }
	if(State->refEffect->nowFrame < 0){ return; }

	SsEffectRenderV2* Effect = State->refEffect;

	for(auto It = Effect->updateList.CreateIterator(); It; ++It)
	{
		if(*It)
		{
			(*It)->setSeedOffset(Effect->seedOffset);
		}
	}
	for(auto It = Effect->updateList.CreateIterator(); It; ++It)
	{
		SsEffectEmitter* Emitter = (*It);
		if(Emitter && Emitter->_parent)
		{
			//グローバルの時間で現在親がどれだけ生成されているのかをチェックする
			Emitter->_parent->updateEmitter(Effect->targetFrame, 0);

			int32 LoopNum = Emitter->_parent->getParticleIDMax();
			for(int32 n = 0; n < LoopNum; ++n)
			{
				const particleExistSt* ExistSt = Emitter->_parent->getParticleDataFromID(n);
				if(ExistSt->born)
				{
					particleDrawData DrawData;
					DrawData.stime = ExistSt->stime;
					DrawData.lifetime = ExistSt->endtime;
					DrawData.id = n;
					DrawData.pid = 0;

					CreateEffectRenderPart(OutRenderParts, State, CanvasSize, Pivot, Emitter, (Effect->targetFrame - DrawData.stime), Emitter->_parent, &DrawData);
				}
			}
		}
		else
		{
			CreateEffectRenderPart(OutRenderParts, State, CanvasSize, Pivot, Emitter, Effect->targetFrame);
		}
	}
}

// エフェクト描画用パーツデータの作成（１パーツ分） 
void FSsPlayer::CreateEffectRenderPart(TArray<FSsRenderPart>& OutRenderParts, const SsPartState* State, const FVector2f& CanvasSize, const FVector2f& Pivot, SsEffectEmitter* Emitter, float Time, SsEffectEmitter* Parent, const particleDrawData* DrawData)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayer_CreateEffectRenderPart);

	// 参照：SsEffectRenderV2::particleDraw()

	if(nullptr == State){ return; }
	if(nullptr == Emitter){ return; }

	SsEffectRenderV2* Effect = State->refEffect;

	int32 ParticleNum = Emitter->getParticleIDMax();
	int32 Slide = (Parent == nullptr) ? 0 : DrawData->id;

	Emitter->updateEmitter(Time, Slide);

	float ParentMatrix[4 * 4];
	float ParentAlpha = 1.f;
	if(Effect->parentState)
	{
		memcpy(ParentMatrix, Effect->parentState->matrixLocal, sizeof(float)*16);
		ParentAlpha = (Effect->parentState->localalpha == 1.f) ? Effect->parentState->alpha : Effect->parentState->localalpha;
	}
	else
	{
		IdentityMatrix(ParentMatrix);
	}

	for(int32 Id = 0; Id < ParticleNum; ++Id)
	{
		const particleExistSt* ExistSt = Emitter->getParticleDataFromID(Id);
		if(!ExistSt->born)
		{
			continue;
		}

		float TargetTime = (Time + 0.f);
		particleDrawData lp;
		particleDrawData pp;
		pp.x = pp.y = 0.f;

		lp.id = Id + ExistSt->cycle;
		lp.stime = ExistSt->stime;
		lp.lifetime = ExistSt->endtime;
		lp.pid = 0;

		if(Parent)
		{
			lp.pid = DrawData->id;
		}

		if(ExistSt->exist)
		{
			if(Parent)
			{
				//親から描画するパーティクルの初期位置を調べる 
				pp.id = DrawData->id;
				pp.stime = DrawData->stime;
				pp.lifetime = DrawData->lifetime;
				pp.pid = DrawData->pid;

				//パーティクルが発生した時間の親の位置を取る 
				int ptime = lp.stime + pp.stime;
				if (ptime > lp.lifetime) { ptime = lp.lifetime; }

				//逆算はデバッグしずらいかもしれない 
				Parent->updateParticle(lp.stime + pp.stime, &pp);
				Emitter->position.X = pp.x;
				Emitter->position.Y = pp.y;
			}

			Emitter->updateParticle(TargetTime, &lp);

			SsFColor fcolor;
			fcolor.fromARGB(lp.color.ToARGB());


			{
				float matrix[4 * 4];
				memcpy(matrix, ParentMatrix, sizeof(float) * 16);

				TranslationMatrixM(matrix, lp.x * Effect->layoutScale.X, lp.y * Effect->layoutScale.Y, 0.f);
				RotationXYZMatrixM(matrix, 0.f, 0.f, FMath::DegreesToRadians(lp.rot) + lp.direc);
				ScaleMatrixM(matrix, lp.scale.X, lp.scale.Y, 1.f);

				fcolor.a *= ParentAlpha;

				if(Emitter->dispCell.cell && (0.f < fcolor.a))
				{
					FVector2f pivot = Emitter->dispCell.cell->Pivot;
					pivot.X *= Emitter->dispCell.cell->Size.X;
					pivot.Y *= Emitter->dispCell.cell->Size.Y;

					FVector2f dispscale = Emitter->dispCell.cell->Size;


					// RenderTargetに対する描画基準位置
					float OffX = (float)(CanvasSize.X / 2) + pivot.X + Pivot.X * CanvasSize.X;
					float OffY = (float)(CanvasSize.Y / 2) + pivot.Y - Pivot.Y * CanvasSize.Y;

					// 頂点座標
					FMatrix44f ViewMatrix(
						FVector3f(matrix[0], matrix[1], matrix[2]),
						FVector3f(matrix[4], matrix[5], matrix[6]),
						FVector3f(matrix[8], matrix[9], matrix[10]),
						FVector3f(matrix[12], matrix[13], matrix[14])
					);
					FVector3f Vertices[4] =
					{
						FVector3f(-(dispscale.X / 2.f),  (dispscale.Y / 2.f), 0.f),
						FVector3f( (dispscale.X / 2.f),  (dispscale.Y / 2.f), 0.f),
						FVector3f(-(dispscale.X / 2.f), -(dispscale.Y / 2.f), 0.f),
						FVector3f( (dispscale.X / 2.f), -(dispscale.Y / 2.f), 0.f),
					};

					FSsRenderPart& RenderPart = OutRenderParts.AddZeroed_GetRef();
					RenderPart.PartIndex = State->index;
					RenderPart.Texture = Emitter->dispCell.texture;
					RenderPart.AlphaBlendType = SsRenderBlendTypeToBlendType(Emitter->refData->BlendType);
					RenderPart.ColorBlendType = SsBlendType::Effect;
					RenderPart.bMaskInfluence = State->maskInfluence;

					RenderPart.Vertices.AddUninitialized(4);
					for (int32 i = 0; i < 4; ++i)
					{
						FVector4f V = ViewMatrix.TransformPosition(Vertices[i]);
						RenderPart.Vertices[i].Position.X = ( V.X + OffX) / CanvasSize.X;
						RenderPart.Vertices[i].Position.Y = (-V.Y + OffY) / CanvasSize.Y;

						RenderPart.Vertices[i].TexCoord = Emitter->dispCell.uvs[i];
						RenderPart.Vertices[i].Color = FColor(lp.color.R, lp.color.G, lp.color.B, (uint8)(lp.color.A * ParentAlpha * MulAlpha));
						RenderPart.Vertices[i].ColorBlendRate = (Emitter->particle.useColor || Emitter->particle.useTransColor) ? 1.f : 0.f;
					}
				}
			}
		}
	}
}

// 再生中にアニメーションのCanvasSizeの取得 
const FVector2f FSsPlayer::GetAnimCanvasSize() const
{
	return (nullptr != Decoder) && (nullptr != Decoder->curAnimation) ? Decoder->curAnimation->Settings.CanvasSize : FVector2f(0,0);
}

// 再生
bool FSsPlayer::Play(int32 InAnimPackIndex, int32 InAnimationIndex, int32 InStartFrame, float InPlayRate, int32 InLoopCount, bool bInRoundTrip)
{
	if(!SsProject.IsValid() || (InAnimPackIndex < 0) || (InAnimationIndex < 0))
	{
		return false;
	}

	if(PlayInternal(InAnimPackIndex, InAnimationIndex, InStartFrame, InPlayRate, InLoopCount, bInRoundTrip))
	{
		bPlayingSequence = false;
		PlayingSequencePackIndex = -1;
		PlayingSequenceIndex = -1;
		PlayingSequenceFrame = 0.f;
		return true;
	}
	return false;
}

bool FSsPlayer::PlayInternal(int32 InAnimPackIndex, int32 InAnimationIndex, int32 InStartFrame, float InPlayRate, int32 InLoopCount, bool bInRoundTrip)
{
	FSsAnimePack* AnimPack = (InAnimPackIndex < SsProject->AnimeList.Num()) ? &(SsProject->AnimeList[InAnimPackIndex]) : nullptr;
	if(nullptr == AnimPack){ return false; }

	FSsAnimation* Animation = (InAnimationIndex < AnimPack->AnimeList.Num()) ? &(AnimPack->AnimeList[InAnimationIndex]) : nullptr;
	if(nullptr == Animation){ return false; }

	if(PlayingAnimPackIndex != InAnimPackIndex)
	{
		CellMapList->set(SsProject.Get(), AnimPack);
	}
	Decoder->setAnimation(&AnimPack->Model, Animation, CellMapList, SsProject.Get());
	Decoder->setPlayFrame((float)(Decoder->getAnimeStartFrame() + InStartFrame));

	bPlaying = true;
	bFirstTick = true;
	PlayRate = InPlayRate;
	LoopCount = InLoopCount;
	bRoundTrip = bInRoundTrip;
	AnimPivot = Animation->Settings.Pivot;
	PlayingAnimPackIndex = InAnimPackIndex;
	PlayingAnimationIndex = InAnimationIndex;

	ResetPartHidden();
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

// シーケンスの再生 
bool FSsPlayer::PlaySequence(int32 InSequencePackIndex, int32 InSequenceIndex, int32 InStartFrame, float InPlayRate)
{
	if(InPlayRate < 0.f)
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("PlaySequence does not support negative PlayRate."));
		InPlayRate = 0.f;
	}

	if(!SsProject.IsValid() || (InSequencePackIndex < 0) || (InSequenceIndex < 0))
	{
		return false;
	}

	FSsSequencePack* SequencePack = (InSequencePackIndex < SsProject->SequenceList.Num()) ? &(SsProject->SequenceList[InSequencePackIndex]) : nullptr;
	if(nullptr == SequencePack){ return false; }

	FSsSequence* Sequence = (InSequenceIndex < SequencePack->SequenceList.Num()) ? &(SequencePack->SequenceList[InSequenceIndex]) : nullptr;
	if(nullptr == Sequence){ return false; }

	int32 AnimPackIndex, AnimationIndex, ItemIndex, RepeatCount, AnimationFrame;
	if(Sequence->GetAnimationBySequenceFrame(SsProject.Get(), InStartFrame, AnimPackIndex, AnimationIndex, ItemIndex, RepeatCount, AnimationFrame))
	{
		if(PlayInternal(AnimPackIndex, AnimationIndex, AnimationFrame, InPlayRate, 0, false))
		{
			bPlayingSequence = true;
			PlayingSequencePackIndex = InSequencePackIndex;
			PlayingSequenceIndex = InSequenceIndex;
			PlayingSequenceFrame = InStartFrame;
			return true;
		}
	}

	return false;
}

// シーケンス名からインデックスを取得 
bool FSsPlayer::GetSequenceIndex(const FName& InSequencePackName, const FName& InSequenceName, int32& OutSequencePackIndex, int32& OutSequneceIndex) const
{
	if(!SsProject.IsValid())
	{
		return false;
	}

	OutSequencePackIndex = SsProject->FindSequencePackIndex(InSequencePackName);
	if(0 <= OutSequencePackIndex)
	{
		FSsSequencePack* SequencePack = &(SsProject->SequenceList[OutSequencePackIndex]);
		OutSequneceIndex = SequencePack->FindSequenceIndex(InSequenceName);
		return (0 <= OutSequneceIndex);
	}

	return false;
}
// シーケンスIDからインデックスを取得 
bool FSsPlayer::GetSequenceIndexById(const FName& InSequencePackName, int32 InSequenceId, int32& OutSequencePackIndex, int32& OutSequneceIndex) const
{
	if(!SsProject.IsValid())
	{
		return false;
	}

	OutSequencePackIndex = SsProject->FindSequencePackIndex(InSequencePackName);
	if(0 <= OutSequencePackIndex)
	{
		FSsSequencePack* SequencePack = &(SsProject->SequenceList[OutSequencePackIndex]);
		for(int32 i = 0; i < SequencePack->SequenceList.Num(); ++i)
		{
			if(InSequenceId == SequencePack->SequenceList[i].Id)
			{
				OutSequneceIndex = i;
				return true;
			}
		}
	}

	return false;
}


// 指定フレーム送り
void FSsPlayer::SetPlayFrame(float Frame)
{
	if(nullptr == Decoder)
	{
		return;
	}

	if(bPlayingSequence)
	{
		PlayingSequenceFrame = Frame;

		FSsPlayerTickResult Result;
		TickAnimation(0.f, Result);
	}
	else
	{
		Decoder->setPlayFrame(Decoder->getAnimeStartFrame() + Frame);
	}
}

// 現在フレーム取得
float FSsPlayer::GetPlayFrame() const
{
	if(nullptr == Decoder)
	{
		return 0.f;
	}

	if(bPlayingSequence)
	{
		return PlayingSequenceFrame;
	}
	else
	{
		return Decoder->nowPlatTime - Decoder->getAnimeStartFrame();
	}
}

// 最終フレーム取得
float FSsPlayer::GetAnimeEndFrame() const
{
	if(nullptr == Decoder)
	{
		return 0.f;
	}

	if(bPlayingSequence)
	{
		check((0 <= PlayingSequencePackIndex) && (PlayingSequencePackIndex < SsProject->SequenceList.Num()));
		check((0 <= PlayingSequenceIndex) && (PlayingSequenceIndex < SsProject->SequenceList[PlayingSequencePackIndex].SequenceList.Num()));

		FSsSequence* PlayingSequence = &(SsProject->SequenceList[PlayingSequencePackIndex].SequenceList[PlayingSequenceIndex]);
		check(PlayingSequence);

		return PlayingSequence->SequenceFrameCount;
	}
	else
	{
		return (nullptr != Decoder->curAnimation) ? Decoder->curAnimation->GetFrameCount() : 0.f;
	}
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
bool FSsPlayer::GetPartTransform(int32 PartIndex, FVector2f& OutPosition, float& OutRotate, FVector2f& OutScale) const
{
	if((nullptr == Decoder) || (PartIndex < 0) || (Decoder->sortList.Num() <= PartIndex))
	{
		return false;
	}

	SsPartState* State = &(Decoder->partState[PartIndex]);
	FVector2f Pivot = GetAnimPivot();
	FVector2f CanvasSize = GetAnimCanvasSize();
	OutPosition = FVector2f(
		State->matrixLocal[12] + ((float)(CanvasSize.X /2) + (Pivot.X * CanvasSize.X)),
		State->matrixLocal[13] - ((float)(CanvasSize.Y /2) - (Pivot.Y * CanvasSize.Y))
	);
	OutPosition.X = OutPosition.X / CanvasSize.X;
	OutPosition.Y = OutPosition.Y / CanvasSize.Y;

	OutRotate = State->rotation.Z;
	OutScale = State->scale * State->localscale;
	for(SsPartState* ParentState = State->parent; NULL != ParentState; ParentState = ParentState->parent)
	{
		OutRotate += ParentState->rotation.Z;
		OutScale *= ParentState->scale;
	}
	while(OutRotate <   0.f){ OutRotate += 360.f; }
	while(OutRotate > 360.f){ OutRotate -= 360.f; }

	return true;
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

// プログラム指定でのパーツ非表示 
void FSsPlayer::SetPartHidden(int32 PartIndex, bool bHidden)
{
	if(bHidden)
	{
		HiddenParts.AddUnique(PartIndex);
	}
	else
	{
		HiddenParts.Remove(PartIndex);
	}
}

// プログラム指定でのパーツ非表示状態をリセット 
void FSsPlayer::ResetPartHidden()
{
	HiddenParts.Empty();
}
