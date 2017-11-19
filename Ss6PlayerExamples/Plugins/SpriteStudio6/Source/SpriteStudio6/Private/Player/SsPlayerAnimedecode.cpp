#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerAnimedecode.h"

#include "Ss6Project.h"
#include "SsCellMap.h"
#include "SsAnimePack.h"
#include "SsAttribute.h"
#include "SsInterpolation.h"
#include "SsPlayerMatrix.h"
#include "SsPlayerPartState.h"
#include "SsPlayerCellmap.h"
#include "SsPlayerEffect2.h"


#define USE_TRIANGLE_FIN (0)


namespace
{
	//乱数シードに利用するユニークIDを作成します。 
	uint32 SeedMakeID = 123456;

	//エフェクトに与えるシードを取得する関数 
	//こちらを移植してください。 
	uint32 GetRandomSeed()
	{
		SeedMakeID++;	//ユニークIDを更新します。 
						//時間＋ユニークIDにする事で毎回シードが変わるようにします。 
		uint32 rc = (uint32)(FDateTime::Now().ToUnixTimestamp() + SeedMakeID);

		return(rc);
	}
}


FSsAnimeDecoder::FSsAnimeDecoder()
	: CurCellMapManager(0)
	, PartState(0)
	, SeedOffset(0)
	, NowPlatTime(0)
	, FrameDelta(0)
	, CurAnimeEndFrame(0)
	, CurAnimeFPS(0)
	, CurAnimeCanvasSize(0, 0)
	, CurAnimePivot(0, 0)
	, CurAnimation(NULL)
	, bCalcHideParts(false)
{
}

FSsAnimeDecoder::~FSsAnimeDecoder()
{
	if(PartState)
	{
		delete [] PartState;
	}
}



void FSsAnimeDecoder::SetAnimation(FSsModel* model, FSsAnimation* anime, FSsCellMapList* cellmap, USs6Project* sspj)
{
	//セルマップリストを取得
	CurCellMapManager = cellmap;
	CurAnimation = anime;

	//アニメの基準枠を取得
	CurAnimeCanvasSize = anime->Settings.CanvasSize;
	CurAnimePivot = anime->Settings.Pivot;

	//partStateをパーツ分作成する
	PartAnimeDic.Empty();

	//パーツの数
	size_t panum = anime->PartAnimes.Num();
	for ( size_t i = 0 ; i < panum ; i++ )
	{
		FSsPartAnime* panime = &anime->PartAnimes[i];
		if(!PartAnimeDic.Contains(panime->PartName))
		{
			PartAnimeDic.Add(panime->PartName);
		}
		PartAnimeDic[panime->PartName] = panime;
	}
	//パーツとパーツアニメを関連付ける
	size_t partNum = model->PartList.Num();

	if ( PartState ) delete [] PartState;
	PartState = new FSsPartState[partNum]();
	SortList.Empty();
	PartAnime.Empty();

	for ( size_t i = 0 ; i < partNum ; i++ ) 
	{
		FSsPart* p = &model->PartList[i];

		if(!PartAnimeDic.Contains(p->PartName))
		{
			PartAnimeDic.Add(p->PartName);	// std::map互換のため、存在しないキーに対して要素NULLで追加する.
		}
		FSsPartAndAnime _temp;
		_temp.Key = p;
		_temp.Value = PartAnimeDic[p->PartName];
		PartAnime.Add( _temp );

		//親子関係の設定
		if ( p->ParentIndex != -1 )
		{
			PartState[i].Parent = &PartState[p->ParentIndex];
		}else{
			PartState[i].Parent = 0;
		}

		//継承率の設定
		PartState[i].InheritRates = p->InheritRates;
		PartState[i].Index = i;


		if (sspj)
		{
			//インスタンスパーツの場合の初期設定
			if ( p->Type == SsPartType::Instance )
			{
				//参照アニメーションを取得
				int32 AnimPackIndex, AnimationIndex;
				if(sspj->FindAnimationIndex(p->RefAnimePack, p->RefAnime, AnimPackIndex, AnimationIndex))
				{
					FSsAnimePack* refpack = &sspj->AnimeList[AnimPackIndex];
					FSsAnimation* refanime = &refpack->AnimeList[AnimationIndex];

					FSsCellMapList* __cellmap = new FSsCellMapList();
					__cellmap->Set( sspj , refpack );

					// インスタンスパーツ再生用のDecoderを生成
					FSsAnimeDecoder* animedecoder = new FSsAnimeDecoder();
					animedecoder->SetAnimation( &refpack->Model , refanime , __cellmap , sspj );

					// 描画先は親のCanvasなので、基準枠の設定はそちらに合わせる
					animedecoder->CurAnimeCanvasSize = CurAnimeCanvasSize;
					animedecoder->CurAnimePivot = CurAnimePivot;

					PartState[i].RefAnime = animedecoder;

					//親子関係を付ける
					animedecoder->PartState[0].Parent = &PartState[i];
				}
			}

			//エフェクトデータの初期設定
			if ( p->Type == SsPartType::Effect )
			{
				int32 EffectIndex = sspj->FindEffectIndex( p->RefEffectName );
				if(0 <= EffectIndex)
				{
					FSsEffectFile* f = &(sspj->EffectList[EffectIndex]);
					FSsEffectRenderV2* er = new FSsEffectRenderV2();
					er->SetParentAnimeState( &PartState[i] );
					er->SetCellmapManager( this->CurCellMapManager );
					er->SetEffectData( &f->EffectData );
					er->SetSeed(GetRandomSeed());
					er->Reload();
					er->Stop();
					er->SetLoop(false);

					PartState[i].RefEffect = er;
				}
			}
		}

		SortList.Add( &PartState[i] );

	}

	//アニメの最大フレーム数を取得
	CurAnimeEndFrame = anime->Settings.FrameCount;
	CurAnimeFPS = anime->Settings.Fps;
}

void FSsAnimeDecoder::SetPlayFrame(float time)
{
	NowPlatTime = time;
	FrameDelta = 0.f;
}

// パーツ名からインデックスを取得
int FSsAnimeDecoder::GetPartIndexFromName(FName PartName) const
{
	for(int i = 0; i < PartAnime.Num(); ++i)
	{
		if(PartAnime[i].Key->PartName == PartName)
		{
			return i;
		}
	}
	return -1;
}

// パーツのTransformを取得 
bool FSsAnimeDecoder::GetPartTransform(int PartIndex, FVector2D& OutPosition, float& OutRotate, FVector2D& OutScale) const
{
	if((PartIndex < 0) ||(SortList.Num() <= PartIndex))
	{
		return false;
	}

	FSsPartState* State = &(PartState[PartIndex]);
	OutPosition = FVector2D(
		State->Matrix[12] + ((float)(CurAnimeCanvasSize.X /2) + (CurAnimePivot.X * CurAnimeCanvasSize.X)),
		State->Matrix[13] - ((float)(CurAnimeCanvasSize.Y /2) - (CurAnimePivot.Y * CurAnimeCanvasSize.Y))
		);
	OutPosition.X = OutPosition.X / CurAnimeCanvasSize.X;
	OutPosition.Y = OutPosition.Y / CurAnimeCanvasSize.Y;

	OutRotate = State->Rotation.Z;
	for(FSsPartState* ParentState = State->Parent; NULL != ParentState; ParentState = ParentState->Parent)
	{
		OutRotate += ParentState->Rotation.Z;
	}
	while(OutRotate <   0.f){ OutRotate += 360.f; }
	while(OutRotate > 360.f){ OutRotate -= 360.f; }

	OutScale = State->Scale;
	return true;
}


void FSsAnimeDecoder::ReloadEffects()
{
#if 0
	for(int i = 0; i < PartAnime.Num(); ++i)
	{
		if(PartState[i].RefEffect)
		{
			PartState[i].RefEffect->Stop();
			PartState[i].RefEffect->SetSeed(GetRandomSeed());
			PartState[i].RefEffect->Reload();
		}
	}
#endif
}

FName FSsAnimeDecoder::GetPartColorLabel(int32 PartIndex)
{
	if((PartIndex < 0) || (PartAnime.Num() <= PartIndex))
	{
		return FName();
	}
	return PartAnime[PartIndex].Key->ColorLabel;
}


//頂点変形アニメデータの取得
void FSsAnimeDecoder::SsInterpolationValue(int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , FSsVertexAnime& v)
{
	//todo ロード時に FSsVertexAnimeを作成してしまうようにする
	FSsVertexAnime	lv;
	FSsVertexAnime	rv;

	if ( rightkey == 0 ) //右側が０出会った場合
	{
		GetFSsVertexAnime( leftkey , v );
		return ;
	}

	GetFSsVertexAnime(leftkey,lv);
	GetFSsVertexAnime(rightkey,rv);

	int range = rightkey->Time - leftkey->Time;
	float now = (float)(time - leftkey->Time) / range;

	FSsCurve curve;
	curve = leftkey->Curve;
	if (leftkey->IpType == SsInterpolationType::Bezier)
	{
		// ベジェのみキーの開始・終了時間が必要
		curve.StartKeyTime = leftkey->Time;
		curve.EndKeyTime = rightkey->Time;
	}

	float rate = SsInterpolate(leftkey->IpType, now, 0.f, 1.f, &curve);
	for ( int i = 0 ; i < 4 ; i++ )
	{
		v.Offsets[i].X = SsInterpolate(SsInterpolationType::Linear, rate, lv.Offsets[i].X, rv.Offsets[i].X, NULL);
		v.Offsets[i].Y = SsInterpolate(SsInterpolationType::Linear, rate, lv.Offsets[i].Y, rv.Offsets[i].Y, NULL);
	}

}


static float clamp(float v, float vmin, float vmax)
{
	float ret = v;
	if(v < vmin){ ret = vmin; }
	if(v > vmax){ ret = vmax; }
	return ret;
}

void	FSsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , FSsColorAnime& v )
{
	if ( rightkey == 0 )
	{
		GetSsColorValue( leftkey , v );
		return ;
	}
	
	FSsColorAnime leftv;
	FSsColorAnime rightv;

	GetSsColorValue( leftkey , leftv );
	GetSsColorValue( rightkey , rightv );


	FSsCurve curve;
	curve = leftkey->Curve;
	if (leftkey->IpType == SsInterpolationType::Bezier)
	{
		// ベジェのみキーの開始・終了時間が必要
		curve.StartKeyTime = leftkey->Time;
		curve.EndKeyTime = rightkey->Time;
	}

	int range = rightkey->Time - leftkey->Time;
	float now = (float)(time - leftkey->Time) / range;

	v.Color.Rgba.A = 0;
	v.Color.Rgba.R = 0;
	v.Color.Rgba.G = 0;
	v.Color.Rgba.B = 0;

	if(leftv.Target == SsColorBlendTarget::Vertex)
	{
		if(rightv.Target == SsColorBlendTarget::Vertex)
		{
			// 両方とも４頂点カラー
			for(int i = 0; i < 4; ++i)
			{
				v.Colors[i].Rate = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rate, rightv.Colors[i].Rate, &curve), 0.f, 1.f);
				v.Colors[i].Rgba.A = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rgba.A, rightv.Colors[i].Rgba.A, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.R = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rgba.R, rightv.Colors[i].Rgba.R, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.G = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rgba.G, rightv.Colors[i].Rgba.G, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.B = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rgba.B, rightv.Colors[i].Rgba.B, &curve), 0.f, 255.f);
				v.Target = SsColorBlendTarget::Vertex;
			}
		}
		else
		{
			// 左は４頂点、右は単色
			for(int i = 0; i < 4; ++i)
			{
				v.Colors[i].Rate = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rate, rightv.Color.Rate, &curve), 0.f, 1.f);
				v.Colors[i].Rgba.A = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rgba.A, rightv.Color.Rgba.A, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.R = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rgba.R, rightv.Color.Rgba.R, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.G = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rgba.G, rightv.Color.Rgba.G, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.B = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Colors[i].Rgba.B, rightv.Color.Rgba.B, &curve), 0.f, 255.f);
				v.Target = SsColorBlendTarget::Vertex;
			}
		}
	}
	else
	{
		if(rightv.Target == SsColorBlendTarget::Vertex)
		{
			// 左は単色、右は４頂点カラー
			for(int i = 0; i < 4; ++i)
			{
				v.Colors[i].Rate = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rate, rightv.Colors[i].Rate, &curve), 0.f, 1.f);
				v.Colors[i].Rgba.A = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rgba.A, rightv.Colors[i].Rgba.A, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.R = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rgba.R, rightv.Colors[i].Rgba.R, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.G = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rgba.G, rightv.Colors[i].Rgba.G, &curve), 0.f, 255.f);
				v.Colors[i].Rgba.B = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rgba.B, rightv.Colors[i].Rgba.B, &curve), 0.f, 255.f);
				v.Target = SsColorBlendTarget::Vertex;
			}
		}
		else
		{
			// 両方とも単色
			v.Color.Rate = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rate, rightv.Color.Rate, &curve), 0.f, 1.f);
			v.Color.Rgba.A = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rgba.A, rightv.Color.Rgba.A, &curve), 0.f, 255.f);
			v.Color.Rgba.R = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rgba.R, rightv.Color.Rgba.R, &curve), 0.f, 255.f);
			v.Color.Rgba.G = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rgba.G, rightv.Color.Rgba.G, &curve), 0.f, 255.f);
			v.Color.Rgba.B = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.Color.Rgba.B, rightv.Color.Rgba.B, &curve), 0.f, 255.f);
			v.Target = SsColorBlendTarget::Whole;
		}
	}
}

void	FSsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , FSsCellValue& v )
{
	FSsRefCell cell;
	GetFSsRefCell( leftkey , cell );

	FSsCelMapLinker* l = this->CurCellMapManager->GetCellMapLink( cell.Mapid );
	v.Cell = l->FindCell( cell.Name );

	v.FilterMode = l->CellMap->FilterMode;
	v.WrapMode = l->CellMap->WrapMode;

	if ( l->Tex )
	{
		v.Texture = l->Tex;
	}
	else
	{
		v.Texture = 0;
	}

	CalcUvs( &v );

}


void	FSsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , bool& v )
{
	if ( rightkey == 0 )
	{
		v = leftkey->Value.get<bool>();
		return ;
	}
	
	float v1 = (float)leftkey->Value.get<bool>();
	float v2 = (float)rightkey->Value.get<bool>();

	int range = rightkey->Time - leftkey->Time;
	float now = (float)(time - leftkey->Time) / range;

	if (leftkey->IpType == SsInterpolationType::Bezier)
	{
		// ベジェのみキーの開始・終了時間が必要
		FSsCurve curve;
		curve = leftkey->Curve;
		curve.StartKeyTime = leftkey->Time;
		curve.EndKeyTime = (float)rightkey->Time;
		v = 0.f != SsInterpolate( leftkey->IpType , now , v1 , v2 , &curve );
	}
	else{
		v = 0.f != SsInterpolate( leftkey->IpType , now , v1 , v2 , &leftkey->Curve );
	}
}

void FSsAnimeDecoder::SsInterpolationValue(int time, const FSsKeyframe* leftkey, const FSsKeyframe* rightkey, FSsInstanceAttr& v)
{
	//補間は行わないので、常に左のキーを出力する
	GetSsInstparamAnime(leftkey, v);
}

void FSsAnimeDecoder::SsInterpolationValue(int time, const FSsKeyframe* leftkey, const FSsKeyframe* rightkey, FSsEffectAttr& v)
{
	//補間は行わないので、常に左のキーを出力する
	GetSsEffectParamAnime(leftkey, v);
}

//float , int 基本型はこれで値の補間を行う
template<typename mytype>
void	FSsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , mytype& v )
{
	if ( rightkey == 0 )
	{
		v = leftkey->Value.get<mytype>();
		return ;
	}
	
	float v1 = (float)leftkey->Value.get<mytype>();
	float v2 = (float)rightkey->Value.get<mytype>();

	int range = rightkey->Time - leftkey->Time;
	float now = (float)(time - leftkey->Time) / range;

	if (leftkey->IpType == SsInterpolationType::Bezier)
	{
		// ベジェのみキーの開始・終了時間が必要
		FSsCurve curve;
		curve = leftkey->Curve;
		curve.StartKeyTime = leftkey->Time;
		curve.EndKeyTime = (float)rightkey->Time;
		v = SsInterpolate( leftkey->IpType , now , v1 , v2 , &curve );
	}
	else{
		v = SsInterpolate( leftkey->IpType , now , v1 , v2 , &leftkey->Curve );
	}
}


template<typename mytype> int	FSsAnimeDecoder::SsGetKeyValue( int time , FSsAttribute* attr , mytype&  value )
{
	int	useTime = 0;

	if ( attr->isEmpty() )
	{
		//デフォルト値を入れる まだ未実装
		return useTime;
	}

	const FSsKeyframe* lkey = attr->FindLeftKey( time );

	//無い場合は、最初のキーを採用する
	if ( lkey == 0 )
	{
		lkey =  attr->FirstKey();
		SsInterpolationValue( time , lkey , 0 , value );

		useTime = lkey->Time;
		return useTime;

	}else if ( lkey->Time == time )
	{
		SsInterpolationValue( time , lkey , 0 , value );
		useTime = time;
		return useTime;
	}else{
		//補間計算をする
		const FSsKeyframe* rkey = attr->FindRightKey( time );
		if (rkey == NULL)
		{
			// 次のキーが無いので補間できない。よって始点キーの値をそのまま返す
			SsInterpolationValue( time , lkey , 0 , value );
			useTime = lkey->Time;
			return useTime;
		}else
		if (lkey->IpType == SsInterpolationType::None)
		{
			// 補間なし指定なので始点キーの値…
			SsInterpolationValue( time , lkey , 0 , value );
			useTime = lkey->Time;
			return useTime ;
		}else
		{
			SsInterpolationValue( time , lkey ,rkey , value );
			useTime = time;
		}
	}
		
	return useTime ;
}


//中間点を求める
static void	CoordinateGetDiagonalIntersection( FVector2D& out , const FVector2D& LU , const FVector2D& RU , const FVector2D& LD , const FVector2D& RD )
{
	out = FVector2D(0.f,0.f);

	/* <<< 係数を求める >>> */
	float c1 = (LD.Y - RU.Y) * (LD.X - LU.X) - (LD.X - RU.X) * (LD.Y - LU.Y);
	float c2 = (RD.X - LU.X) * (LD.Y - LU.Y) - (RD.Y - LU.Y) * (LD.X - LU.X);
	float c3 = (RD.X - LU.X) * (LD.Y - RU.Y) - (RD.Y - LU.Y) * (LD.X - RU.X);


	if ( c3 <= 0 && c3 >=0) return;

	float ca = c1 / c3;
	float cb = c2 / c3;

	/* <<< 交差判定 >>> */
	if(((0.0f <= ca) && (1.0f >= ca)) && ((0.0f <= cb) && (1.0f >= cb)))
	{	/* 交差している */
		out.X = LU.X + ca * (RD.X - LU.X);
		out.Y = LU.Y + ca * (RD.Y - LU.Y);
	}
}

static FVector2D GetLocalScale( float matrix[16] )
{
	float sx = FVector2D::Distance( FVector2D( matrix[0] , matrix[1] ) , FVector2D( 0 , 0 ) );
	float sy = FVector2D::Distance( FVector2D( matrix[4 * 1 + 0] , matrix[4 * 1 + 1] ) , FVector2D( 0 , 0 ) );

	return FVector2D( sx , sy );
}


///現在の時間からパーツのアトリビュートの補間値を計算する
void	FSsAnimeDecoder::UpdateState( int nowTime , FSsPart* part , FSsPartAnime* anime , FSsPartState* state )
{
	//ステートの初期値を設定
	state->Init();
	state->InheritRates = part->InheritRates;
	if ( anime == 0 ){
		IdentityMatrix( state->Matrix );
		state->Hide = true;
		return ;
	}

	// 親の継承設定を引用する設定の場合、ここで参照先を親のものに変えておく。
	if (part->InheritType == SsInheritType::Parent)
	{
		if ( state->Parent )
		{
			state->InheritRates = state->Parent->InheritRates;
		}
	}

	bool	size_x_key_find = false;
	bool	size_y_key_find = false;

	state->IsVertexTransform = false;
	state->IsColorBlend = false;
	state->AlphaBlendType = part->AlphaBlendType;


	bool hidekey_find = false;

	if ( 0 < anime->Attributes.Num() )
	{
		for(int i = 0; i < anime->Attributes.Num(); ++i)
		{
			FSsAttribute* attr = &(anime->Attributes[i]);
			switch( attr->Tag )
			{
				case SsAttributeKind::Invalid:	///< 無効値。旧データからの変換時など
					break;
				case SsAttributeKind::Cell:		///< 参照セル
					{
						SsGetKeyValue( nowTime , attr , state->CellValue );
						state->NoCells = false;
					}
					break;
				case SsAttributeKind::Posx:		///< 位置.X
					SsGetKeyValue( nowTime , attr , state->Position.X );
					break;
				case SsAttributeKind::Posy:		///< 位置.Y
					SsGetKeyValue( nowTime , attr , state->Position.Y );
					break;
				case SsAttributeKind::Posz:		///< 位置.Z
					SsGetKeyValue( nowTime , attr , state->Position.Z );
					break;
				case SsAttributeKind::Rotx:		///< 回転.X
					SsGetKeyValue( nowTime , attr , state->Rotation.X );
					break;
				case SsAttributeKind::Roty:		///< 回転.Y
					SsGetKeyValue( nowTime , attr , state->Rotation.Y );
					break;
				case SsAttributeKind::Rotz:		///< 回転.Z
					SsGetKeyValue( nowTime , attr , state->Rotation.Z );
					break;
				case SsAttributeKind::Sclx:		///< スケール.X
					SsGetKeyValue( nowTime , attr , state->Scale.X );
					break;
				case SsAttributeKind::Scly:		///< スケール.Y
					SsGetKeyValue( nowTime , attr , state->Scale.Y );
					break;
				case SsAttributeKind::Alpha:	///< 不透明度
					SsGetKeyValue( nowTime , attr , state->Alpha );
					break;
				case SsAttributeKind::Prio:		///< 優先度
					SsGetKeyValue( nowTime , attr , state->Prio );
					break;
				case SsAttributeKind::Fliph:	///< 左右反転(セルの原点を軸にする)
					SsGetKeyValue( nowTime , attr , state->HFlip );
					break;
				case SsAttributeKind::Flipv:	///< 上下反転(セルの原点を軸にする)
					SsGetKeyValue( nowTime , attr , state->VFlip );
					break;
				case SsAttributeKind::Hide:		///< 非表示
					{
						int useTime = SsGetKeyValue( nowTime , attr , state->Hide );
						// 非表示キーがないか、先頭の非表示キーより手前の場合は常に非表示にする。
						if ( useTime > nowTime )
						{
							state->Hide = true;
						}
						// 非表示キーがあり、かつ最初のキーフレームを取得した
						else
						{
							hidekey_find = true;
						}
					}
					break;
				case SsAttributeKind::Color:	///< カラーブレンド
					SsGetKeyValue( nowTime , attr , state->ColorValue );
					state->IsColorBlend = true;
					break;
				case SsAttributeKind::Vertex:	///< 頂点変形
					SsGetKeyValue( nowTime , attr , state->VertexValue );
					state->IsVertexTransform = true;
					break;
				case SsAttributeKind::Pivotx:	///< 原点オフセット.X
					SsGetKeyValue( nowTime , attr , state->PivotOffset.X );
					break;
				case SsAttributeKind::Pivoty:	///< 原点オフセット.Y
					SsGetKeyValue( nowTime , attr , state->PivotOffset.Y );
					break;
				case SsAttributeKind::Anchorx:	///< アンカーポイント.X
					SsGetKeyValue( nowTime , attr , state->Anchor.X );
					break;
				case SsAttributeKind::Anchory:	///< アンカーポイント.Y
					SsGetKeyValue( nowTime , attr , state->Anchor.Y );
					break;
				case SsAttributeKind::Sizex:	///< 表示サイズ.X
					SsGetKeyValue( nowTime , attr , state->Size.X );
					size_x_key_find = true;
					break;
				case SsAttributeKind::Sizey:	///< 表示サイズ.Y
					SsGetKeyValue( nowTime , attr , state->Size.Y );
					size_y_key_find = true;
					break;
				case SsAttributeKind::Imgfliph:	///< イメージ左右反転(常にイメージの中央を原点とする)
					SsGetKeyValue( nowTime , attr , state->ImageFlipH );
					break;
				case SsAttributeKind::Imgflipv:	///< イメージ上下反転(常にイメージの中央を原点とする)
					SsGetKeyValue( nowTime , attr , state->ImageFlipV );
					break;
				case SsAttributeKind::Uvtx:		///< UVアニメ.移動.X
					SsGetKeyValue( nowTime , attr , state->UvTranslate.X );
					break;
				case SsAttributeKind::Uvty:		///< UVアニメ.移動.Y
					SsGetKeyValue( nowTime , attr , state->UvTranslate.Y );
					break;
				case SsAttributeKind::Uvrz:		///< UVアニメ.回転
					SsGetKeyValue( nowTime , attr , state->UvRotation );
					break;
				case SsAttributeKind::Uvsx:		///< UVアニメ.スケール.X
					SsGetKeyValue( nowTime , attr , state->UvScale.X );
					break;
				case SsAttributeKind::Uvsy:		///< UVアニメ.スケール.Y
					SsGetKeyValue( nowTime , attr , state->UvScale.Y );
					break;
				case SsAttributeKind::Boundr:	///< 当たり判定用の半径
					SsGetKeyValue(nowTime, attr, state->BoundingRadius);
					break;
				case SsAttributeKind::User:		///< Ver.4 互換ユーザーデータ
					break;
				case SsAttributeKind::Instance:	///インスタンスパラメータ
					{
						int t = SsGetKeyValue(nowTime, attr, state->InstanceValue);
						//先頭にキーが無い場合
						if(t > nowTime)
						{
							FSsInstanceAttr d;
							state->InstanceValue = d;
						}
					} break;
				case SsAttributeKind::Effect:
					{
						int t = SsGetKeyValue(nowTime, attr, state->EffectValue);

						//先頭にキーが無い場合
						if(t > nowTime)
						{
							FSsEffectAttr d;
							state->EffectValue = d;
						}
						else
						{
							state->EffectTime = t;
							if(!state->EffectValue.AttrInitialized)
							{
								state->EffectValue.AttrInitialized = true;
								state->EffectTimeTotal = state->EffectValue.StartTime;
								state->EffectTime = t;
							}
						}
					} break;
			}
		}
	}



	// カラー値だけアニメが無いと設定されないので初期値を入れておく。
	// alpha はupdateで初期化されるのでOK
	// 当たり判定パーツ用のカラー。赤の半透明にする
	static const float sColorsForBoundsParts[] = {0.5f, 0.f, 0.f, 1.f};
	for (int i = 0; i < (4*4) ; ++i)
	{
		if (state->NoCells)
			state->Colors[i] = sColorsForBoundsParts[i & 3];
		else
			state->Colors[i] = 1.f;
	}

	// 継承
	if (state->Parent)
	{
		// α
		if (state->Inherits_(SsAttributeKind::Alpha))
		{
			state->Alpha *= state->Parent->Alpha;
		}

		// フリップの継承。継承ONだと親に対しての反転になる…ヤヤコシス
		if (state->Inherits_(SsAttributeKind::Fliph))
		{
			state->HFlip = state->Parent->HFlip ^ state->HFlip;
		}
		if (state->Inherits_(SsAttributeKind::Flipv))
		{
			state->VFlip = state->Parent->VFlip ^ state->VFlip;
		}

		// 引き継ぐ場合は親の値をそのまま引き継ぐ
		if (state->Inherits_(SsAttributeKind::Hide))
		{
			state->Hide = state->Parent->Hide;
		}
	}

	// 非表示キーがないか、先頭の非表示キーより手前の場合は常に非表示にする。(継承関係なし)
	if (!hidekey_find)
	{
		state->Hide = true;
	}

	// 頂点の設定
	if ( part->Type == SsPartType::Normal )
	{
		FSsCell * cell = state->CellValue.Cell;
		if (cell && anime)
		{
			//サイズアトリビュートが指定されていない場合、セルのサイズを設定する
			if ( !size_x_key_find ) state->Size.X = cell->Size.X;
			if ( !size_y_key_find ) state->Size.Y = cell->Size.Y;
		}
		UpdateVertices(part , anime , state);
	}
}

void	FSsAnimeDecoder::UpdateMatrix(FSsPart* part , FSsPartAnime* anime , FSsPartState* state)
{

	IdentityMatrix( state->Matrix );

	if (state->Parent)
	{
		memcpy( state->Matrix , state->Parent->Matrix , sizeof( float ) * 16 );
	}

	// アンカー
	if ( state->Parent )
	{
		const FVector2D& parentSize = state->Parent->Size;
		state->Position.X = state->Position.X + state->Parent->Size.X * state->Anchor.X;
		state->Position.Y = state->Position.Y + state->Parent->Size.Y * state->Anchor.Y;
	}

	TranslationMatrixM( state->Matrix , state->Position.X, state->Position.Y, state->Position.Z );//
	RotationXYZMatrixM( state->Matrix , FMath::DegreesToRadians(state->Rotation.X) , FMath::DegreesToRadians(state->Rotation.Y) , FMath::DegreesToRadians( state->Rotation.Z) );
	ScaleMatrixM(  state->Matrix , state->Scale.X, state->Scale.Y, 1.0f );
}


void	FSsAnimeDecoder::UpdateVertices(FSsPart* part , FSsPartAnime* anime , FSsPartState* state)
{

	FSsCell * cell = state->CellValue.Cell;

	FVector2D pivot;
	if (cell)
	{
		// セルに設定された原点オフセットを適用する
		// ※セルの原点は中央が0,0で＋が右上方向になっている
		float cpx = cell->Pivot.X + 0.5f;
		if (state->HFlip) cpx = 1 - cpx;	// 水平フリップによって原点を入れ替える
		pivot.X = cpx * state->Size.X;
		// 上が＋で入っているのでここで反転する。
		float cpy = -cell->Pivot.Y + 0.5f;
		if (state->VFlip) cpy = 1 - cpy;	// 垂直フリップによって原点を入れ替える
		pivot.Y = cpy * state->Size.Y;
	}
	else
	{
		// セルが無いパーツでも原点が中央に来るようにする。
		pivot.X = 0.5f * state->Size.X;
		pivot.Y = 0.5f * state->Size.Y;
	}

	// 次に原点オフセットアニメの値を足す
	pivot.X += state->PivotOffset.X * state->Size.X;
	pivot.Y += -state->PivotOffset.Y * state->Size.Y;

	float sx = -pivot.X;
	float ex = sx + state->Size.X;
	float sy = +pivot.Y;
	float ey = sy - state->Size.Y;

	// Z順
	/*
		これは実は上下ひっくり返って裏面になっているためUV値も上下反転させている。
		左上が最初に来る並びの方が頂点カラー・頂点変形のデータと同じで判りやすいのでこれでいく。
	*/
	float vtxPosX[4] = {sx, ex, sx, ex};
	float vtxPosY[4] = {sy, sy, ey, ey};

	FVector2D * vtxOfs = state->VertexValue.Offsets;

	//きれいな頂点変形に対応
#if USE_TRIANGLE_FIN

	if ( state->is_color_blend || state->is_vertex_transform )
	{

		FVector2D	vertexCoordinateLU = FVector2D( sx + (float)vtxOfs[0].x , sy + (float)vtxOfs[0].y );// : 左上頂点座標（ピクセル座標系）
		FVector2D	vertexCoordinateRU = FVector2D( ex + (float)vtxOfs[1].x , sy + (float)vtxOfs[1].y );// : 右上頂点座標（ピクセル座標系）
		FVector2D	vertexCoordinateLD = FVector2D( sx + (float)vtxOfs[2].x , ey + (float)vtxOfs[2].y );// : 左下頂点座標（ピクセル座標系）
		FVector2D	vertexCoordinateRD = FVector2D( ex + (float)vtxOfs[3].x , ey + (float)vtxOfs[3].y );// : 右下頂点座標（ピクセル座標系）

		FVector2D CoordinateLURU = (vertexCoordinateLU + vertexCoordinateRU) * 0.5f;
		FVector2D CoordinateLULD = (vertexCoordinateLU + vertexCoordinateLD) * 0.5f;
		FVector2D CoordinateLDRD = (vertexCoordinateLD + vertexCoordinateRD) * 0.5f;
		FVector2D CoordinateRURD = (vertexCoordinateRU + vertexCoordinateRD) * 0.5f;

		FVector2D center;
		CoordinateGetDiagonalIntersection( center , CoordinateLURU, CoordinateRURD, CoordinateLULD, CoordinateLDRD );

		FVector2D*	coodinatetable[] = { &vertexCoordinateLU , &vertexCoordinateRU , &vertexCoordinateLD , &vertexCoordinateRD , &center };


		for (int i = 0; i < 5; ++i)
		{
			state->vertices[ i * 3 ] = coodinatetable[i]->x;
			state->vertices[ i * 3 + 1 ] = coodinatetable[i]->y;
			state->vertices[ i * 3 + 2]	= 0;
		}
	}else{
		for (int i = 0; i < 4; ++i)
		{
			state->vertices[i * 3]		= vtxPosX[i];
			state->vertices[i * 3 + 1]	= vtxPosY[i];
			state->vertices[i * 3 + 2]	= 0;

			++vtxOfs;
		}

	}

#else
	//4点変形の場合
	// 頂点変形のデータはＺ字順に格納されている。
	//FVector2D * vtxOfs = vertexValue.offsets;
	for (int i = 0; i < 4; ++i)
	{
		state->Vertices[i * 3]		= vtxPosX[i] + (float)vtxOfs->X;
		state->Vertices[i * 3 + 1]	= vtxPosY[i] + (float)vtxOfs->Y;
		state->Vertices[i * 3 + 2]	= 0;

		++vtxOfs;
	}
#endif



}

void	FSsAnimeDecoder::UpdateInstance( int nowTime , FSsPart* part , FSsPartAnime* partanime , FSsPartState* state )
{
	if ( state->RefAnime == 0 ){ return; }
	
	FSsAnimation* anime = state->RefAnime->CurAnimation;
	const FSsInstanceAttr& instanceValue = state->InstanceValue;

	//プレイヤー等では再生開始時にいったん計算してしまって値にしてしまった方がいい。
	//エディター側のロジックなのでそのまま検索する
	//インスタンスアニメ内のスタート位置
    int	startframe = CalcAnimeLabel2Frame(instanceValue.StartLabel, instanceValue.StartOffset, anime);
    int	endframe = CalcAnimeLabel2Frame(instanceValue.EndLabel, instanceValue.EndOffset, anime);

    state->InstanceValue.StartFrame = startframe;		//ラベル位置とオフセット位置を加えた実際のフレーム数
    state->InstanceValue.EndFrame = endframe;			//ラベル位置とオフセット位置を加えた実際のフレーム数

    //タイムライン上の時間 （絶対時間）
	int time = nowTime;

	//独立動作の場合
	if ( instanceValue.Independent )
	{
		//float delta = animeState->frame - parentBackTime;
		float delta = this->FrameDelta;

		state->InstanceValue.LiveFrame+= ( delta * instanceValue.Speed );
		//parentBackTime = animeState->frame;
		time = (int)instanceValue.LiveFrame;

	}

    //このインスタンスが配置されたキーフレーム（絶対時間）
    int	selfTopKeyframe = instanceValue.CurKeyframe;


    int reftime = (time - selfTopKeyframe) * instanceValue.Speed;
    if ( reftime < 0 ) return ; //そもそも生存時間に存在していない
	if (selfTopKeyframe > time ) return ;

    int inst_scale = (endframe - startframe) + 1; //インスタンスの尺


	//尺が０もしくはマイナス（あり得ない
	if ( inst_scale <= 0 ) return ;

	int	nowloop =  (reftime / inst_scale);	//現在までのループ数

    int checkloopnum = instanceValue.LoopNum;

	//pingpongの場合では２倍にする
    if ( instanceValue.Pingpong ) checkloopnum = checkloopnum * 2;

	//無限ループで無い時にループ数をチェック
    if ( !instanceValue.Infinity )   //無限フラグが有効な場合はチェックせず
	{
        if ( nowloop >= checkloopnum )
		{
			reftime = inst_scale-1;
			nowloop = checkloopnum-1;
		}
	}

	int temp_frame = reftime % inst_scale;  //ループを加味しないインスタンスアニメ内のフレーム

    //参照位置を決める
    //現在の再生フレームの計算
    int _time = 0;
	bool reverse = instanceValue.Reverse;
	if(instanceValue.Pingpong && (nowloop % 2 == 1))
	{
		if(reverse)
		{
			reverse = false;
		}
		else
		{
			reverse = true;
		}
	}

	if (reverse)
	{
		//リバースの時
		_time = endframe - temp_frame;
	}else{
		//通常時
		_time = temp_frame + startframe;
	}

	state->RefAnime->SetPlayFrame( _time );
	state->RefAnime->Update();
}

void FSsAnimeDecoder::UpdateEffect(float frameDelta, int nowTime, FSsPart* part, FSsPartAnime* part_anime, FSsPartState* state)
{
	if(state->Hide)
	{
		return;
	}

	if(state->EffectValue.Independent)
	{
		if(state->RefEffect && state->EffectValue.AttrInitialized)
		{
			state->EffectTimeTotal += frameDelta * state->EffectValue.Speed;
			state->RefEffect->SetLoop(true);
			state->RefEffect->SetFrame(state->EffectTimeTotal);
			state->RefEffect->Play();
			state->RefEffect->Update();
		}
	}
	else
	{
		if(state->RefEffect)
		{
			float _time = nowTime - state->EffectTime;
			if (_time < 0)
			{
				return;
			}

			_time *= state->EffectValue.Speed;
			_time += state->EffectValue.StartTime;

			state->RefEffect->SetSeedOffset(SeedOffset);
			state->RefEffect->SetFrame(_time);
			state->RefEffect->Play();
			state->RefEffect->Update();
		}
	}
}

int		FSsAnimeDecoder::FindAnimetionLabel(const FName& str, FSsAnimation* Animation)
{
	for(int i = 0; i < Animation->Labels.Num(); ++i)
	{
		if ( str == Animation->Labels[i].LabelName )
		{
			return Animation->Labels[i].Time;
		}
	}

	return 0;
}

int		FSsAnimeDecoder::CalcAnimeLabel2Frame(const FName& str, int offset, FSsAnimation* Animation)
{

	//10フレームのアニメだと11が入ってるため計算がずれるため-1する
	int maxframe = Animation->Settings.FrameCount - 1;
    int ret2 = offset;

    if (  str == "_start" )
	{
    	return offset;
	}else if ( str == "_end" )
	{
        return maxframe + offset;
	}else if ( str == "none" )
	{
        return offset;
	}else{
		int ret = FindAnimetionLabel(str, Animation);

        if ( ret != -1 )
        {
			int ret3 = ret + offset;
            if ( ret3 < 0 ) ret3 = 0;
			if ( ret3 > maxframe ) ret3 = maxframe;

        	return ret3;
		}
		//警告など出すべき？
	}

    if ( ret2 < 0 ) ret2 = 0;
    if ( ret2 > maxframe ) ret2 = maxframe;

	return ret2;



}

///SS5の場合  SsPartのarrayIndexは、親子順（子は親より先にいない）と
///なっているためそのまま木構造を作らずUpdateを行う
void FSsAnimeDecoder::Update()
{
	int32 time = (int32)NowPlatTime;

	for(int i = 0; i < PartAnime.Num(); ++i)
	{
		FSsPart* part = PartAnime[i].Key;
		FSsPartAnime* anime = PartAnime[i].Value;

		UpdateState(time, part, anime, &PartState[i]);
		UpdateMatrix(part, anime, &PartState[i]);

		if(part->Type == SsPartType::Instance)
		{
			UpdateInstance(time, part, anime, &PartState[i]);
			UpdateVertices(part, anime, &PartState[i]);
		}
		if(part->Type == SsPartType::Effect)
		{
			UpdateEffect(FrameDelta, time, part, anime, &PartState[i]);
		}
	}

	SortList.Sort();

}

// 描画用パーツデータの作成
void FSsAnimeDecoder::CreateRenderParts(TArray<FSsRenderPart>& OutRenderParts, const FVector2D* InCanvasSize, const FVector2D* InPivot)
{
	FVector2D CanvasSize = (NULL != InCanvasSize) ? *InCanvasSize : CurAnimeCanvasSize;
	FVector2D Pivot = (NULL != InPivot) ? *InPivot : CurAnimePivot;

	for(int i = 0; i < SortList.Num(); ++i)
	{
		FSsPartState* State = SortList[i];

		if(State->RefAnime)
		{
			if(!State->Hide)
			{
				State->RefAnime->CreateRenderParts(OutRenderParts, &CanvasSize, &Pivot);
			}
		}
		else if(State->RefEffect)
		{
			if(!State->Hide)
			{
				State->RefEffect->CreateRenderParts(OutRenderParts, State, CanvasSize, Pivot);
			}
		}
		else
		{
			FSsRenderPart RenderPart;
			if(CreateRenderPart(RenderPart, State, CanvasSize, Pivot))
			{
				OutRenderParts.Add(RenderPart);
			}
		}
	}
}


// 描画用パーツデータの作成、１パーツ分
bool FSsAnimeDecoder::CreateRenderPart(FSsRenderPart& OutRenderPart, FSsPartState* State, const FVector2D& CanvasSize, const FVector2D& Pivot)
{
	// 各種非表示チェック
	if(!State){ return false; }
	if(State->NoCells){ return false; }
	if(NULL == State->CellValue.Cell) { return false; }
	if(NULL == State->CellValue.Texture) { return false; }
	float HideAlpha = 1.f;
	if(!bCalcHideParts)
	{
		if(State->Hide) { return false; }
		if(0.0f == State->Alpha){ return false; }
	}
	else
	{
		if(State->Hide)
		{
			HideAlpha = 0.f;
		}
	}


	// RenderTargetに対する描画基準位置
	float OffX = (float)(CanvasSize.X /2) + (Pivot.X * CanvasSize.X);
	float OffY = (float)(CanvasSize.Y /2) - (Pivot.Y * CanvasSize.Y);

	// 頂点座標
	FMatrix ViewMatrix(
		FVector(State->Matrix[ 0], State->Matrix[ 1], State->Matrix[ 2]),
		FVector(State->Matrix[ 4], State->Matrix[ 5], State->Matrix[ 6]),
		FVector(State->Matrix[ 8], State->Matrix[ 9], State->Matrix[10]),
		FVector(State->Matrix[12], State->Matrix[13], State->Matrix[14])
		);
	FVector2D Vertices2D[4];
	for(int i = 0; i < 4; ++i)
	{
		FVector4 V = ViewMatrix.TransformPosition(FVector(
			State->Vertices[i*3 + 0],
			State->Vertices[i*3 + 1],
			State->Vertices[i*3 + 2]
			));
		Vertices2D[i] = FVector2D(V.X + OffX, -V.Y + OffY);
	}

	// 上下反転，左右反転
	if(State->HFlip)
	{
		FVector2D tmp;
		tmp = Vertices2D[0];
		Vertices2D[0] = Vertices2D[1];
		Vertices2D[1] = tmp;
		tmp = Vertices2D[2];
		Vertices2D[2] = Vertices2D[3];
		Vertices2D[3] = tmp;
	}
	if(State->VFlip)
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
		UVs[i] = FVector2D(State->CellValue.Uvs[i].X + State->Uvs[i*2 + 0] + State->UvTranslate.X, State->CellValue.Uvs[i].Y + State->Uvs[i*2 + 1] + State->UvTranslate.Y);
	}
	if(1.f != State->UvScale.X)
	{
		float Center;
		Center = (UVs[1].X - UVs[0].X) / 2.f + UVs[0].X;
		UVs[0].X = Center - ((Center - UVs[0].X) * State->UvScale.X);
		UVs[1].X = Center - ((Center - UVs[1].X) * State->UvScale.X);
		Center = (UVs[3].X - UVs[2].X) / 2.f + UVs[2].X;
		UVs[2].X = Center - ((Center - UVs[2].X) * State->UvScale.X);
		UVs[3].X = Center - ((Center - UVs[3].X) * State->UvScale.X);
	}
	if(0.f != State->UvRotation)
	{
		FVector2D UVCenter((UVs[1].X - UVs[0].X) / 2.f + UVs[0].X, (UVs[2].Y - UVs[0].Y) / 2.f + UVs[0].Y);
		float S = FMath::Sin(FMath::DegreesToRadians(State->UvRotation));
		float C = FMath::Cos(FMath::DegreesToRadians(State->UvRotation));
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
	if(1.f != State->UvScale.Y)
	{
		float Center;
		Center = (UVs[2].Y - UVs[0].Y) / 2.f + UVs[0].Y;
		UVs[0].Y = Center - ((Center - UVs[0].Y) * State->UvScale.Y);
		UVs[2].Y = Center - ((Center - UVs[2].Y) * State->UvScale.Y);
		Center = (UVs[3].Y - UVs[1].Y) / 2.f + UVs[1].Y;
		UVs[1].Y = Center - ((Center - UVs[1].Y) * State->UvScale.Y);
		UVs[3].Y = Center - ((Center - UVs[3].Y) * State->UvScale.Y);
	}

	// イメージ反転
	if(State->ImageFlipH)
	{
		FVector2D tmp;
		tmp = UVs[0];
		UVs[0] = UVs[1];
		UVs[1] = tmp;
		tmp = UVs[2];
		UVs[2] = UVs[3];
		UVs[3] = tmp;
	}
	if(State->ImageFlipV)
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
	if(State->IsColorBlend)
	{
		if(State->ColorValue.Target == SsColorBlendTarget::Whole)
		{
			const FSsColorBlendValue& cbv = State->ColorValue.Color;
			VertexColors[0].R = cbv.Rgba.R;
			VertexColors[0].G = cbv.Rgba.G;
			VertexColors[0].B = cbv.Rgba.B;
			VertexColors[0].A = (uint8)(cbv.Rgba.A * State->Alpha * HideAlpha);
			ColorBlendRate[0] = cbv.Rate;

			for(int32 i = 1; i < 4; ++i)
			{
				VertexColors[i] = VertexColors[0];
				ColorBlendRate[i] = cbv.Rate;
			}
		}
		else
		{
			for(int32 i = 0; i < 4; ++i)
			{
				const FSsColorBlendValue& cbv = State->ColorValue.Colors[i];
				VertexColors[i].R = cbv.Rgba.R;
				VertexColors[i].G = cbv.Rgba.G;
				VertexColors[i].B = cbv.Rgba.B;
				VertexColors[i].A = (uint8)(cbv.Rgba.A * State->Alpha * HideAlpha);
				ColorBlendRate[i] = cbv.Rate;
			}
		}
	}
	else
	{
		const FSsColorBlendValue& cbv = State->ColorValue.Color;
		for(int32 i = 0; i < 4; ++i)
		{
			VertexColors[i] = FColor(255, 255, 255, (uint8)(255 * State->Alpha * HideAlpha));
			ColorBlendRate[i] = 1.f;
		}
	}

	OutRenderPart.PartIndex = State->Index;
	OutRenderPart.Texture = State->CellValue.Texture;
	OutRenderPart.ColorBlendType = State->ColorValue.BlendType;
	OutRenderPart.AlphaBlendType = State->AlphaBlendType;
	for(int32 i = 0; i < 4; ++i)
	{
		OutRenderPart.Vertices[i].Position = FVector2D(Vertices2D[i].X/CanvasSize.X, Vertices2D[i].Y/CanvasSize.Y);
		OutRenderPart.Vertices[i].TexCoord = UVs[i];
		OutRenderPart.Vertices[i].Color = VertexColors[i];
		OutRenderPart.Vertices[i].ColorBlendRate = ColorBlendRate[i];
	}
	return true;
}
