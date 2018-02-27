#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayer.h"

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
	Decoder->setPlayFrame( AnimeFrame );
	Decoder->update(DeltaSeconds * Decoder->getAnimeFPS());

	// 描画情報更新 
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
				CreateEffectRenderParts(RenderParts, State, CanvasSize, Pivot);
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
	float Alpha = (State->localalpha == 1.f) ? State->alpha : State->localalpha;
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
			HideAlpha = 0.f;
		}
	}

	// TODO: 現バージョンでは未実装のパーツ種別は無視 
	if(    (State->partType == SsPartType::Armature)
		|| (State->partType == SsPartType::MoveNode)
		|| (State->partType == SsPartType::Constraint)
		|| (State->partType == SsPartType::Mask)
		|| (State->partType == SsPartType::Joint)
		|| (State->partType == SsPartType::BonePoint)
		)
	{
		return false;
	}

	OutRenderPart.PartIndex = State->index;
	OutRenderPart.Texture = bHideParts ? nullptr : State->cellValue.texture;	// 座標計算だけを行う非表示パーツはテクスチャをNULLにしておき、これを基準に描画をスキップする. 
	OutRenderPart.ColorBlendType = ((SsBlendType::Mix == State->partsColorValue.blendType) && (State->is_parts_color) && (State->partsColorValue.target == SsColorBlendTarget::Vertex))
			? SsBlendType::MixVertex
			: State->partsColorValue.blendType;
	OutRenderPart.AlphaBlendType = State->alphaBlendType;


	// RenderTargetに対する描画基準位置
	float OffX = (float)(CanvasSize.X /2) + (Pivot.X * CanvasSize.X);
	float OffY = (float)(CanvasSize.Y /2) - (Pivot.Y * CanvasSize.Y);

	FMatrix ViewMatrix(
		FVector(State->matrixLocal[ 0], State->matrixLocal[ 1], State->matrixLocal[ 2]),
		FVector(State->matrixLocal[ 4], State->matrixLocal[ 5], State->matrixLocal[ 6]),
		FVector(State->matrixLocal[ 8], State->matrixLocal[ 9], State->matrixLocal[10]),
		FVector(State->matrixLocal[12], State->matrixLocal[13], State->matrixLocal[14])
	);

	// 通常パーツ 
	if(State->partType != SsPartType::Mesh)
	{
		// 頂点座標
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
					const SsColorBlendValue& cbv = State->partsColorValue.colors[i];
					VertexColors[i].R = cbv.rgba.r;
					VertexColors[i].G = cbv.rgba.g;
					VertexColors[i].B = cbv.rgba.b;
					VertexColors[i].A = (uint8)(cbv.rgba.a * Alpha * HideAlpha);
					ColorBlendRate[i] = cbv.rate;
				}
			}
		}
		else
		{
			for(int32 i = 0; i < 4; ++i)
			{
				VertexColors[i] = FColor(255, 255, 255, (uint8)(255 * Alpha * HideAlpha));
				ColorBlendRate[i] = 1.f;
			}
		}

		for(int32 i = 0; i < 4; ++i)
		{
			OutRenderPart.Vertices[i].Position = FVector2D(Vertices2D[i].X/CanvasSize.X, Vertices2D[i].Y/CanvasSize.Y);
			OutRenderPart.Vertices[i].TexCoord = UVs[i];
			OutRenderPart.Vertices[i].Color = VertexColors[i];
			OutRenderPart.Vertices[i].ColorBlendRate = ColorBlendRate[i];
		}
	}
	// メッシュパーツ 
	else
	{
		check(nullptr != State->meshPart);
		int32 Idx = OutRenderPart.Mesh.Add(FSsRenderMesh());
		check(0 == Idx);
		FSsRenderMesh& RenderMesh = OutRenderPart.Mesh[Idx];

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
				FVector4 V = ViewMatrix.TransformPosition(FVector(
					State->meshPart->vertices[i*3 + 0],
					State->meshPart->vertices[i*3 + 1],
					State->meshPart->vertices[i*3 + 2]
				));
				RenderMesh.Vertices[i].Position.X = ( V.X + OffX) / CanvasSize.X;
				RenderMesh.Vertices[i].Position.Y = (-V.Y + OffY) / CanvasSize.Y;
			}
			RenderMesh.Vertices[i].TexCoord.X = State->meshPart->uvs[i*2 + 0];
			RenderMesh.Vertices[i].TexCoord.Y = State->meshPart->uvs[i*2 + 1];
		}

		RenderMesh.Indices.AddUninitialized(State->meshPart->tri_size * 3);
		for(int32 i = 0; i < (State->meshPart->tri_size * 3); ++i)
		{
			RenderMesh.Indices[i] = (uint32)State->meshPart->indices[i];
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
void FSsPlayer::CreateEffectRenderParts(TArray<FSsRenderPart>& OutRenderParts, const SsPartState* State, const FVector2D& CanvasSize, const FVector2D& Pivot)
{
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

					CreateEffectRenderPart(RenderParts, State, CanvasSize, Pivot, Emitter, (Effect->targetFrame - DrawData.stime), Emitter->_parent, &DrawData);
				}
			}
		}
		else
		{
			CreateEffectRenderPart(RenderParts, State, CanvasSize, Pivot, Emitter, Effect->targetFrame);
		}
	}
}

// エフェクト描画用パーツデータの作成（１パーツ分） 
void FSsPlayer::CreateEffectRenderPart(TArray<FSsRenderPart>& OutRenderParts, const SsPartState* State, const FVector2D& CanvasSize, const FVector2D& Pivot, SsEffectEmitter* Emitter, float Time, SsEffectEmitter* Parent, const particleDrawData* DrawData)
{
	// 参照：SsEffectRenderV2::particleDraw()

	if(nullptr == State){ return; }
	if(nullptr == Emitter){ return; }

	SsEffectRenderV2* Effect = State->refEffect;

	int32 ParticleNum = Emitter->getParticleIDMax();
	int32 Slide = (Parent == nullptr) ? 0 : DrawData->id;

	Emitter->updateEmitter(Time, Slide);

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


			FSsRenderPart RenderPart;
			RenderPart.PartIndex = State->index;
			RenderPart.Texture = Emitter->dispCell.texture;
			RenderPart.AlphaBlendType = SsRenderBlendTypeToBlendType(Emitter->refData->BlendType);
			RenderPart.ColorBlendType = SsBlendType::Effect;

			{
				float matrix[4 * 4];
				IdentityMatrix(matrix);

				float ParentAlpha = 1.f;
				if(Effect->parentState)
				{
					memcpy(matrix, Effect->parentState->matrixLocal, sizeof(float)*16);
					ParentAlpha = (Effect->parentState->localalpha == 1.f) ? Effect->parentState->alpha : Effect->parentState->localalpha;
				}

				TranslationMatrixM(matrix, lp.x * Effect->layoutScale.X, lp.y * Effect->layoutScale.Y, 0.f);
				RotationXYZMatrixM(matrix, 0.f, 0.f, FMath::DegreesToRadians(lp.rot) + lp.direc);
				ScaleMatrixM(matrix, lp.scale.X, lp.scale.Y, 1.f);

				fcolor.a *= ParentAlpha;

				if(Emitter->dispCell.cell && (0.f < fcolor.a))
				{
					FVector2D pivot = Emitter->dispCell.cell->Pivot;
					pivot.X *= Emitter->dispCell.cell->Size.X;
					pivot.Y *= Emitter->dispCell.cell->Size.Y;

					FVector2D dispscale = Emitter->dispCell.cell->Size;


					// RenderTargetに対する描画基準位置
					float OffX = (float)(CanvasSize.X / 2) + pivot.X + Pivot.X * CanvasSize.X;
					float OffY = (float)(CanvasSize.Y / 2) + pivot.Y - Pivot.Y * CanvasSize.Y;

					// 頂点座標
					FMatrix ViewMatrix(
						FVector(matrix[0], matrix[1], matrix[2]),
						FVector(matrix[4], matrix[5], matrix[6]),
						FVector(matrix[8], matrix[9], matrix[10]),
						FVector(matrix[12], matrix[13], matrix[14])
					);
					FVector Vertices[4] =
					{
						FVector(-(dispscale.X / 2.f),  (dispscale.Y / 2.f), 0.f),
						FVector((dispscale.X / 2.f),  (dispscale.Y / 2.f), 0.f),
						FVector(-(dispscale.X / 2.f), -(dispscale.Y / 2.f), 0.f),
						FVector((dispscale.X / 2.f), -(dispscale.Y / 2.f), 0.f),
					};

					for (int32 i = 0; i < 4; ++i)
					{
						FVector4 V = ViewMatrix.TransformPosition(Vertices[i]);
						RenderPart.Vertices[i].Position.X = ( V.X + OffX) / CanvasSize.X;
						RenderPart.Vertices[i].Position.Y = (-V.Y + OffY) / CanvasSize.Y;

						RenderPart.Vertices[i].TexCoord = Emitter->dispCell.uvs[i];
						RenderPart.Vertices[i].Color = FColor(lp.color.R, lp.color.G, lp.color.B, (uint8)(lp.color.A * ParentAlpha));
						RenderPart.Vertices[i].ColorBlendRate = (Emitter->particle.useColor || Emitter->particle.useTransColor) ? 1.f : 0.f;
					}

					OutRenderParts.Add(RenderPart);
				}
			}
		}
	}
}

// 再生中にアニメーションのCanvasSizeの取得 
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
	Decoder->setPlayFrame((float)(Decoder->getAnimeStartFrame() + StartFrame));

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
		Decoder->setPlayFrame(Decoder->getAnimeStartFrame() + Frame);
	}
}

// 現在フレーム取得
float FSsPlayer::GetPlayFrame() const
{
	if(nullptr != Decoder)
	{
		return Decoder->nowPlatTime - Decoder->getAnimeStartFrame();
	}
	return 0.f;
}

// 最終フレーム取得
float FSsPlayer::GetAnimeEndFrame() const
{
	if(nullptr != Decoder)
	{
		return Decoder->getAnimeEndFrame() - Decoder->getAnimeStartFrame() + 1;
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
	if((nullptr == Decoder) || (PartIndex < 0) || (Decoder->sortList.Num() <= PartIndex))
	{
		return false;
	}

	SsPartState* State = &(Decoder->partState[PartIndex]);
	FVector2D Pivot = GetAnimPivot();
	FVector2D CanvasSize = GetAnimCanvasSize();
	OutPosition = FVector2D(
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
