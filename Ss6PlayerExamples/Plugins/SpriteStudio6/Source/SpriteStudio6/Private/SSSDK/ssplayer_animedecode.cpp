#include "ssplayer_animedecode.h"

#include <stdio.h>
#include <cstdlib>
#include <time.h>   //時間

//#include "../Loader/ssloader.h"
#include "ssplayer_matrix.h"
//#include "ssplayer_render.h"
//#include "ssplayer_effect.h"
#include "ssplayer_effect2.h"
#include "ssplayer_mesh.h"
#include "ssInterpolation.h"

//stdでののforeach宣言　


//乱数シードに利用するユニークIDを作成します。
int seedMakeID = 123456;
//エフェクトに与えるシードを取得する関数
//こちらを移植してください。
unsigned int getRandomSeed()
{
	seedMakeID++;	//ユニークIDを更新します。
	//時間＋ユニークIDにする事で毎回シードが変わるようにします。
	unsigned int rc = (unsigned int)time(0) + ( seedMakeID );

	return(rc);
}


SsAnimeDecoder::SsAnimeDecoder() : 
	curCellMapManager(0),
	partState(0),
	seedOffset(0),
	nowPlatTime(0) ,
	nowPlatTimeOld(0),
	curAnimeStartFrame(0), 
	curAnimeEndFrame(0),
	curAnimeTotalFrame(0),
	curAnimeFPS(0),
	curAnimation(nullptr),
	instancePartsHide(false),
	maskFuncFlag(true),
	maskParentSetting(true),
	meshAnimator(0)
	{
	}

SsAnimeDecoder::~SsAnimeDecoder()
{
	if ( partState )
	{
		delete [] partState;
	}

	if ( meshAnimator )
	{
		delete meshAnimator;
	}
}


void	SsAnimeDecoder::reset()
{
	for(auto e = sortList.CreateConstIterator(); e; ++e)
	{
		SsPartState* state = (*e);
		if ( state->refEffect )
		{
			state->reset();
			state->refEffect->setSeed(getRandomSeed());
			state->refEffect->reload();
			state->refEffect->stop();
			//state->refEffect->reset();
		}
	}
}

void	SsAnimeDecoder::restart()
{
#if 0
	foreach( std::list<SsPartState*> , sortList , e )
	{
		SsPartState* state = (*e);
		if ( state->refEffect )
		{
			state->refEffect->setSeed(getRandomSeed());
			state->refEffect->reload();
			state->refEffect->stop();
		}
	}
#endif

}

bool	SsAnimeDecoder::getFirstCell(FSsPart* part , SsCellValue& out)
{
	FSsPartAnime** ppSetupAnime = setupPartAnimeDic.Find(part->PartName);
	if(nullptr == ppSetupAnime)
	{
		ppSetupAnime = partAnimeDic.Find(part->PartName);
		if(nullptr == ppSetupAnime)
		{
			return false;
		}
	}

	bool	retFlag = false;

	FSsPartAnime* setupAnime = *ppSetupAnime;
	if (setupAnime && !(0 == setupAnime->Attributes.Num()))
	{
		for(auto e = setupAnime->Attributes.CreateIterator(); e; ++e)
		{
			FSsAttribute* attr = &(*e);
			switch (attr->Tag)
			{
				case SsAttributeKind::Cell:		///< 参照セル
				{
					SsGetKeyValue(part, 0, attr, out);
					retFlag = true;
				}
				break;
				default:
					break;
			}
		}

	}

	return retFlag;

}


//void	SsAnimeDecoder::setAnimation(SsModel*	model, SsAnimation* anime, SsAnimePack *animepack, SsCellMapList* cellmap, SsProject* sspj )
void	SsAnimeDecoder::setAnimation( FSsModel*	model , FSsAnimation* anime , SsCellMapList* cellmap , USs6Project* sspj )
{
	//プロジェクト情報の保存
	project = sspj;

	//セルマップリストを取得
	curCellMapManager = cellmap;
	curAnimation = anime;

	//partStateをパーツ分作成する
	partAnimeDic.Empty();
	setupPartAnimeDic.Empty();

	myModel = model;

	//パーツの数
	size_t panum = anime->PartAnimes.Num();
	for ( size_t i = 0 ; i < panum ; i++ )
	{
		FSsPartAnime* panime = &anime->PartAnimes[i];
		partAnimeDic.Add(panime->PartName, panime);
	}
	//セットアップデータの作成
	if (model->SetupAnimation)
	{
		panum = model->SetupAnimation->PartAnimes.Num();
		for (size_t i = 0; i < panum; i++)
		{
			FSsPartAnime* panime = &model->SetupAnimation->PartAnimes[i];
			setupPartAnimeDic.Add(panime->PartName, panime);
		}
	}

	//パーツとパーツアニメを関連付ける
	size_t partNum = model->PartList.Num();

	if ( partState ) delete [] partState;
	partState = new SsPartState[partNum]();
	sortList.Empty();
	partAnime.Empty();
	setupPartAnime.Empty();
	partStatesMask_.Empty();
	//マスクがあるアニメーションからないアニメーションに切り替えいた場合にdrawで無効なマスクのパーツステートを参照してしまうためクリアを追加
	maskIndexList.Empty();	
	stateNum = partNum;

	for ( size_t i = 0 ; i < partNum ; i++ ) 
	{
		FSsPart* p = &model->PartList[i];

		SsPartAndAnime _temp;
		_temp.Key = p;
		_temp.Value = partAnimeDic.Contains(p->PartName) ? partAnimeDic[p->PartName] : nullptr;
		partAnime.Add( _temp );

		SsPartAndAnime _tempSetup;
		_tempSetup.Key = p;
		_tempSetup.Value = setupPartAnimeDic.Contains(p->PartName) ? setupPartAnimeDic[p->PartName] : nullptr;
		setupPartAnime.Add(_tempSetup);

		//親子関係の設定
		if ( p->ParentIndex != -1 )
		{
			partState[i].parent = &partState[p->ParentIndex];
		}else{
			partState[i].parent = 0;
		}
		partState[i].part = p;

		//継承率の設定
		partState[i].inheritRates = p->InheritRates;
		partState[i].index = i;
		partState[i].partType = p->Type;
		partState[i].maskInfluence = p->MaskInfluence && getMaskParentSetting();


		if (sspj)
		{
			//インスタンスパーツの場合の初期設定
			if ( p->Type == SsPartType::Instance )
			{

				//参照アニメーションを取得
				FSsAnimePack* refpack = const_cast<FSsAnimePack*>(sspj->FindAnimationPack( p->RefAnimePack ));
				FSsAnimation* refanime = const_cast<FSsAnimation*>(refpack->FindAnimation( p->RefAnime ));

				SsCellMapList* __cellmap = new SsCellMapList();
				__cellmap->set( sspj , refpack );
				SsAnimeDecoder* animedecoder = new SsAnimeDecoder();

				//インスタンスパーツの設定setAnimationでソースアニメになるパーツに適用するので先に設定を行う
				animedecoder->setMaskFuncFlag(false);					//マスク機能を無効にする
				animedecoder->setMaskParentSetting(p->MaskInfluence);	//親のマスク対象を設定する 

				animedecoder->setAnimation( &refpack->Model , refanime, __cellmap , sspj );
				partState[i].refAnime = animedecoder;
				partState[i].refCellMapList = __cellmap;
				//親子関係を付ける
				animedecoder->partState[0].parent = &partState[i];
			}

			//エフェクトデータの初期設定
			if ( p->Type == SsPartType::Effect )
			{
				FSsEffectFile* f = const_cast<FSsEffectFile*>(sspj->FindEffect( p->RefEffectName ));
				if ( f )
				{
					SsEffectRenderV2* er = new SsEffectRenderV2();
					er->setParentAnimeState( &partState[i] );

					er->setCellmapManager( this->curCellMapManager );
					er->setEffectData( &f->EffectData );
					er->setSeed(getRandomSeed());
					er->reload();
					er->stop();
					er->setLoop(false);

					partState[i].refEffect = er;
				}
			}

			//マスクパーツの追加
			if (p->Type == SsPartType::Mask )
			{
				partStatesMask_.Add( &partState[i]);
			}

			//メッシュパーツの追加
			if (p->Type == SsPartType::Mesh)
			{
				SsMeshPart* mesh = new SsMeshPart();
				partState[i].meshPart = mesh;
				mesh->myPartState = &partState[i];
				//使用するセルを調査する
				SsCellValue cellv;
				bool ret = getFirstCell(p, cellv);
				if (ret)
				{
					mesh->targetCell = cellv.cell;
					mesh->targetTexture = cellv.texture;
					mesh->makeMesh();
				}
				else {
					//not found cell
				}
			}
		}

		sortList.Add( &partState[i] );

	}


	//アニメの最大フレーム数を取得
	curAnimeStartFrame = anime->Settings.StartFrame;	//Ver6.0.0開始終了フレーム対応
	curAnimeEndFrame = anime->Settings.EndFrame;
	curAnimeTotalFrame = anime->Settings.FrameCount;
	curAnimeFPS = anime->Settings.Fps;

	//メッシュアニメーションを初期化
	if(nullptr != meshAnimator)
	{
		delete meshAnimator;
		meshAnimator = nullptr;
	}
	bool bNeedMesh = false;
	for(int32 i = 0; i < stateNum; ++i)
	{
		if(partState[i].partType == SsPartType::Mesh)
		{
			bNeedMesh = true;
			break;
		}
	}
	if(bNeedMesh)
	{
		meshAnimator = new SsMeshAnimator();
		meshAnimator->setAnimeDecoder(this);
		meshAnimator->makeMeshBoneList();
	}

	
}



//頂点変形アニメデータの取得
void	SsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , FSsVertexAnime& v )
{
	//☆Mapを使っての参照なので高速化必須 
	//todo ロード時に SsVertexAnimeを作成してしまうようにする
	FSsVertexAnime	lv;
	FSsVertexAnime	rv;

	if ( rightkey == 0 ) //右側が０出会った場合
	{
		GetSsVertexAnime( leftkey , v );
		return ;
	}

	GetSsVertexAnime(leftkey,lv);
	GetSsVertexAnime(rightkey,rv);

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
	
	float rate = SsInterpolate( leftkey->IpType , now , 0.0f , 1.0f , &curve );	
	for ( int i = 0 ; i < 4 ; i++ )
	{
		
		v.Offsets[i].X = SsInterpolate( SsInterpolationType::Linear , rate , lv.Offsets[i].X , rv.Offsets[i].X , 0 );	
		v.Offsets[i].Y = SsInterpolate( SsInterpolationType::Linear , rate , lv.Offsets[i].Y , rv.Offsets[i].Y , 0 );	
//		v.offsets[i].x = SsInterpolate( leftkey->ipType , now , lv.offsets[i].x , rv.offsets[i].x , &curve );	
//		v.offsets[i].y = SsInterpolate( leftkey->ipType , now , lv.offsets[i].y , rv.offsets[i].y , &curve );	
	}

}


static	float clamp( float v , float min , float max )
{
	float ret = v;

	if ( v < min ) ret = min;
	if ( v > max ) ret = max;

	return ret;

}

void	SsAnimeDecoder::SsInterpolationValue(int time, const FSsKeyframe* leftkey, const FSsKeyframe* rightkey, SsPartsColorAnime& v)
{
	//☆Mapを使っての参照なので高速化必須
	if (rightkey == 0)
	{
		GetSsPartsColorValue(leftkey, v);
		return;
	}

	SsPartsColorAnime leftv;
	SsPartsColorAnime rightv;

	GetSsPartsColorValue(leftkey, leftv);
	GetSsPartsColorValue(rightkey, rightv);


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

	//初期化しておく
	v.color.rgba.a = 0;
	v.color.rgba.r = 0;
	v.color.rgba.g = 0;
	v.color.rgba.b = 0;
	v.target = SsColorBlendTarget::Vertex;
	v.blendType = leftv.blendType;

	now = SsInterpolate(leftkey->IpType, now, 0.0f, 1.0f, &curve);

	if (leftv.target == SsColorBlendTarget::Vertex)
	{
		if (rightv.target == SsColorBlendTarget::Vertex)
		{
			//両方とも４頂点カラー
			for (int i = 0; i < 4; i++)
			{
				v.colors[i].rate = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rate, rightv.colors[i].rate, &curve), 0.0f, 1.0f);
				v.colors[i].rgba.a = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rgba.a, rightv.colors[i].rgba.a, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.r = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rgba.r, rightv.colors[i].rgba.r, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.g = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rgba.g, rightv.colors[i].rgba.g, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.b = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rgba.b, rightv.colors[i].rgba.b, &curve), 0.0f, 255.0f);
			}
		}
		else
		{
			//左は４頂点、右は単色
			for (int i = 0; i < 4; i++)
			{
				v.colors[i].rate = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rate, rightv.color.rate, &curve), 0.0f, 1.0f);
				v.colors[i].rgba.a = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rgba.a, rightv.color.rgba.a, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.r = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rgba.r, rightv.color.rgba.r, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.g = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rgba.g, rightv.color.rgba.g, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.b = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.colors[i].rgba.b, rightv.color.rgba.b, &curve), 0.0f, 255.0f);
			}
		}
	}
	else
	{
		if (rightv.target == SsColorBlendTarget::Vertex)
		{
			//左は単色、右は４頂点カラー
			for (int i = 0; i < 4; i++)
			{
				v.colors[i].rate = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rate, rightv.colors[i].rate, &curve), 0.0f, 1.0f);
				v.colors[i].rgba.a = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rgba.a, rightv.colors[i].rgba.a, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.r = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rgba.r, rightv.colors[i].rgba.r, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.g = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rgba.g, rightv.colors[i].rgba.g, &curve), 0.0f, 255.0f);
				v.colors[i].rgba.b = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rgba.b, rightv.colors[i].rgba.b, &curve), 0.0f, 255.0f);
			}
		}
		else
		{
			//両方とも単色
			v.color.rate = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rate, rightv.color.rate, &curve), 0.0f, 1.0f);
			v.color.rgba.a = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rgba.a, rightv.color.rgba.a, &curve), 0.0f, 255.0f);
			v.color.rgba.r = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rgba.r, rightv.color.rgba.r, &curve), 0.0f, 255.0f);
			v.color.rgba.g = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rgba.g, rightv.color.rgba.g, &curve), 0.0f, 255.0f);
			v.color.rgba.b = clamp(SsInterpolate(SsInterpolationType::Linear, now, leftv.color.rgba.b, rightv.color.rgba.b, &curve), 0.0f, 255.0f);
			v.target = SsColorBlendTarget::Whole;
		}
	}

}

void	SsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsColorAnime& v )
{
	//☆Mapを使っての参照なので高速化必須
	if ( rightkey == 0 )
	{
		GetSsColorValue( leftkey , v );
		return ;
	}
	
	SsColorAnime leftv;
	SsColorAnime rightv;

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

	//初期化しておく
	v.color.rgba.a = 0;	
	v.color.rgba.r = 0;	
	v.color.rgba.g = 0;	
	v.color.rgba.b = 0;	
	v.target = SsColorBlendTarget::Vertex;
	v.blendType = leftv.blendType;

	now = SsInterpolate( leftkey->IpType , now , 0.0f , 1.0f , &curve );	

	if ( leftv.target == SsColorBlendTarget::Vertex )
	{
		if ( rightv.target == SsColorBlendTarget::Vertex )
		{
			//両方とも４頂点カラー
			for ( int i = 0 ; i < 4 ; i++ )
			{
				v.colors[i].rate = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rate , rightv.colors[i].rate  , &curve ) , 0.0f , 1.0f );	
				v.colors[i].rgba.a = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rgba.a , rightv.colors[i].rgba.a  , &curve ) , 0.0f , 255.0f );	
				v.colors[i].rgba.r = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rgba.r , rightv.colors[i].rgba.r  , &curve ) , 0.0f , 255.0f );	
				v.colors[i].rgba.g = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rgba.g , rightv.colors[i].rgba.g  , &curve ) , 0.0f , 255.0f );	
				v.colors[i].rgba.b = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rgba.b , rightv.colors[i].rgba.b  , &curve ) , 0.0f , 255.0f );	
			}
		}
		else
		{
			//左は４頂点、右は単色
			for ( int i = 0 ; i < 4 ; i++ )
			{
				v.colors[i].rate = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rate , rightv.color.rate  , &curve ) , 0.0f , 1.0f );	
				v.colors[i].rgba.a = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rgba.a , rightv.color.rgba.a  , &curve ) , 0.0f , 255.0f );	
				v.colors[i].rgba.r = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rgba.r , rightv.color.rgba.r  , &curve ) , 0.0f , 255.0f );	
				v.colors[i].rgba.g = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rgba.g , rightv.color.rgba.g  , &curve ) , 0.0f , 255.0f );	
				v.colors[i].rgba.b = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.colors[i].rgba.b , rightv.color.rgba.b  , &curve ) , 0.0f , 255.0f );	
			}
		}
	}
	else
	{
		if ( rightv.target == SsColorBlendTarget::Vertex )
		{
			//左は単色、右は４頂点カラー
			for ( int i = 0 ; i < 4 ; i++ )
			{
				v.colors[i].rate = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rate , rightv.colors[i].rate  , &curve ) , 0.0f , 1.0f );	
				v.colors[i].rgba.a = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rgba.a , rightv.colors[i].rgba.a  , &curve ) , 0.0f , 255.0f );		
				v.colors[i].rgba.r = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rgba.r , rightv.colors[i].rgba.r  , &curve ) , 0.0f , 255.0f );		
				v.colors[i].rgba.g = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rgba.g , rightv.colors[i].rgba.g  , &curve ) , 0.0f , 255.0f );		
				v.colors[i].rgba.b = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rgba.b , rightv.colors[i].rgba.b  , &curve ) , 0.0f , 255.0f );		
			}
		}
		else
		{
			//両方とも単色
			v.color.rate = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rate , rightv.color.rate  , &curve ) , 0.0f , 1.0f );	
			v.color.rgba.a = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rgba.a , rightv.color.rgba.a  , &curve ) , 0.0f , 255.0f );	
			v.color.rgba.r = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rgba.r , rightv.color.rgba.r  , &curve ) , 0.0f , 255.0f );	
			v.color.rgba.g = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rgba.g , rightv.color.rgba.g  , &curve ) , 0.0f , 255.0f );	
			v.color.rgba.b = clamp( SsInterpolate( SsInterpolationType::Linear , now , leftv.color.rgba.b , rightv.color.rgba.b  , &curve) , 0.0f , 255.0f );	
			v.target = SsColorBlendTarget::Whole;
		}
	}

}


void	SsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsCellValue& v )
{
//	SsRefCell cell;
//	GetSsRefCell( leftkey , cell );
//
//	getCellValue(	this->curCellMapManager ,
//		cell.mapid , cell.name , v );


	SsCellValue* pcellvalue = RefCellCache.Find(leftkey);
	if(pcellvalue)
	{
		v = *pcellvalue;
	}
	else
	{
		SsRefCell cell;
		GetSsRefCell( leftkey , cell );
		getCellValue(this->curCellMapManager, cell.mapid , cell.name , v);
		RefCellCache.Add(leftkey, v);
	}
}

//インスタンスアニメデータ
void	SsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsInstanceAttr& v )
{
	//補間は行わないので、常に左のキーを出力する
	GetSsInstparamAnime( leftkey , v );
}

void	SsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsEffectAttr& v )
{
	//補間は行わないので、常に左のキーを出力する
	GetSsEffectParamAnime( leftkey , v );
	
}


void	SsAnimeDecoder::SsInterpolationValue(int time, const FSsKeyframe* leftkey, const FSsKeyframe* rightkey, SsDeformAttr& v)
{
	v.verticeChgList.Empty();

	if (rightkey == 0)
	{
		GetSsDeformAnime(leftkey, v);
		return;
	}

	SsDeformAttr startValue;
	SsDeformAttr endValue;

	GetSsDeformAnime(leftkey, startValue);
	GetSsDeformAnime(rightkey, endValue);

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

	float rate = SsInterpolate(leftkey->IpType, now, 0.0f, 1.0f, &curve);


	//スタートとエンドの頂点数を比較し、多い方に合わせる(足りない部分は0とみなす)
	int numPoints = FMath::Max<int>(startValue.verticeChgList.Num(), endValue.verticeChgList.Num());

	TArray<FVector2D>& start = startValue.verticeChgList;
	//start.resize(numPoints);
	for (int i = start.Num(); i < numPoints; i++)
	{
		start.Add(FVector2D(0, 0));
	}

	TArray<FVector2D>& end = endValue.verticeChgList;
	//end.resize(numPoints);
	for (int i = end.Num(); i < numPoints; i++)
	{
		end.Add(FVector2D(0, 0));
	}

	//SsDebugPrint("start : %d, end : %d", start.size(), end.size());

	for (int i = 0; i < numPoints; i++)
	{
		FVector2D outVec;

		outVec = SsInterpolate(SsInterpolationType::Linear, rate, start[i], end[i], 0);
		v.verticeChgList.Add(outVec);

	}


}


//float , int , bool基本型はこれで値の補間を行う
template<typename mytype>
void	SsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , mytype& v )
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
void	SsAnimeDecoder::SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , bool& v )
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


template<> int	SsAnimeDecoder::SsGetKeyValue<double>(FSsPart* part, int time , const FSsAttribute* attr , double& value )
{
	float float_value = (float)value;
	int ret = SsGetKeyValue(part, time, attr, float_value);
	value = (double)float_value;
	return ret;
}
template<typename mytype> int	SsAnimeDecoder::SsGetKeyValue(FSsPart* part, int time , const FSsAttribute* attr , mytype&  value )
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
		if (curAnimation->IsSetup == false)
		{
			//セットアップアニメから先頭キーを取得する
			FSsPartAnime* setupAnime = setupPartAnimeDic.Contains(part->PartName) ? setupPartAnimeDic[part->PartName] : nullptr;
			if ((setupAnime) && (0 != setupAnime->Attributes.Num()))
			{
				const TArray<FSsAttribute>& attList = setupAnime->Attributes;
				for(auto e = attList.CreateConstIterator(); e; ++e)
				{
					const FSsAttribute* setupattr = &(*e);
					if (setupattr->Tag == attr->Tag)
					{
						lkey = setupattr->FirstKey();
						break;
					}
				}
			}
		}
		if (lkey == 0 )	//セットアップデータにキーが無い
		{
			lkey = attr->FirstKey();	//現在のアニメの先頭キーを設定する
		}

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
void	SsAnimeDecoder::updateState( int nowTime , FSsPart* part , FSsPartAnime* anime , SsPartState* state )
{

	//ステートの初期値を設定
	state->init();
	state->inheritRates = part->InheritRates;

	FSsPartAnime* setupAnime;
	{
		FSsPartAnime** ppSetupAnime = setupPartAnimeDic.Find(part->PartName);
		setupAnime = (nullptr == ppSetupAnime) ? nullptr : *ppSetupAnime;	//セットアップアニメを取得する
	}

	if ( ( anime == 0 ) && ( setupAnime == 0 ) ){
		state->hide = true;
		IdentityMatrix( state->matrix );
		state->hide = true;
		return;
	}

	// 親の継承設定を引用する設定の場合、ここで参照先を親のものに変えておく。
	if (part->InheritType == SsInheritType::Parent)
	{
		if ( state->parent )
		{
			state->inheritRates = state->parent->inheritRates;
		}
	}

	bool	size_x_key_find = false;
	bool	size_y_key_find = false;

	state->is_vertex_transform = false;
	state->is_parts_color = false;
//	state->is_color_blend = false;
	state->alphaBlendType = part->AlphaBlendType;

	bool hidekey_find = false;
	bool hideTriger = false;
	state->masklimen = 0;
	state->is_localAlpha = false;
	state->is_defrom = false;

	state->position.X = part->BonePosition.X;
	state->position.Y = part->BonePosition.Y;
	state->rotation.Z = part->BoneRotation;

	//セットアップデータをアニメーションデータ
	int idx = 0;
	for (idx = 0; idx < 2; idx++)
	{
		const TArray<FSsAttribute>* attList = nullptr;

		if (idx == 0)
		{
			//セットアップデータで初期化する
			if (!setupAnime)
			{
				continue;
			}
			if (setupAnime->Attributes.Num() == 0)
			{
				continue;
			}
			attList = &setupAnime->Attributes;
		}
		else
		{
			//アニメーションデータを解析する
			if (!anime)
			{
				continue;
			}
			if (anime->Attributes.Num() == 0)
			{
				continue;
			}
			attList = &anime->Attributes;
		}
		for(auto e = attList->CreateConstIterator(); e; ++e)
		{
			const FSsAttribute* attr = &(*e);
			switch( attr->Tag )
			{
				case SsAttributeKind::Invalid:	///< 無効値。旧データからの変換時など
					break;
				case SsAttributeKind::Cell:		///< 参照セル
					{
						SsGetKeyValue( part, nowTime , attr , state->cellValue );
						state->noCells = false;
					}
					break;
				case SsAttributeKind::Posx:		///< 位置.X
					SsGetKeyValue( part, nowTime , attr , state->position.X );
					break;
				case SsAttributeKind::Posy:		///< 位置.Y
					SsGetKeyValue( part, nowTime , attr , state->position.Y );
					break;
				case SsAttributeKind::Posz:		///< 位置.Z
					SsGetKeyValue( part, nowTime , attr , state->position.Z );
					break;
				case SsAttributeKind::Rotx:		///< 回転.X
					SsGetKeyValue( part, nowTime , attr , state->rotation.X );
					break;
				case SsAttributeKind::Roty:		///< 回転.Y
					SsGetKeyValue( part, nowTime , attr , state->rotation.Y );
					break;
				case SsAttributeKind::Rotz:		///< 回転.Z
					SsGetKeyValue( part, nowTime , attr , state->rotation.Z );
					break;
				case SsAttributeKind::Sclx:		///< スケール.X
					SsGetKeyValue( part, nowTime , attr , state->scale.X );
					break;
				case SsAttributeKind::Scly:		///< スケール.Y
					SsGetKeyValue( part, nowTime , attr , state->scale.Y );
					break;
				case SsAttributeKind::Losclx:	///< ローカルスケール.X
					SsGetKeyValue( part, nowTime , attr , state->localscale.X);
					break;
				case SsAttributeKind::Loscly:	///< ローカルスケール.X
					SsGetKeyValue( part, nowTime , attr , state->localscale.Y);
					break;
				case SsAttributeKind::Alpha:	///< 不透明度
					SsGetKeyValue( part, nowTime , attr , state->alpha);
					break;
				case SsAttributeKind::Loalpha:	///< ローカル不透明度
					SsGetKeyValue( part, nowTime , attr , state->localalpha);
					state->is_localAlpha = true;
					break;
				case SsAttributeKind::Prio:		///< 優先度
					SsGetKeyValue( part, nowTime , attr , state->prio );
					break;
//				case SsAttributeKind::fliph:	///< 左右反転(セルの原点を軸にする) Ver6非対応
//					SsGetKeyValue( nowTime , attr , state->hFlip );
//					break;
//				case SsAttributeKind::flipv:	///< 上下反転(セルの原点を軸にする) Ver6非対応
//					SsGetKeyValue( nowTime , attr , state->vFlip );
//					break;
				case SsAttributeKind::Hide:		///< 非表示
					{
						int useTime = SsGetKeyValue( part, nowTime , attr , state->hide );
						// 非表示キーがないか、先頭の非表示キーより手前の場合は常に非表示にする。
						//セットアップによってhidekey_findがあった場合は強制非表示にしない
						if ( ( useTime > nowTime ) && ( hidekey_find == false ) )
						{
							state->hide = true;
						}
						else
						{
							//非表示キーがあり、かつ最初のキーフレームを取得した
							hidekey_find = true;
						}
					}
					break;
				case SsAttributeKind::PartsColor:
					SsGetKeyValue( part, nowTime , attr , state->partsColorValue);
					state->is_parts_color = true;
					break;
//				case SsAttributeKind::color:	///< カラーブレンド  Ver6非対応
//					SsGetKeyValue( part, nowTime , attr , state->colorValue );
//					state->is_color_blend = true;
//					break;
				case SsAttributeKind::Vertex:	///< 頂点変形
					SsGetKeyValue( part, nowTime , attr , state->vertexValue );
					state->is_vertex_transform = true;
					break;
				case SsAttributeKind::Pivotx:	///< 原点オフセット.X
					SsGetKeyValue( part, nowTime , attr , state->pivotOffset.X );
					break;
				case SsAttributeKind::Pivoty:	///< 原点オフセット.Y
					SsGetKeyValue( part, nowTime , attr , state->pivotOffset.Y );
					break;
//				case SsAttributeKind::anchorx:	///< アンカーポイント.X Ver6非対応
//					SsGetKeyValue( part, nowTime , attr , state->anchor.x );
//					break;
//				case SsAttributeKind::anchory:	///< アンカーポイント.Y Ver6非対応
//					SsGetKeyValue( part, nowTime , attr , state->anchor.y );
//					break;
				case SsAttributeKind::Sizex:	///< 表示サイズ.X
					SsGetKeyValue( part, nowTime , attr , state->size.X );
					size_x_key_find = true;
					break;
				case SsAttributeKind::Sizey:	///< 表示サイズ.Y
					SsGetKeyValue( part, nowTime , attr , state->size.Y );
					size_y_key_find = true;
					break;
				case SsAttributeKind::Imgfliph:	///< イメージ左右反転(常にイメージの中央を原点とする)
					SsGetKeyValue( part, nowTime , attr , state->imageFlipH );
					break;
				case SsAttributeKind::Imgflipv:	///< イメージ上下反転(常にイメージの中央を原点とする)
					SsGetKeyValue( part, nowTime , attr , state->imageFlipV );
					break;
				case SsAttributeKind::Uvtx:		///< UVアニメ.移動.X
					SsGetKeyValue( part, nowTime , attr , state->uvTranslate.X );
					break;
				case SsAttributeKind::Uvty:		///< UVアニメ.移動.Y
					SsGetKeyValue( part, nowTime , attr , state->uvTranslate.Y );
					break;
				case SsAttributeKind::Uvrz:		///< UVアニメ.回転
					SsGetKeyValue( part, nowTime , attr , state->uvRotation );
					break;
				case SsAttributeKind::Uvsx:		///< UVアニメ.スケール.X
					SsGetKeyValue( part, nowTime , attr , state->uvScale.X );
					break;
				case SsAttributeKind::Uvsy:		///< UVアニメ.スケール.Y
					SsGetKeyValue( part, nowTime , attr , state->uvScale.Y );
					break;
//				case SsAttributeKind::Boundr:	///< 当たり判定用の半径
//					SsGetKeyValue( part, nowTime , attr , state->boundingRadius );
//					break;
				case SsAttributeKind::User:		///< Ver.4 互換ユーザーデータ
					break;
				case SsAttributeKind::Instance:	///インスタンスパラメータ
					{
						int t = SsGetKeyValue( part, nowTime , attr , state->instanceValue );
						//先頭にキーが無い場合
						if ( t  > nowTime )
						{
							state->instanceValue = SsInstanceAttr();
						}
					}
					break;
				case SsAttributeKind::Effect:
					{

						int t = SsGetKeyValue( part, nowTime , attr , state->effectValue );

						//先頭にキーが無い場合
						if ( t > nowTime )
						{
							state->effectValue = SsEffectAttr();
						}else{
							state->effectTime = t;
							if ( !state->effectValue.attrInitialized )
							{
								state->effectValue.attrInitialized  = true;
								state->effectTimeTotal = state->effectValue.startTime;
								state->effectTime = t;//state->effectValue.startTime;
							}
						}
					}
					break;
				case SsAttributeKind::Mask:
					SsGetKeyValue( part, nowTime, attr, state->masklimen);
					break;
				case SsAttributeKind::Deform:
					state->is_defrom = true;
					SsGetKeyValue(part, nowTime, attr, state->deformValue);
					break;

			}
		}
	}

	// カラー値だけアニメが無いと設定されないので初期値を入れておく。
	// alpha はupdateで初期化されるのでOK
	// 当たり判定パーツ用のカラー。赤の半透明にする
//	static const float sColorsForBoundsParts[] = {0.5f, 0.f, 0.f, 1.f};
//	for (int i = 0; i < (4*4) ; ++i)
//	{
//		if (state->noCells)
//			state->colors[i] = sColorsForBoundsParts[i & 3];
//		else
//			state->colors[i] = 1.f;
//	}

	// 非表示キーがないか、先頭の非表示キーより手前の場合は常に非表示にする。
	// 継承する場合は継承を優先するため先に処理する
	if (!hidekey_find)
	{
		state->hide = true;
	}


	// 継承

    {
		// 継承 SS5
		if (state->parent)
		{
			// α
			if (state->inherits_(SsAttributeKind::Alpha))
			{
				state->alpha *= state->parent->alpha;
			}

			//親がインスタンスパーツでかつ非表示フラグがある場合は非表示にする。
			if (instancePartsHide == true )
			{
				state->hide = true;
			}
		}
	}

	// 頂点の設定
	if ( part->Type == SsPartType::Normal || part->Type == SsPartType::Mask )
	{
		FSsCell * cell = state->cellValue.cell;
		if (cell && ( anime || setupAnime ) )
		{
			//サイズアトリビュートが指定されていない場合、セルのサイズを設定する
			if ( !size_x_key_find ) state->size.X = cell->Size.X;
			if ( !size_y_key_find ) state->size.Y = cell->Size.Y;
		}

		updateVertices(part , anime , state);
	}



}

void	SsAnimeDecoder::updateMatrix(FSsPart* part , FSsPartAnime* anime , SsPartState* state)
{
	int num = 1;
	if ((state->localscale.X != 1.0f) || (state->localscale.Y != 1.0f))
	{
		//ローカルスケール適用マトリクスを作成する
		num = 2;
	}
	int matcnt;
	for (matcnt = 0; matcnt < num; matcnt++)
	{
		float *pmat = state->matrix;	//子パーツに継承するマトリクス
		if (matcnt > 0)
		{
			pmat = state->matrixLocal;	//自分だけに適用するローカルマトリクス
		}

		if (state->parent)	//親パーツがある場合は親のマトリクスを継承する
		{
			memcpy( pmat , state->parent->matrix , sizeof( float ) * 16 );
		}
		else
		{
			IdentityMatrix( pmat );
		}

		// アンカー
//		if ( state->parent )
//		{
//			const FVector2D& parentSize = state->parent->size;
//			state->position.X = state->position.X + state->parent->size.X * state->anchor.X;
//			state->position.Y = state->position.Y + state->parent->size.Y * state->anchor.Y;
//		}

		TranslationMatrixM( pmat , state->position.X, state->position.Y, state->position.Z );//
		RotationXYZMatrixM( pmat , FMath::DegreesToRadians(state->rotation.X) , FMath::DegreesToRadians(state->rotation.Y) , FMath::DegreesToRadians( state->rotation.Z) );
		float sx = state->scale.X;
		float sy = state->scale.Y;
		if (matcnt > 0)
		{
			//ローカルスケールを適用する
			sx *= state->localscale.X;
			sy *= state->localscale.Y;
		}
		if ((sx != 1.f) || (sy != 1.f))
		{
			ScaleMatrixM(pmat, sx, sy, 1.0f);
		}
	}
	if (num == 1)
	{
		//ローカルスケールが使用されていない場合は継承マトリクスをローカルマトリクスに適用
		memcpy(state->matrixLocal, state->matrix, sizeof(state->matrix));
	}


}


void	SsAnimeDecoder::updateVertices(FSsPart* part , FSsPartAnime* anime , SsPartState* state)
{

	FSsCell * cell = state->cellValue.cell;

	FVector2D pivot;

	if (cell)
	{
		// セルに設定された原点オフセットを適用する
		// ※セルの原点は中央が0,0で＋が右上方向になっている
		float cpx = cell->Pivot.X + 0.5f;
//		if (state->hFlip) cpx = 1 - cpx;	// 水平フリップによって原点を入れ替える
		pivot.X = cpx * state->size.X;
		// 上が＋で入っているのでここで反転する。
		float cpy = -cell->Pivot.Y + 0.5f;
//		if (state->vFlip) cpy = 1 - cpy;	// 垂直フリップによって原点を入れ替える
		pivot.Y = cpy * state->size.Y;
	}
	else
	{
		// セルが無いパーツでも原点が中央に来るようにする。
		pivot.X = 0.5f * state->size.X;
		pivot.Y = 0.5f * state->size.Y;
	}

	// 次に原点オフセットアニメの値を足す
	pivot.X += state->pivotOffset.X * state->size.X;
	pivot.Y += -state->pivotOffset.Y * state->size.Y;

	float sx = -pivot.X;
	float ex = sx + state->size.X;
	float sy = +pivot.Y;
	float ey = sy - state->size.Y;

	// Z順
	/*
		これは実は上下ひっくり返って裏面になっているためUV値も上下反転させている。
		左上が最初に来る並びの方が頂点カラー・頂点変形のデータと同じで判りやすいのでこれでいく。
	*/
	float vtxPosX[4] = {sx, ex, sx, ex};
	float vtxPosY[4] = {sy, sy, ey, ey};

	FVector2D * vtxOfs = state->vertexValue.Offsets;

	//きれいな頂点変形に対応
#if USE_TRIANGLE_FIN

	if ( state->is_parts_color || state->is_vertex_transform )
	{

		FVector2D	vertexCoordinateLU = FVector2D( sx + (float)vtxOfs[0].X , sy + (float)vtxOfs[0].Y );// : 左上頂点座標（ピクセル座標系）
		FVector2D	vertexCoordinateRU = FVector2D( ex + (float)vtxOfs[1].X , sy + (float)vtxOfs[1].Y );// : 右上頂点座標（ピクセル座標系）
		FVector2D	vertexCoordinateLD = FVector2D( sx + (float)vtxOfs[2].X , ey + (float)vtxOfs[2].Y );// : 左下頂点座標（ピクセル座標系）
		FVector2D	vertexCoordinateRD = FVector2D( ex + (float)vtxOfs[3].X , ey + (float)vtxOfs[3].Y );// : 右下頂点座標（ピクセル座標系）

		FVector2D CoordinateLURU = (vertexCoordinateLU + vertexCoordinateRU) * 0.5f;
		FVector2D CoordinateLULD = (vertexCoordinateLU + vertexCoordinateLD) * 0.5f;
		FVector2D CoordinateLDRD = (vertexCoordinateLD + vertexCoordinateRD) * 0.5f;
		FVector2D CoordinateRURD = (vertexCoordinateRU + vertexCoordinateRD) * 0.5f;

		FVector2D center;
		CoordinateGetDiagonalIntersection( center , CoordinateLURU, CoordinateRURD, CoordinateLULD, CoordinateLDRD );

		FVector2D*	coodinatetable[] = { &vertexCoordinateLU , &vertexCoordinateRU , &vertexCoordinateLD , &vertexCoordinateRD , &center };


		for (int i = 0; i < 5; ++i)
		{
			state->vertices[ i * 3 ] = coodinatetable[i]->X;
			state->vertices[ i * 3 + 1 ] = coodinatetable[i]->Y;
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
	//SsPoint2 * vtxOfs = vertexValue.offsets;
	for (int i = 0; i < 4; ++i)
	{
		const FSs6ProjectSetting& projsetting = project->GetProjectSetting();
		if (projsetting.VertexAnimeFloat != 0 )	//頂点変形を少数で行う
		{
			state->vertices[i * 3] = vtxPosX[i] + (float)vtxOfs->X;
			state->vertices[i * 3 + 1] = vtxPosY[i] + (float)vtxOfs->Y;
		}
		else
		{
			state->vertices[i * 3] = vtxPosX[i] + (int)vtxOfs->X;
			state->vertices[i * 3 + 1] = vtxPosY[i] + (int)vtxOfs->Y;
		}


		state->vertices[i * 3 + 2]	= 0;

		++vtxOfs;
	}
#endif



}





void	SsAnimeDecoder::updateInstance( int nowTime , FSsPart* part , FSsPartAnime* partanime , SsPartState* state )
{
	if ( state->refAnime == 0 ) return ;
	//state->refAnime->setPlayFrame( nowTime );
	//state->refAnime->update();

	FSsAnimation* anime = state->refAnime->curAnimation;
	const SsInstanceAttr& instanceValue = state->instanceValue;

    //プレイヤー等では再生開始時にいったん計算してしまって値にしてしまった方がいい。
    //エディター側のロジックなのでそのまま検索する
    //インスタンスアニメ内のスタート位置
    int	startframe = CalcAnimeLabel2Frame( instanceValue.startLabel , instanceValue.startOffset, anime);
    int	endframe = CalcAnimeLabel2Frame( instanceValue.endLabel , instanceValue.endOffset, anime);

    state->instanceValue.startFrame = startframe;		//ラベル位置とオフセット位置を加えた実際のフレーム数
    state->instanceValue.endFrame = endframe;			//ラベル位置とオフセット位置を加えた実際のフレーム数


    //タイムライン上の時間 （絶対時間）
	int time = nowTime;

	//独立動作の場合
	if ( instanceValue.independent )
	{
		//float delta = animeState->frame - parentBackTime;
		float delta = this->frameDelta;

		state->instanceValue.liveFrame+= ( delta * instanceValue.speed );
		//parentBackTime = animeState->frame;
		time = (int)instanceValue.liveFrame;

	}

    //このインスタンスが配置されたキーフレーム（絶対時間）
    int	selfTopKeyframe = instanceValue.curKeyframe;


    int reftime = ( time - selfTopKeyframe) * instanceValue.speed;
    //int	reftime = (time*instanceValue.speed) - selfTopKeyframe; //開始から現在の経過時間
	if ( reftime < 0 ) return ; //そもそも生存時間に存在していない
	if ( selfTopKeyframe > time ) return ;

    int inst_scale = (endframe - startframe) + 1; //インスタンスの尺


	//尺が０もしくはマイナス（あり得ない
	if ( inst_scale <= 0 ) return ;

	int	nowloop =  (reftime / inst_scale);	//現在までのループ数

    int checkloopnum = instanceValue.loopNum;

	//pingpongの場合では２倍にする
    if ( instanceValue.pingpong ) checkloopnum = checkloopnum * 2;

	//無限ループで無い時にループ数をチェック
    if ( !instanceValue.infinity )   //無限フラグが有効な場合はチェックせず
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
	bool	reverse = instanceValue.reverse;
	if ( instanceValue.pingpong && (nowloop % 2 == 1) )
	{
		if (reverse)
		{
			reverse = false;//反転
		}
		else
		{
			reverse = true;//反転
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

	state->refAnime->setInstancePartsHide(state->hide);
	state->refAnime->setPlayFrame(_time);

	//Ver6 ローカルスケール対応
	//ローカルスケールを適用するために一時的に継承マトリクスを入れ替える
	float mattemp[16];
	memcpy(mattemp, state->matrix, sizeof(state->matrix));					//継承用マトリクスを退避する
	memcpy(state->matrix, state->matrixLocal, sizeof(state->matrixLocal));	//ローカルをmatrixに適用する

	float orgAlpha = state->alpha;
	if (state->is_localAlpha)
	{
		//ローカス不透明度が使用されている場合は不透明度をアップデート中だけ上書きする
		state->alpha = state->localalpha;									// ローカル不透明度対応
	}

	state->refAnime->update(this->frameDelta);								//インスタンスが参照するソースアニメのアップデート

	memcpy(state->matrix, mattemp, sizeof(mattemp));						//継承用マトリクスを戻す
	state->alpha = orgAlpha;

  
	//頂点の作成
    //update_vertices();

}

int		SsAnimeDecoder::findAnimetionLabel(const FName& str, FSsAnimation* Animation)
{
	for(auto itr = Animation->Labels.CreateConstIterator(); itr; ++itr)
	{
		if ( str == (*itr).LabelName )
		{
			return (*itr).Time;
		}
	}

	return 0;
}

int		SsAnimeDecoder::CalcAnimeLabel2Frame(const FName& str, int offset, FSsAnimation* Animation )
{

	//10フレームのアニメだと11が入ってるため計算がずれるため-1する
	int maxframe = Animation->Settings.FrameCount - 1;
	int startframe = Animation->Settings.StartFrame;	//Ver6.0対応
	int endframe = Animation->Settings.EndFrame;		//Ver6.0対応
	int ret2 = offset;

    if (  str == "_start" )
	{
		ret2 = startframe + offset;
	}else if ( str == "_end" )
	{
		ret2 = endframe + offset;
	}else if ( str == "none" )
	{
        return offset;
	}else{
		int ret = findAnimetionLabel(str, Animation);

        if ( ret != -1 )
        {
			int ret3 = ret + offset;
			if ( ret3 < startframe ) ret3 = startframe;
			if ( ret3 > endframe ) ret3 = endframe;

        	return ret3;
		}
		//警告など出すべき？
	}

    if ( ret2 < startframe ) ret2 = startframe;
	if ( ret2 > endframe ) ret2 = endframe;

	return ret2;



}

//マスク用ステンシルバッファの初期化を行うか？
//インスタンスパーツとして再生する場合のみ設定する
void	SsAnimeDecoder::setMaskFuncFlag(bool flg)
{
	maskFuncFlag = flg;
}

//親のマスク対象
//インスタンスパーツとして再生する場合のみ設定する
//各パーツのマスク対象とアンドを取って処理する
void	SsAnimeDecoder::setMaskParentSetting(bool flg)
{
	maskParentSetting = flg;
}

//static SsPartStateLess _ssPartStateLess;

///SS5の場合  SsPartのarrayIndexは、親子順　（子は親より先にいない）と
///なっているためそのまま木構造を作らずUpdateを行う
void	SsAnimeDecoder::update(float frameDeltaLocal)
{
	int	time = (int)nowPlatTime;

	this->frameDelta = frameDeltaLocal;

	int cnt = 0;
	for(auto e = partAnime.CreateConstIterator(); e; ++e)
	{
		FSsPart* part = e->Key;
		FSsPartAnime* anime = e->Value;

		updateState( time , part , anime , &partState[cnt] );

		updateMatrix( part , anime , &partState[cnt]);

		if ( part->Type == SsPartType::Instance )
		{
			updateInstance( time , part , anime , &partState[cnt] );
			updateVertices( part , anime , &partState[cnt] );
		}

		if ( part->Type == SsPartType::Effect)
		{
			updateMatrix( part , anime , &partState[cnt]);
			updateEffect(frameDeltaLocal, time, part, anime, &partState[cnt]);
		}

		cnt++;
	}


	if (meshAnimator)
		meshAnimator->update();


	sortList.Sort();
	partStatesMask_.Sort();

	maskIndexList.Empty();
	for(auto it = partStatesMask_.CreateConstIterator(); it; ++it)
	{
		SsPartState * ps = (*it);
		maskIndexList.Add(ps);
	}

	//今回再生した時間を保存しておく
	nowPlatTimeOld = nowPlatTime;

}


void	SsAnimeDecoder::updateEffect( float frameDeltaLocal , int nowTime , FSsPart* part , FSsPartAnime* part_anime , SsPartState* state )
{
	if ( state->hide ) return ;

	if ( state->effectValue.independent )
	{
		if (state && state->refEffect && state->effectValue.attrInitialized )
		{
			state->effectTimeTotal += frameDeltaLocal* state->effectValue.speed;
			state->refEffect->setLoop(true);
			state->refEffect->setFrame( state->effectTimeTotal );
			state->refEffect->play();
			state->refEffect->update();
		}
	}else{
		if (state && state->refEffect)
		{
			float _time = nowTime - state->effectTime;
			if ( _time < 0 )
			{
				return ;
			}

			_time*= state->effectValue.speed;
			_time += state->effectValue.startTime;

			state->refEffect->setSeedOffset( seedOffset );
			state->refEffect->setFrame( _time );
			state->refEffect->play();
			state->refEffect->update();
		}
	}



}
#if 0
//描画
void	SsAnimeDecoder::draw()
{

	SsCurrentRenderer::getRender()->renderSetup();


	if (maskFuncFlag == true) //マスク機能が有効（インスタンスのソースアニメではない）
	{
		//初期に適用されているマスクを精製
		for (size_t i = 0; i < maskIndexList.size(); i++)
		{
			SsPartState * ps = maskIndexList[i];

			if (!ps->hide)
			{
				//ステンシルバッファの作成
				//ps->partType = SsPartType::mask;
				SsCurrentRenderer::getRender()->renderPart(ps);
			}
		}
	}

	int mask_index = 0;

	foreach( std::list<SsPartState*> , sortList , e )
	{
		SsPartState* state = (*e);

		if (state->partType == SsPartType::mask)
		{
			//マスクパーツ

			//6.2対応
			//非表示の場合でもマスクの場合は処理をしなくてはならない
			//マスクはパーツの描画より先に奥のマスクパーツから順にマスクを作成していく必要があるため
			//通常パーツの描画順と同じ箇所で非表示によるスキップを行うとマスクのバッファがクリアされずに、
			//マスクが手前の優先度に影響するようになってしまう。
			if (maskFuncFlag == true) //マスク機能が有効（インスタンスのソースアニメではない）
			{
				SsCurrentRenderer::getRender()->clearMask();
				mask_index++;	//0番は処理しないので先にインクメントする

				for (size_t i = mask_index; i < maskIndexList.size(); i++)
				{
					SsPartState * ps2 = maskIndexList[i];
					if (!ps2->hide)
					{
						SsCurrentRenderer::getRender()->renderPart(ps2);
					}
				}
			}
		}

		if ( state->hide )continue;

		if ( state->refAnime )
		{
			//インスタンスパーツ
			SsCurrentRenderer::getRender()->execMask(state);
			state->refAnime->draw();
		}
		else if ( state->refEffect )
		{
			//エフェクトパーツ
			SsCurrentRenderer::getRender()->execMask(state);

			//Ver6 ローカルスケール対応
			//ローカルスケールを適用するために一時的に継承マトリクスを入れ替える
			float mattemp[16];
			memcpy(mattemp, state->matrix, sizeof(state->matrix));					//継承用マトリクスを退避する
			memcpy(state->matrix, state->matrixLocal, sizeof(state->matrixLocal));	//ローカルをmatrixに適用する

			float orgAlpha = state->alpha;
			if (state->is_localAlpha)
			{
				//ローカス不透明度が使用されている場合は不透明度をアップデート中だけ上書きする
				state->alpha = state->localalpha;									// ローカル不透明度対応
			}

			state->refEffect->draw();

			memcpy(state->matrix, mattemp, sizeof(mattemp));						//継承用マトリクスを戻す
			state->alpha = orgAlpha;
		}
		else if (state->partType != SsPartType::mask)
		{
			//通常パーツ
			SsCurrentRenderer::getRender()->renderPart(state);
		}
	}

	SsCurrentRenderer::getRender()->enableMask(false);
}
#endif
