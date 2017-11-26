#include "SpriteStudio6PrivatePCH.h"

//#include <stdio.h>
//#include <cstdlib>

//#include "../Loader/ssloader.h"
#include "ssplayer_animedecode.h"
#include "ssplayer_effect.h"
#include "ssplayer_macro.h"
#include "ssplayer_matrix.h"
//#include "ssplayer_render.h"
#include "ssplayer_effectfunction.h"

#include "SsEffectElement.h"



/*
//二つの値の範囲から値をランダムで得る
static uint8 GetRandamNumberRange( SsEffectRenderEmitter* e , uint8 a , uint8 b )
{
	uint8 min = a < b ? a : b;
	uint8 max = a < b ? b : a;

	uint8 diff = ( max - min );


    if ( diff == 0 ) return min;
	return min + (e->MT->genrand_uint32() % diff);

}

static void VarianceCalcColor( SsEffectRenderEmitter* e ,  FSsU8Color& out , FSsU8Color  color1 , FSsU8Color color2 )
{

	out.R = GetRandamNumberRange( e , color1.R , color2.R );
	out.G = GetRandamNumberRange( e ,  color1.G , color2.G );
	out.B = GetRandamNumberRange( e ,  color1.B , color2.B );
	out.A = GetRandamNumberRange( e ,  color1.A , color2.A );

}


float frand(unsigned v) {
    unsigned res = (v>>9) | 0x3f800000;
    return (*(float*)&res) - 1.0f;
}

static float VarianceCalc( SsEffectRenderEmitter* e ,  float base , float variance )
{

	unsigned long r = e->MT->genrand_uint32();

	float len = variance - base;

    return base + len * frand( r );


}

static float VarianceCalcFin( SsEffectRenderEmitter* e ,  float base , float variance )
{
	unsigned long r = e->MT->genrand_uint32();

	return base + (-variance + variance* ( frand(r) * 2.0f ));

}


static uint8 blendNumber( uint8 a , uint8 b , float rate )
{
	return ( a + ( b - a ) * rate );
}


static float blendFloat( float a,float b , float rate )
{
	return   ( a + ( b - a ) * rate );
}
*/


class EffectFuncBase
{
public:
	EffectFuncBase(){}
	virtual ~EffectFuncBase(){}
/*
	virtual void	initalizeEmmiter ( FSsEffectElementBase* ele , SsEffectRenderEmitter* emmiter){}
	virtual void	updateEmmiter( FSsEffectElementBase* ele , SsEffectRenderEmitter* emmiter){}
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle ){}
	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle ){}
*/

	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* emmiter){}
};




//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementBasic : public EffectFuncBase
{
public:
	FuncFSsParticleElementBasic(){}
	virtual ~FuncFSsParticleElementBasic(){}
/*
	virtual void	initalizeEmmiter ( FSsEffectElementBase* ele , SsEffectRenderEmitter* e)
	{
		FSsParticleElementBasic* source = static_cast<FSsParticleElementBasic*>(ele);

		e->maxParticle = source->MaximumParticle;
		e->interval = source->Interval;
		e->_lifetime = source->Lifetime;
		e->_life = source->Lifetime;
		e->burst = source->AttimeCreate;

		e->undead = false;
		e->drawPriority = source->Priority;

		if ( e->_lifetime == 0 ) e->undead = true;
		
	
	}

	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementBasic* source = static_cast<FSsParticleElementBasic*>(ele);
		FVector eVec = e->getPosition();
		float eAngle = 0;

		p->_baseEmiterPosition.X = eVec.X;
		p->_baseEmiterPosition.Y = eVec.Y;
		p->_position.X = p->_baseEmiterPosition.X;
		p->_position.Y = p->_baseEmiterPosition.Y;
		p->_size = FVector2D( 1.0f , 1.0f );


		p->_color = FSsU8Color(255,255,255,255) ;
		p->_startcolor = FSsU8Color(255,255,255,255) ;
		p->_endcolor = p->_startcolor;


		p->_backposition = p->_position;

		p->_lifetime   = VarianceCalc( e , source->Lifespan.GetMinValue() , source->Lifespan.GetMaxValue() );
		p->_life = source->Lifetime;
		float temp_angle = VarianceCalcFin( e ,  source->Angle+eAngle , source->AngleVariance/2.0f);

		float angle_rad = DegreeToRadian( (temp_angle+90.0f) );
		float lspeed = VarianceCalc(  e , source->Speed.GetMinValue() , source->Speed.GetMaxValue() );

		p->speed = lspeed;
		p->firstspeed = lspeed;
		p->vector.X =  cos( angle_rad );
		p->vector.Y =  sin( angle_rad );

		p->_force = FVector2D(0,0);//p->vector * p->speed;
		p->direction = 0;
		p->isTurnDirection = false;

		p->_rotation = 0;
		p->_rotationAdd = 0;
		p->_rotationAddDst = 0;
		p->_rotationAddOrg = 0;

		p->_rotation = 0;
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementBasic* source = static_cast<FSsParticleElementBasic*>(ele);
		e->priority = source->Priority;

		//エミッターパラメータ
		e->emitter.emitmax = source->MaximumParticle;
		e->emitter.interval = source->Interval;
		e->emitter.life = source->Lifetime;
		e->emitter.emitnum = source->AttimeCreate;
		e->emitter.particleLife = 10;//
		e->emitter.Infinite = false;
		e->emitter.loopGen = 0;


		//パーティクルパラメータ
		e->emitter.particleLife = source->Lifespan.GetMinValue();
		e->emitter.particleLife2 = source->Lifespan.GetMaxValue() - source->Lifespan.GetMinValue();

		e->particle.scale = FVector2D( 1.0f , 1.0f );
		e->particle.startcolor = FSsU8Color(255,255,255,255) ;
		e->particle.endcolor = FSsU8Color(255,255,255,255) ;

		e->particle.speed = source->Speed.GetMinValue();
		e->particle.speed2 = source->Speed.GetMaxValue() - source->Speed.GetMinValue();

		e->particle.angle = DegreeToRadian( (source->Angle+90.0f) );
		e->particle.angleVariance = DegreeToRadian( source->AngleVariance );

		e->particle.useTanAccel = false;

		//重力
		e->particle.useGravity = false;
		e->particle.gravity = FVector2D(0,0);

		//オフセット
		e->particle.useOffset = false;
		e->particle.offset = FVector2D(0,0);
		e->particle.offset2 = FVector2D(0,0);


		//回転
		e->particle.useRotation = false;
		e->particle.useRotationTrans = false;

		//カラー
		e->particle.useColor = false;
		e->particle.useTransColor = false;

		//スケール
		e->particle.useInitScale = false;
 		e->particle.useTransScale = false;

		e->particle.delay = 0;

		e->particle.usePGravity = false;

		e->particle.useTransColor = false;
		e->particle.useInitScale = false;
		e->particle.usePGravity = false;
		e->particle.useAlphaFade = false;
		e->particle.useTransSpeed = false;
		e->particle.useTurnDirec = false;
		e->particle.userOverrideRSeed = false;
	}

};
static FuncFSsParticleElementBasic		funcBasic;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementRndSeedChange : public EffectFuncBase
{
public:
	FuncFSsParticleElementRndSeedChange(){}
	virtual ~FuncFSsParticleElementRndSeedChange(){}
/*
	virtual void	initalizeEmmiter ( FSsEffectElementBase* ele , SsEffectRenderEmitter* emmiter)	
	{
		FSsParticleElementRndSeedChange* source = static_cast<FSsParticleElementRndSeedChange*>(ele);
		emmiter->setMySeed( source->Seed );
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementRndSeedChange* source = static_cast<FSsParticleElementRndSeedChange*>(ele);
		e->particle.userOverrideRSeed = true;

		e->particle.overrideRSeed  =  source->Seed + SEED_MAGIC;
		e->emitterSeed = source->Seed + SEED_MAGIC;
	}
	
};
static FuncFSsParticleElementRndSeedChange		funcRndSeedChange;


//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementDelay : public EffectFuncBase
{
public:
	FuncFSsParticleElementDelay(){}
	virtual ~FuncFSsParticleElementDelay(){}
/*
	virtual void	initalizeEmmiter ( FSsEffectElementBase* ele , SsEffectRenderEmitter* emmiter)
	{
		FSsParticleElementDelay* source = static_cast<FSsParticleElementDelay*>(ele);
		emmiter->delay = source->DelayTime;
		emmiter->_lifetime = emmiter->_lifetime + source->DelayTime;
		emmiter->_life = emmiter->_lifetime;
		emmiter->generate_ok = false;	
	}

	virtual void	updateEmmiter( FSsEffectElementBase* ele , SsEffectRenderEmitter* emmiter)
	{
		FSsParticleElementDelay* source = static_cast<FSsParticleElementDelay*>(ele);
		//既定の時間までストップ？
		if ( emmiter->_exsitTime >= source->DelayTime )
		{
			emmiter->generate_ok = true;
		}	
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementDelay* source = static_cast<FSsParticleElementDelay*>(ele);
		e->particle.delay = source->DelayTime;

	}
	
};
static FuncFSsParticleElementDelay		funcDelay;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementGravity : public EffectFuncBase
{
public:
	FuncFSsParticleElementGravity(){}
	virtual ~FuncFSsParticleElementGravity(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementGravity* source = static_cast<FSsParticleElementGravity*>(ele);
		p->_gravity = source->Gravity;
	}
	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle )
	{
		FSsParticleElementGravity* source = static_cast<FSsParticleElementGravity*>(ele);
		particle->_gravity = source->Gravity * particle->_exsitTime;	
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementGravity* source = static_cast<FSsParticleElementGravity*>(ele);
		e->particle.useGravity = true;
		e->particle.gravity = source->Gravity;

	}
};
static FuncFSsParticleElementGravity		funcGravity;


//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementPosition : public EffectFuncBase
{
public:
	FuncFSsParticleElementPosition(){}
	virtual ~FuncFSsParticleElementPosition(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementPosition* source = static_cast<FSsParticleElementPosition*>(ele);
		p->_position.X = p->_baseEmiterPosition.X + VarianceCalc( e , source->OffsetX.GetMinValue() , source->OffsetX.GetMaxValue() );
		p->_position.Y = p->_baseEmiterPosition.Y + VarianceCalc( e , source->OffsetY.GetMinValue() , source->OffsetY.GetMaxValue() );
	}

	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle ){}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementPosition* source = static_cast<FSsParticleElementPosition*>(ele);
		e->particle.useOffset = true;
		e->particle.offset = FVector2D(source->OffsetX.GetMinValue(), source->OffsetY.GetMinValue());
		e->particle.offset2 = FVector2D( source->OffsetX.GetMaxValue() - source->OffsetX.GetMinValue() , source->OffsetY.GetMaxValue() - source->OffsetY.GetMinValue() );
	}
};
static FuncFSsParticleElementPosition		funcPosition;


#if 0		//オミット
//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementTransPosition: public EffectFuncBase
{
public:
	FuncFSsParticleElementTransPosition(){}
	virtual ~FuncFSsParticleElementTransPosition(){}
	
};
static FuncFSsParticleElementPosition		funcTransPosition;
#endif


//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementRotation: public EffectFuncBase
{
public:
	FuncFSsParticleElementRotation(){}
	virtual ~FuncFSsParticleElementRotation(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementRotation* source = static_cast<FSsParticleElementRotation*>(ele);

		p->_rotation = VarianceCalc( e , source->Rotation.GetMinValue() , source->Rotation.GetMaxValue() ) ;
		p->_rotationAdd =  VarianceCalc( e , source->RotationAdd.GetMinValue() , source->RotationAdd.GetMaxValue() );
		p->_rotationAddDst = p->_rotationAdd;
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementRotation* source = static_cast<FSsParticleElementRotation*>(ele);
		e->particle.useRotation = true;
		e->particle.rotation = source->Rotation.GetMinValue();
		e->particle.rotation2 = source->Rotation.GetMaxValue() - source->Rotation.GetMinValue();

		e->particle.rotationAdd = source->RotationAdd.GetMinValue();
		e->particle.rotationAdd2 = source->RotationAdd.GetMaxValue() - source->RotationAdd.GetMinValue();

	}
};
static FuncFSsParticleElementRotation		funcRotation ;



//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementRotationTrans : public EffectFuncBase
{
public:
	FuncFSsParticleElementRotationTrans(){}
	virtual ~FuncFSsParticleElementRotationTrans(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementRotationTrans* source = static_cast<FSsParticleElementRotationTrans*>(ele);
		if ( p->_lifetime == 0 ) return ;
		if ( source->EndLifeTimePer == 0 )
		{
			p->_rotationAddDst = p->_rotationAdd * source->RotationFactor;
			p->_rotationAddOrg = p->_rotationAdd ;
			return;
		}
		p->_rotationAddDst = p->_rotationAdd * source->RotationFactor;
		p->_rotationAddOrg = p->_rotationAdd;
	}
	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementRotationTrans* source = static_cast<FSsParticleElementRotationTrans*>(ele);

		if ( (p->_lifetime*source->EndLifeTimePer) == 0 )
		{
			p->_rotationAdd = blendFloat( p->_rotationAddOrg  , p->_rotationAddDst , 1.0f );
			return;
		}
		float per = ( (float)p->_exsitTime / ((float)p->_lifetime*( source->EndLifeTimePer / 100.0f))  );// * 100.0f;

		if ( per > 1.0f ) per = 1.0f;

		p->_rotationAdd = blendFloat( p->_rotationAddOrg  , p->_rotationAddDst , per );
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementRotationTrans* source = static_cast<FSsParticleElementRotationTrans*>(ele);
		e->particle.useRotationTrans = true;
		e->particle.rotationFactor = source->RotationFactor;
		e->particle.endLifeTimePer = source->EndLifeTimePer / 100.0f;
	}
};
static FuncFSsParticleElementRotationTrans		funcRotationTrans ;


//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementTransSpeed : public EffectFuncBase
{
public:
	FuncFSsParticleElementTransSpeed(){}
	virtual ~FuncFSsParticleElementTransSpeed(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementTransSpeed* source = static_cast<FSsParticleElementTransSpeed*>(ele);
		p->lastspeed = VarianceCalc( e, source->Speed.GetMinValue() , source->Speed.GetMaxValue() );
	}

	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		//FSsParticleElementTransSpeed* source = static_cast<FSsParticleElementTransSpeed*>(ele);
		float per = ( (float)p->_exsitTime / (float)p->_lifetime );
		p->speed = ( p->firstspeed + ( p->lastspeed - p->firstspeed ) * per );
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementTransSpeed* source = static_cast<FSsParticleElementTransSpeed*>(ele);
		e->particle.useTransSpeed = true;
		e->particle.transSpeed = source->Speed.GetMinValue();
		e->particle.transSpeed2 = source->Speed.GetMaxValue() - source->Speed.GetMinValue();
	}

};
static FuncFSsParticleElementTransSpeed		funcTransSpeed ;


//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementTangentialAcceleration : public EffectFuncBase
{
public:
	FuncFSsParticleElementTangentialAcceleration(){}
	virtual ~FuncFSsParticleElementTangentialAcceleration(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementTangentialAcceleration* source = static_cast<FSsParticleElementTangentialAcceleration*>(ele);
		p->_tangentialAccel = VarianceCalc( e , source->Acceleration.GetMinValue() , source->Acceleration.GetMaxValue() );
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementTangentialAcceleration* source = static_cast<FSsParticleElementTangentialAcceleration*>(ele);
		e->particle.useTanAccel = true;
		e->particle.tangentialAccel = source->Acceleration.GetMinValue();
		e->particle.tangentialAccel2 = ( source->Acceleration.GetMaxValue() - source->Acceleration.GetMinValue());

	}
};
static FuncFSsParticleElementTangentialAcceleration		funcTangentialAcceleration ;

static void getRange( uint8 a , uint8 b , uint8& min, uint8& diff)
{
	min = a < b ? a : b;
	uint8 max = a < b ? b : a;

	diff = ( max - min );
}

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementInitColor : public EffectFuncBase
{
public:
	FuncFSsParticleElementInitColor(){}
	virtual ~FuncFSsParticleElementInitColor(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementInitColor* source = static_cast<FSsParticleElementInitColor*>(ele);
		VarianceCalcColor( e , p->_startcolor , source->Color.GetMinValue() , source->Color.GetMaxValue() );
		p->_color = p->_startcolor;
	
	}
*/

	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementInitColor* source = static_cast<FSsParticleElementInitColor*>(ele);
		e->particle.useColor = true;

		FSsU8Color color1 = source->Color.GetMinValue();
		FSsU8Color color2 = source->Color.GetMaxValue();

		getRange( color1.A , color2.A , e->particle.initColor.A , e->particle.initColor2.A );
		getRange( color1.R , color2.R , e->particle.initColor.R , e->particle.initColor2.R );
		getRange( color1.G , color2.G , e->particle.initColor.G , e->particle.initColor2.G );
		getRange( color1.B , color2.B , e->particle.initColor.B , e->particle.initColor2.B );

	}
};
static FuncFSsParticleElementInitColor		funcInitColor;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementTransColor : public EffectFuncBase
{
public:
	FuncFSsParticleElementTransColor(){}
	virtual ~FuncFSsParticleElementTransColor(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementTransColor* source = static_cast<FSsParticleElementTransColor*>(ele);
		VarianceCalcColor( e , p->_endcolor , source->Color.GetMinValue() , source->Color.GetMaxValue() );
	}

	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		float per = ( (float)p->_exsitTime / (float)p->_lifetime );

		if ( per > 1.0f )per = 1.0f;

		p->_color.A = blendNumber( p->_startcolor.A , p->_endcolor.A , per );
		p->_color.R = blendNumber( p->_startcolor.R , p->_endcolor.R , per  );
		p->_color.G = blendNumber( p->_startcolor.G , p->_endcolor.G , per  );
		p->_color.B = blendNumber( p->_startcolor.B , p->_endcolor.B , per  );
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementTransColor* source = static_cast<FSsParticleElementTransColor*>(ele);

		e->particle.useTransColor = true;

		FSsU8Color color1 = source->Color.GetMinValue();
		FSsU8Color color2 = source->Color.GetMaxValue();

		getRange( color1.A , color2.A , e->particle.transColor.A , e->particle.transColor2.A );
		getRange( color1.R , color2.R , e->particle.transColor.R , e->particle.transColor2.R );
		getRange( color1.G , color2.G , e->particle.transColor.G , e->particle.transColor2.G );
		getRange( color1.B , color2.B , e->particle.transColor.B , e->particle.transColor2.B );
	}
};
static FuncFSsParticleElementTransColor		funcTransColor;


//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementAlphaFade : public EffectFuncBase
{
public:
	FuncFSsParticleElementAlphaFade(){}
	virtual ~FuncFSsParticleElementAlphaFade(){}
/*
	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle )
	{
		FSsParticleElementAlphaFade* source = static_cast<FSsParticleElementAlphaFade*>(ele);
	
		if ( particle->_lifetime == 0 ) return ;

		float per = ( (float)particle->_exsitTime / (float)particle->_lifetime ) * 100.0f;

		float start = source->Disprange.GetMinValue();
		float end = source->Disprange.GetMaxValue();

		if ( per < start )
		{
			float alpha = (start - per) / start;
			particle->_color.A*= 1.0f - alpha;
			return;
		}

		if ( per > end )
		{

			if (end>=100.0f)
			{
				particle->_color.A = 0;
				return;
			}
			float alpha = (per-end) / (100.0f-end);
			particle->_color.A*= 1.0f - alpha;
			return;
		}

	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementAlphaFade* source = static_cast<FSsParticleElementAlphaFade*>(ele);
		e->particle.useAlphaFade = true;
		e->particle.alphaFade = source->Disprange.GetMinValue();
		e->particle.alphaFade2 = source->Disprange.GetMaxValue();

	}

};
static FuncFSsParticleElementAlphaFade		funcAlphaFade;



//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementSize : public EffectFuncBase
{
public:
	FuncFSsParticleElementSize(){}
	virtual ~FuncFSsParticleElementSize(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{

		FSsParticleElementSize* source = static_cast<FSsParticleElementSize*>(ele);

		p->_size.X = VarianceCalc( e, source->SizeX.GetMinValue() , source->SizeX.GetMaxValue() );
		p->_size.Y = VarianceCalc( e, source->SizeY.GetMinValue() , source->SizeY.GetMaxValue() );
		float sf   = VarianceCalc( e, source->ScaleFactor.GetMinValue() , source->ScaleFactor.GetMaxValue() );

		p->_size = p->_size * sf;
		p->_startsize = p->_size;	
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementSize* source = static_cast<FSsParticleElementSize*>(ele);

		e->particle.useInitScale = true;

		e->particle.scale.X = source->SizeX.GetMinValue();
		e->particle.scaleRange.X = source->SizeX.GetMaxValue() - source->SizeX.GetMinValue();

		e->particle.scale.Y = source->SizeY.GetMinValue();
		e->particle.scaleRange.Y = source->SizeY.GetMaxValue() - source->SizeY.GetMinValue();

		e->particle.scaleFactor = source->ScaleFactor.GetMinValue();
		e->particle.scaleFactor2 = source->ScaleFactor.GetMaxValue() - source->ScaleFactor.GetMinValue();
	}

};
static FuncFSsParticleElementSize		funcSize;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncFSsParticleElementTransSize : public EffectFuncBase
{
public:
	FuncFSsParticleElementTransSize(){}
	virtual ~FuncFSsParticleElementTransSize(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticleElementTransSize* source = static_cast<FSsParticleElementTransSize*>(ele);
		FVector2D endsize;
		endsize.X = VarianceCalc( e, source->SizeX.GetMinValue() , source->SizeX.GetMaxValue() );
		endsize.Y = VarianceCalc( e, source->SizeY.GetMinValue() , source->SizeY.GetMaxValue() );

		float sf = VarianceCalc( e, source->ScaleFactor.GetMinValue() , source->ScaleFactor.GetMaxValue() );

		endsize = endsize * sf;

		p->_divsize = ( endsize - p->_startsize ) / p->_lifetime;	
	}
	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
	
		p->_size = p->_startsize + ( p->_divsize * ( p->_exsitTime ) );
	
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleElementTransSize* source = static_cast<FSsParticleElementTransSize*>(ele);
		e->particle.useTransScale = true;

		e->particle.transscale.X = source->SizeX.GetMinValue();
		e->particle.transscaleRange.X = source->SizeX.GetMaxValue() - source->SizeX.GetMinValue();

		e->particle.transscale.Y = source->SizeY.GetMinValue();
		e->particle.transscaleRange.Y = source->SizeY.GetMaxValue() - source->SizeY.GetMinValue();

		e->particle.transscaleFactor = source->ScaleFactor.GetMinValue();
		e->particle.transscaleFactor2 = source->ScaleFactor.GetMaxValue() - source->ScaleFactor.GetMinValue();

	}
};
static FuncFSsParticleElementTransSize		funcTransSize;


//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncParticlePointGravity : public EffectFuncBase
{
public:
	FuncParticlePointGravity(){}
	virtual ~FuncParticlePointGravity(){}
/*
	virtual void	initalizeEmmiter ( FSsEffectElementBase* ele , SsEffectRenderEmitter* emmiter){}
	virtual void	updateEmmiter( FSsEffectElementBase* ele , SsEffectRenderEmitter* emmiter){}
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		//p->_orggravity = p->_gravity;	
	}
	virtual void	updateParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* p )
	{
		FSsParticlePointGravity* source = static_cast<FSsParticlePointGravity*>(ele);

		FVector2D Target;
		Target.X = source->Position.X + p->parentEmitter->position.X;
		Target.Y = source->Position.Y + p->parentEmitter->position.Y;

		//現在地点から指定された点に対してのベクトル*パワーを与える
		FVector2D v2 = Target - p->_position;
		FVector2D v2_temp = v2;

		v2.Normalize();
		v2 = v2 * source->Power;

		p->_gravity = p->_gravity + v2;

	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticlePointGravity* source = static_cast<FSsParticlePointGravity*>(ele);
	   e->particle.usePGravity = true;
	   e->particle.gravityPos = source->Position;
//	   e->particle.gravityPower = source->Power / 100.0f;
	   e->particle.gravityPower = source->Power;

	}
};
static FuncParticlePointGravity		funcPointGravity;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncParticleTurnToDirectionEnabled : public EffectFuncBase
{
public:
	FuncParticleTurnToDirectionEnabled(){}
	virtual ~FuncParticleTurnToDirectionEnabled(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle )
	{
		particle->isTurnDirection = true;
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		FSsParticleTurnToDirectionEnabled* source = static_cast<FSsParticleTurnToDirectionEnabled*>(ele);
		e->particle.useTurnDirec = true;
		e->particle.direcRotAdd = source->Rotation;
	}
};
static FuncParticleTurnToDirectionEnabled		funcTurnToDirectionEnabled;



//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FuncParticleInfiniteEmitEnabled : public EffectFuncBase
{
public:
	FuncParticleInfiniteEmitEnabled(){}
	virtual ~FuncParticleInfiniteEmitEnabled(){}
/*
	virtual void	initializeParticle( FSsEffectElementBase* ele , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle )
	{
	}
*/
	virtual void	initalizeEffect ( FSsEffectElementBase* ele , SsEffectEmitter* e)
	{
		e->emitter.Infinite = true;
	}
};
static FuncParticleInfiniteEmitEnabled		funcParticleInfiniteEmitEnabled;


//-------------------------------------------------------------------
//挙動反映クラスの呼び出しテーブル
//SsEffectFunctionTypeの順に並べること
//-------------------------------------------------------------------
static EffectFuncBase* callTable[] =
{
	0,
	&funcBasic,
	&funcRndSeedChange,
	&funcDelay,
	&funcGravity,
	&funcPosition,
	//&funcTransPosition,
	&funcRotation,
	&funcRotationTrans,
	&funcTransSpeed,
	&funcTangentialAcceleration,
	&funcInitColor,
	&funcTransColor,
	&funcAlphaFade,
	&funcSize,
	&funcTransSize,
	&funcPointGravity,
	&funcTurnToDirectionEnabled,
	&funcParticleInfiniteEmitEnabled,
};






///----------------------------------------------------------------------------------------------------
//
///----------------------------------------------------------------------------------------------------
void	SsEffectFunctionExecuter::initializeEffect( FSsEffectBehavior* beh ,  SsEffectEmitter* emmiter)
{
	for(auto e = beh->PList.CreateIterator(); e; ++e)
	{
		EffectFuncBase* cf = callTable[(*e)->MyType];
		cf->initalizeEffect( (*e).Get() , emmiter );
	}
}
