#include "SpriteStudio6PrivatePCH.h"


#include <stdio.h>
#include <cstdlib>

//#include "../Loader/ssloader.h"

#include "ssplayer_animedecode.h"
#include "ssplayer_effect2.h"
#include "ssplayer_macro.h"
#include "ssplayer_matrix.h"
//#include "ssplayer_render.h"
#include "ssplayer_effectfunction.h"

#include "SsEffectFile.h"

#define DEBUG_DISP (0)
#define BUILD_ERROR_0418 (0)

#pragma warning(disable:4456)


static uint8 blendNumber( uint8 a , uint8 b , float rate )
{
	return ( a + ( b - a ) * rate );
}

static float blendFloat( float a,float b , float rate )
{
	return   ( a + ( b - a ) * rate );
}

static float get_angle_360(const FVector2D& v0, const FVector2D& v1)
{
	FVector2D uv0(v0), uv1(v1);
	uv0.Normalize();
	uv1.Normalize();

	float ang;
	{
		float ip = FVector2D::DotProduct(uv0, uv1);
		if (ip > 1.0f) { ip = 1.0f; }
		if (ip < -1.0f) { ip = -1.0f; }
		ang = FMath::Acos(ip);
	}

	float c = FVector2D::CrossProduct(uv0, uv1);

	if (c < 0)
	{
		ang = (3.1415926535897932385f)*2.0f - ang;
	}
	return ang;
}


double OutQuad(double t,double totaltime,double max ,double min )
{
	if( totaltime == 0.0 ) return 0.0;

	if ( t > totaltime ) t = totaltime;
	max -= min;
	t /= totaltime;
	return -max*t*(t-2)+min;
}

//現在時間から産出される位置を求める
//time変数から求められる式とする
//パーティクル座標計算のコア
void	SsEffectEmitter::updateParticle(float time, particleDrawData* p, bool recalc )
{
	float _t = (float)(time - p->stime);
	float _tm = (float)(_t - 1.0f );
	float _t2 = _t * _t; //(経過時間の二乗)
	float _life = (float)( p->lifetime - p->stime);

	if ( _life == 0 ) return ;
	float _lifeper = (float)( _t / _life );


	//_t = 0時点の値を作る
	//シード値で固定化されることが前提
  	unsigned long pseed = seedList[p->id % seedTableLen];


	//自身のシード値、エミッターのシード値、親パーティクルのＩＤをシード値とする
	rand.init_genrand(( pseed + emitterSeed + p->pid + seedOffset ));


	float rad = particle.angle + (rand.genrand_float32() * (particle.angleVariance ) - particle.angleVariance/2.0f);
	//float speed = rand.genrand_float32() * particle.speed;
	float speed = particle.speed + ( particle.speed2 * rand.genrand_float32() );



	//接線加速度
	float addr = 0;
	if ( particle.useTanAccel )
	{
		float accel = particle.tangentialAccel + (rand.genrand_float32() * particle.tangentialAccel2);

		float _speed = speed;
		if ( _speed <= 0 )_speed = 0.1f;
		//平均角速度を求める
		float l = _life * _speed * 0.2f; //円の半径
		float c = 3.14 * l;

		//最円周 / 加速度(pixel)
		addr = ( accel / c ) * _t;
	}

	float x = cos(rad + addr) * speed * (float)_t;
	float y = sin(rad + addr) * speed * (float)_t;

	if ( particle.useTransSpeed )
	{
		float transspeed = particle.transSpeed + ( particle.transSpeed2 * rand.genrand_float32() );
        float speedadd = transspeed / _life;

		float addtx =  cos(rad + addr) * speed;
		float addtx_trans =  cos(rad + addr) * speedadd;

		float addx = (( addtx_trans * _t ) + addtx ) * (_t+1.0f) / 2.0f;


		float addty =  sin(rad + addr) * speed;
		float addty_trans =  sin(rad + addr) * speedadd;

		float addy = (( addty_trans * _t ) + addty ) * ( _t+1.0f) / 2.0f;

		x = addx;
		y = addy;

	}


	//重力加速度の計算
	if ( particle.useGravity )
	{
		x += (0.5 * particle.gravity.X * (_t2));
		y += (0.5 * particle.gravity.Y * (_t2));
	}

	//初期位置オフセット
	float ox,oy;
	ox = oy = 0;
	if ( particle.useOffset )
	{
		ox = (particle.offset.X + (particle.offset2.X * rand.genrand_float32()));
		oy = (particle.offset.Y + (particle.offset2.Y * rand.genrand_float32()));
	}

	//角度初期値
	p->rot = 0;
	if ( particle.useRotation )
	{
		p->rot = particle.rotation + (rand.genrand_float32() * particle.rotation2);
		float add = particle.rotationAdd + (rand.genrand_float32() * particle.rotationAdd2);

		//角度変化
		if ( particle.useRotationTrans )
		{
			//到達までの絶対時間
			float lastt = _life * particle.endLifeTimePer;

			float addf = 0;
			if ( lastt == 0 )
			{
			  	float addrf =  (add * particle.rotationFactor) * _t;
				p->rot+=addrf;
			}else{
				//1フレームで加算される量
				addf = ( add * particle.rotationFactor - add ) / lastt;

				//あまり時間
				float mod_t = _t - lastt;
				if ( mod_t < 0 ) mod_t = 0;

				//現在時間（最終時間でリミット
				float nowt = _t;
				if ( nowt > lastt ) nowt = lastt;

				//最終項 + 初項 x F / 2
				float final_soul = add + addf * nowt;
				float addrf = ( final_soul + add ) * (nowt+1.0f) / 2.0f;
				addrf-=add;
				addrf+= ( mod_t * ( final_soul ) ); //あまりと終項の積を加算
				p->rot+=addrf;
			}
		}else{
			p->rot+= ( (add*_t) );
		}
	}

	//カラーの初期値、カラーの変化
	p->color.A = 0xff;
	p->color.R = 0xff;
	p->color.G = 0xff;
	p->color.B = 0xff;

	if ( particle.useColor)
	{
		p->color.A = particle.initColor.A + (rand.genrand_float32() * particle.initColor2.A );
		p->color.R = particle.initColor.R + (rand.genrand_float32() * particle.initColor2.R );
		p->color.G = particle.initColor.G + (rand.genrand_float32() * particle.initColor2.G );
		p->color.B = particle.initColor.B + (rand.genrand_float32() * particle.initColor2.B );
	}

	if ( particle.useTransColor )
	{
		FSsU8Color ecolor;
		ecolor.A = particle.transColor.A + (rand.genrand_float32() * particle.transColor2.A );
		ecolor.R = particle.transColor.R + (rand.genrand_float32() * particle.transColor2.R );
		ecolor.G = particle.transColor.G + (rand.genrand_float32() * particle.transColor2.G );
		ecolor.B = particle.transColor.B + (rand.genrand_float32() * particle.transColor2.B );

		p->color.A = blendNumber( p->color.A , ecolor.A , _lifeper );
		p->color.R = blendNumber( p->color.R , ecolor.R , _lifeper );
		p->color.G = blendNumber( p->color.G , ecolor.G , _lifeper );
		p->color.B = blendNumber( p->color.B , ecolor.B , _lifeper );
	}

	if ( particle.useAlphaFade )
	{

		float start = particle.alphaFade;
		float end = particle.alphaFade2;
        float per = _lifeper * 100.0f;


		if ( ( per < start ) && ( start > 0.0f ) ) //Ver6.2　0除算発生する可能性対策
		{
			float alpha = (start - per) / start;
			p->color.A*= 1.0f - alpha;
		}else{

			if ( per > end )
			{

				if (end>=100.0f)
				{
					p->color.A = 0;
				}else{
					float alpha = (per-end) / (100.0f-end);
                    if ( alpha >=1.0f ) alpha = 1.0f;

					p->color.A*= 1.0f - alpha;
				}
			}
		}
	}


	//スケーリング
	p->scale.X = 1.0f;
	p->scale.Y = 1.0f;
	float scalefactor = 1.0f;

	if ( particle.useInitScale )
	{
		p->scale.X = particle.scale.X + (rand.genrand_float32() * particle.scaleRange.X );
		p->scale.Y = particle.scale.Y + (rand.genrand_float32() * particle.scaleRange.Y );

        scalefactor = particle.scaleFactor + (rand.genrand_float32() * particle.scaleFactor2 );


	}

	if ( particle.useTransScale )
	{
		FVector2D s2;
		float sf2;
		s2.X = particle.transscale.X + (rand.genrand_float32() * particle.transscaleRange.X );
		s2.Y = particle.transscale.Y + (rand.genrand_float32() * particle.transscaleRange.Y );

		sf2 = particle.transscaleFactor + (rand.genrand_float32() * particle.transscaleFactor2 );

		p->scale.X = blendFloat( p->scale.X , s2.X , _lifeper );
		p->scale.Y = blendFloat( p->scale.Y , s2.Y , _lifeper );
        scalefactor = blendFloat( scalefactor , sf2 , _lifeper );

	}

	p->scale.X*=scalefactor;
	p->scale.Y*=scalefactor;

	p->x = x + ox + position.X;//エミッタからのオフセットを加算
	p->y = y + oy + position.Y;//エミッタからのオフセットを加算


  	//指定の点へよせる
	if ( particle.usePGravity )
	{

		//生成地点からの距離
		FVector2D v = FVector2D(  particle.gravityPos.X - (ox + position.X) ,
                         particle.gravityPos.Y - (oy + position.Y) );


		FVector2D nv = v;
		nv.Normalize();

		float gp = particle.gravityPower;
		if (gp > 0) {
			FVector2D v2 = FVector2D(p->x, p->y);

			//6.2対応　収束点座標を(0, 0)にすると収束しない
			float len = v.Size(); // 生成位置からの距離
			if (len == 0.0f) {
				len = 0.1f;
				nv.X = 1;
				nv.Y = 0;
			}

			float et = (len / gp)*0.90f;;

			float _gt = _t;
			if ( _gt >= (int)et )
			{
				_gt = et*0.90f;// + (_t / _life *0.1f);
			}

			nv = nv * gp * _gt;
			p->x += nv.X;
			p->y += nv.Y;


			float blend = OutQuad(_gt, et, 0.9f, 0.0f);
//			blend = blend; // *gp;
			blend += (_t / _life *0.1f);

			p->x = blendFloat(p->x, particle.gravityPos.X, blend);
			p->y = blendFloat(p->y, particle.gravityPos.Y, blend);

		}
		else {
			nv = nv * gp * _t;
			// パワーマイナスの場合は単純に反発させる
			// 距離による減衰はない
			p->x += nv.X;
			p->y += nv.Y;
		}

#if 0
		float gx = OutQuad( _t *0.8f , _life ,  particle.gravityPos.X , ox + position.X );
		float gy = OutQuad( _t *0.8f , _life ,  particle.gravityPos.Y , oy + position.Y );

		float gp = particle.gravityPower;

		if ( gp < 0 )
		{
			p->x = blendFloat( p->x , -gx , -gp);
			p->y = blendFloat( p->y , -gy , -gp);

		}else{
			p->x = blendFloat( p->x , gx , gp);
			p->y = blendFloat( p->y , gy , gp);
		}
#endif
	}

    //前のフレームからの方向を取る
	p->direc = 0.0f;
	if ( particle.useTurnDirec && recalc==false )
	{
		particleDrawData dp;
        dp = *p;

//		if ( time > 0.0f )
		{
			updateParticle( time + 1.0f , &dp , true );
			p->direc =  get_angle_360(
								FVector2D( 1 , 0 ) ,
								FVector2D(p->x - dp.x, p->y - dp.y) ) + DegreeToRadian(90) + DegreeToRadian(particle.direcRotAdd);
		}
	}


}


bool compare_life( emitPattern& left,  emitPattern& right)
{
	if ( left.life == right.life )
	{
        if ( left.uid < right.uid ) return true;

	}

	return left.life < right.life ;
}

void	SsEffectEmitter::precalculate2()
{
	rand.init_genrand( emitterSeed );

	_emitpattern.Empty();
	//_lifeExtend.clear();
	_offsetPattern.Empty();

	if ( particleExistList == 0 )
	{
		particleExistList = new particleExistSt[emitter.emitmax]; //存在しているパーティクルが入る計算用バッファ
	}

	memset( particleExistList , 0 , sizeof(particleExistSt) * emitter.emitmax );

	if ( emitter.emitnum < 1 ) emitter.emitnum = 1;

	int cycle =  (int)(( (float)(emitter.emitmax * emitter.interval)  / (float)emitter.emitnum ) + 0.5f) ;
    int group =  emitter.emitmax / emitter.emitnum;

	int extendsize = emitter.emitmax*LIFE_EXTEND_SCALE;
    if ( extendsize < LIFE_EXTEND_MIN ) extendsize = LIFE_EXTEND_MIN;




	int shot = 0;
	int offset = particle.delay;
	for ( int i = 0 ; i < emitter.emitmax ; i++ )
	{
		if ( shot >= emitter.emitnum )
		{
			shot = 0;
			offset+= emitter.interval;
		}
		_offsetPattern.Add(offset);
		shot++;
	}


	for ( int i = 0 ; i < extendsize ; i++ )
	{
		emitPattern e;
		e.uid = i;
		e.life = emitter.particleLife + emitter.particleLife2 * rand.genrand_float32();
		e.cycle = cycle;

		if ( e.life > cycle )
		{
			e.cycle = e.life;
		}

		_emitpattern.Add( e );
	}


	if (seedList != NULL)
	{
		delete[] seedList;
	}

    particleListBufferSize = emitter.emitmax;


	rand.init_genrand((emitterSeed));

	seedTableLen = particleListBufferSize * 3;
	seedList = new unsigned long[seedTableLen];
	//各パーティクルＩＤから参照するシード値をテーブルとして作成する
	for ( size_t i = 0 ; i < seedTableLen ; i++ )
	{
    	seedList[i] = rand.genrand_uint32();
	}

}



//----------------------------------------------------------------------------------




void SsEffectEmitter::updateEmitter( double _time , int slide ) 
{
	int onum = _offsetPattern.Num();
	int pnum = _emitpattern.Num();
	slide = slide * SEED_MAGIC;


	for ( int i = 0 ; i < onum ; i ++ )
	{
		int slide_num = ( i + slide ) % pnum;

		emitPattern* targetEP = &_emitpattern[slide_num];

		int t = (int)(_time - _offsetPattern[i]);

		particleExistList[i].exist = false;
		particleExistList[i].born = false;

		if ( targetEP->cycle != 0 )
		{
			int loopnum = t / targetEP->cycle;
			int cycle_top = loopnum * targetEP->cycle;

            particleExistList[i].cycle = loopnum;

			particleExistList[i].stime = cycle_top + _offsetPattern[i];
			particleExistList[i].endtime = particleExistList[i].stime + targetEP->life;// + _lifeExtend[slide_num];

			if ( (double)particleExistList[i].stime <= _time &&  (double)particleExistList[i].endtime > _time )
			{
				particleExistList[i].exist = true;
				particleExistList[i].born = true;
			}

			if ( !this->emitter.Infinite )
			{
				if ( particleExistList[i].stime >= this->emitter.life ) //エミッターが終了している
				{
					particleExistList[i].exist = false;    //作られてない

					//最終的な値に計算し直し <-事前計算しておくといいかも・
					int t2 = this->emitter.life - _offsetPattern[i];
					int loopnum2 = t2 / targetEP->cycle;

					int cycle_top2 = loopnum2 * targetEP->cycle;

					particleExistList[i].stime = cycle_top2 + _offsetPattern[i];

					particleExistList[i].endtime = particleExistList[i].stime + targetEP->life;// + _lifeExtend[slide_num];
					particleExistList[i].born = false;
				}else{
					particleExistList[i].born = true;
				}
			}

			if ( t < 0 ){
				 particleExistList[i].exist = false;
				 particleExistList[i].born = false;
			}
		}
	}

}


const particleExistSt*	SsEffectEmitter::getParticleDataFromID(int id)
{

	return &particleExistList[id];
}

/*
void	SsEffectRenderV2::drawSprite(
		SsCellValue*		dispCell,
		FVector2D _position,
		FVector2D _size,
		float     _rotation,
		float	  direction,
		SsFColor	_color,
		SsRenderBlendType::_enum blendType
	)
{

	//SsCellValue*			dispCell;

	SsCurrentRenderer::getRender()->renderSetup();	

	switch( blendType )
	{
		case SsRenderBlendType::Mix:
			SsCurrentRenderer::getRender()->SetAlphaBlendMode(SsBlendType::mix);					
			break;
		case SsRenderBlendType::Add:
			SsCurrentRenderer::getRender()->SetAlphaBlendMode(SsBlendType::add);					
			break;
	}

	SsCurrentRenderer::getRender()->SetTexture( dispCell );


	float		matrix[4 * 4];	///< 行列
	IdentityMatrix( matrix );

	float parentAlpha = 1.0f;

	if ( parentState )
	{
		memcpy( matrix , parentState->matrix , sizeof( float ) * 16 );
    	parentAlpha = parentState->alpha;
	}


	TranslationMatrixM( matrix , _position.X * layoutScale.X , _position.Y * layoutScale.Y , 0.0f );

	RotationXYZMatrixM( matrix , 0 , 0 , DegreeToRadian(_rotation)+direction );

    ScaleMatrixM(  matrix , _size.X, _size.Y, 1.0f );

	SsFColor fcolor;
	fcolor.fromARGB( _color.toARGB() );
	fcolor.a = fcolor.a * parentAlpha;


	if ( ( dispCell->cell ) && ( fcolor.a != 0.0f ) )
	{

		FVector2D pivot = FVector2D( dispCell->cell->pivot.X ,dispCell->cell->pivot.Y);

		pivot.X = pivot.X * dispCell->cell->size.X;
		pivot.Y = pivot.Y * dispCell->cell->size.Y;

		FVector2D dispscale = dispCell->cell->size;


		SsCurrentRenderer::getRender()->renderSpriteSimple(
			matrix,
			dispscale.X , dispscale.Y ,  pivot,
					dispCell->uvs[0],
					dispCell->uvs[3], fcolor );
	}	


}
*/
/*
void SsEffectRenderV2::particleDraw(SsEffectEmitter* e , double time , SsEffectEmitter* parent , particleDrawData* plp )
{
	double t = time;

	if (e == 0) return;

	int pnum = e->getParticleIDMax();

	int slide = (parent == 0) ? 0 : plp->id;

	e->updateEmitter( time, slide ); 


	for (auto id = 0; id < pnum; id++)
	{
		const particleExistSt* drawe = e->getParticleDataFromID(id);

        if ( !drawe->born )continue;

		float targettime = (t + 0.0f);
		particleDrawData lp;
		particleDrawData pp;
		pp.X = 0; pp.Y = 0;

		lp.id = id + drawe->cycle;
		lp.stime = drawe->stime;
		lp.lifetime = drawe->endtime;
		lp.pid = 0;

		if ( parent )lp.pid = plp->id;

		//if ( lp.stime == lp.lifetime ) continue;

		//if ( lp.stime <= targettime && lp.lifetime >= targettime)
		if ( drawe->exist )
		{

			if (parent)
			{
				//親から描画するパーティクルの初期位置を調べる
				pp.id = plp->id;
				pp.stime = plp->stime;
				pp.lifetime = plp->lifetime;
				pp.pid = plp->pid;
				//パーティクルが発生した時間の親の位置を取る

				int ptime = lp.stime + pp.stime;
				if ( ptime > lp.lifetime ) ptime = lp.lifetime;

				//逆算はデバッグしずらいかもしれない
				parent->updateParticle( lp.stime + pp.stime , &pp);
				e->position.X = pp.X;
				e->position.Y = pp.Y;

			}


			e->updateParticle(targettime, &lp);


			SsFColor fcolor;
			fcolor.fromARGB(lp.color.toARGB());

			drawSprite( &e->dispCell ,
						FVector2D(lp.X,lp.Y),
						lp.scale,
						lp.rot , lp.direc , fcolor , e->refData->BlendType );


		}

	}



}
*/


//パラメータをコピーする
void	SsEffectRenderV2::initEmitter( SsEffectEmitter* e , FSsEffectNode* node)
{

	e->refData = node->GetMyBehavior();
	e->refCell = e->refData->RefCell;

	//セルの初期化
	SsCelMapLinker* link = this->curCellMapManager->getCellMapLink( e->refData->CellMapName );

	if ( link )
	{
		FSsCell * cell = link->findCell( e->refData->CellName );
		
		getCellValue(	this->curCellMapManager , 
			e->refData->CellMapName ,
			e->refData->CellName , 
			e->dispCell ); 
	}else{
		UE_LOG(LogSpriteStudio, Warning, TEXT("cell not found : %s , %s\n") , 
			*(e->refData->CellMapName.ToString()), 
			*(e->refData->CellName.ToString())
			);
	}

	SsEffectFunctionExecuter::initializeEffect( e->refData , e );

	e->emitterSeed = this->mySeed;

	if ( e->particle.userOverrideRSeed )
	{
		e->emitterSeed = e->particle.overrideRSeed;

	}else{
		if ( this->effectData->IsLockRandSeed )
		{
			e->emitterSeed = (this->effectData->LockRandSeed+1) * SEED_MAGIC;
		}
	}

	e->emitter.life+= e->particle.delay;//ディレイ分加算
}


void	SsEffectRenderV2::clearEmitterList()
{
	for ( int i = 0 ; i < this->emmiterList.Num(); i++)
	{
		delete emmiterList[i];
	}

    emmiterList.Empty();
	updateList.Empty();

}



void	SsEffectRenderV2::setEffectData(FSsEffectModel* data)
{
	effectData = data;

    reload();

}


void	SsEffectRenderV2::update()
{

	if ( !m_isPlay ) return;

	targetFrame = nowFrame;

	if ( !this->Infinite )
	{
		if ( this->isloop() ) //自動ループの場合
		{
			if ( nowFrame > getEffectTimeLength() )
			{
				targetFrame = (int)((int)nowFrame % getEffectTimeLength());
				int l = ( nowFrame / getEffectTimeLength() );
				setSeedOffset( l );
			}
		}
	}

}

/*
void	SsEffectRenderV2::draw()
{
    if ( nowFrame < 0 ) return;

	for ( size_t i = 0 ; i < updateList.Num() ; i++ )
	{
		SsEffectEmitter* e = updateList[i];
		if ( e )
		{
			e->setSeedOffset( seedOffset ); 
		}
	}

	for ( size_t i = 0 ; i < updateList.Num() ; i++ )
	{
		SsEffectEmitter* e = updateList[i];

		if ( e->_parent )
		{
			//グローバルの時間で現在親がどれだけ生成されているのかをチェックする
			e->_parent->updateEmitter(targetFrame , 0);

			int loopnum =  e->_parent->getParticleIDMax();
			for ( int n = 0 ; n < loopnum ; n ++ )
			{
				const particleExistSt* drawe = e->_parent->getParticleDataFromID(n);

				if ( drawe->born )
				{
					particleDrawData lp;
					lp.stime = drawe->stime;
					lp.lifetime = drawe->endtime;
					lp.id = n;
					lp.pid = 0;

					float targettime = (targetFrame + 0.0f);
					float ptime = (targettime - lp.stime );

	  				particleDraw( e , ptime , e->_parent , &lp);
				}
			}

		}else{
			particleDraw( e , targetFrame );
		}
	}

}
*/

bool compare_priority( SsEffectEmitter* left,  SsEffectEmitter* right)
{
	if ( left->priority == right->priority )
	{
		if ( left->uid < right->uid ) return true;
	}
  //	return true;
  return left->priority < right->priority ;
}


void    SsEffectRenderV2::reload()
{
	nowFrame = 0;

    //updateが必要か
	stop();
	clearEmitterList();

	FSsEffectNode* root = this->effectData->Root;

    //this->effectData->updateNodeList();//ツールじゃないので要らない
    TArray<FSsEffectNode>& list = this->effectData->NodeList;

	layoutScale.X = (float)(this->effectData->LayoutScaleX) / 100.0f;
	layoutScale.Y = (float)(this->effectData->LayoutScaleY) / 100.0f;

	int* cnum = new int[list.Num()];
    memset( cnum , 0 , sizeof(int) * list.Num() );

	bool _Infinite = false;
	//パラメータを取得
	//以前のデータ形式から変換
	for ( int i = 0 ; i < list.Num() ; i ++ )
	{
		FSsEffectNode *node =  &(list[i]);

		if ( node->GetType() == SsEffectNodeType::Emmiter )
		{
			SsEffectEmitter* e = new SsEffectEmitter();
			//パラメータをコピー

			e->_parentIndex = node->ParentIndex;
			//繋ぎ先は恐らくパーティクルなのでエミッタに変換
			if ( e->_parentIndex != 0 )
			{
				e->_parentIndex = list[e->_parentIndex].ParentIndex;

			}

			cnum[e->_parentIndex]++;
			if ( cnum[e->_parentIndex] > 10 )
			{
				_isWarningData = true;
				continue; //子１０ノード表示制限
			}

			//孫抑制対策
			if ( e->_parentIndex != 0 )
			{
				int a = list[e->_parentIndex].ParentIndex;
				if ( a != 0 )
				{
				   if ( list[a].ParentIndex > 0 ) {
						_isWarningData = true;
						continue;
				   }
				}
			}

			initEmitter( e , node );
			this->emmiterList.Add(e);
			if ( e->emitter.Infinite ) _Infinite = true;
		}else
		{
            //エミッター同士を繋ぎたいので
			this->emmiterList.Add(0);
		}
	}

	delete[] cnum;
	Infinite = _Infinite;


    //親子関係整理


	effectTimeLength = 0;
	//事前計算計算  updateListにルートの子を配置し親子関係を結ぶ
	for ( int i = 0 ; i < this->emmiterList.Num(); i++)
	{
		if (emmiterList[i] != 0 )
		{
			emmiterList[i]->uid = i;
			//emmiterList[i]->precalculate();
			emmiterList[i]->precalculate2(); //ループ対応形式


			int  pi =  emmiterList[i]->_parentIndex;

			if ( emmiterList[i]->_parentIndex == 0 )  //ルート直下
			{
				emmiterList[i]->_parent = 0;
				emmiterList[i]->globaltime = emmiterList[i]->getTimeLength();
				updateList.Add(emmiterList[i]);
			}else{

				void* t = this->emmiterList[pi];

                emmiterList[i]->_parent = emmiterList[pi];

				emmiterList[i]->globaltime = emmiterList[i]->getTimeLength() + this->emmiterList[pi]->getTimeLength();

				updateList.Add(emmiterList[i]);
			}

			if ( emmiterList[i]->globaltime > effectTimeLength )
			{
				effectTimeLength = emmiterList[i]->globaltime;
			}
		}
	}
	//プライオリティソート
	updateList.Sort();


}


size_t  SsEffectRenderV2::getEffectTimeLength()
{

	return effectTimeLength;
}


int	SsEffectRenderV2::getCurrentFPS(){
	if (effectData)
	{
		if ( effectData->FPS == 0 ) return 30;

		return effectData->FPS;
	}
	return 30;
}
