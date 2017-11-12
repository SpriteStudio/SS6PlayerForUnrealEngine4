#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerEffectFunction.h"

#include "SsPlayerAnimedecode.h"
#include "SsEffectElement.h"
#include "SsEffectBehavior.h"


namespace
{
	static void GetRange(uint8 a, uint8 b, uint8& OutMin, uint8& OutDiff)
	{
		OutMin = a < b ? a : b;
		uint8 TmpMax = a < b ? b : a;
		OutDiff = (TmpMax - OutMin);
	}
}



class FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e){}
};

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementBasic : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementBasic* source = static_cast<FSsParticleElementBasic*>(ele);
		e->Priority = source->Priority;

		//エミッターパラメータ
		e->Emitter.Emitmax = source->MaximumParticle;
		e->Emitter.Interval = source->Interval;
		e->Emitter.Life = source->Lifetime;
		e->Emitter.Emitnum = source->AttimeCreate;
		e->Emitter.ParticleLife = 10;//
		e->Emitter.bInfinite = false;
		e->Emitter.LoopGen = 0;


		//パーティクルパラメータ
		e->Emitter.ParticleLife = source->Lifespan.GetMinValue();
		e->Emitter.ParticleLife2 = source->Lifespan.GetMaxValue() - source->Lifespan.GetMinValue();

		e->Particle.Scale = FVector2D(1.0f, 1.0f);
		e->Particle.StartColor = FSsU8Color(255, 255, 255, 255);
		e->Particle.EndColor = FSsU8Color(255, 255, 255, 255);

		e->Particle.Speed = source->Speed.GetMinValue();
		e->Particle.Speed2 = source->Speed.GetMaxValue() - source->Speed.GetMinValue();

		e->Particle.Angle = FMath::DegreesToRadians((source->Angle + 90.0f));
		e->Particle.AngleVariance = FMath::DegreesToRadians(source->AngleVariance);

		e->Particle.bUseTanAccel = false;

		//重力
		e->Particle.bUseGravity = false;
		e->Particle.Gravity = FVector2D(0, 0);

		//オフセット
		e->Particle.bUseOffset = false;
		e->Particle.Offset = FVector2D(0, 0);
		e->Particle.Offset2 = FVector2D(0, 0);


		//回転
		e->Particle.bUseRotation = false;
		e->Particle.bUseRotationTrans = false;

		//カラー
		e->Particle.bUseColor = false;

		//スケール
		e->Particle.bUseTransScale = false;

		e->Particle.Delay = 0;

		e->Particle.bUseTransColor = false;
		e->Particle.bUseInitScale = false;
		e->Particle.bUsePGravity = false;
		e->Particle.bUseAlphaFade = false;
		e->Particle.bUseTransSpeed = false;
		e->Particle.bUseTurnDirec = false;
		e->Particle.bUserOverrideRSeed = false;
	}
};
static FFuncParticleElementBasic FuncBasic;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementRndSeedChange : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementRndSeedChange* source = static_cast<FSsParticleElementRndSeedChange*>(ele);
		e->Particle.bUserOverrideRSeed = true;

		e->Particle.OverrideRSeed = source->Seed + SS_EFFECT_SEED_MAGIC;
		e->EmitterSeed = source->Seed + SS_EFFECT_SEED_MAGIC;
	}
};
static FFuncParticleElementRndSeedChange FuncRndSeedChange;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementDelay : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementDelay* source = static_cast<FSsParticleElementDelay*>(ele);
		e->Particle.Delay = source->DelayTime;
	}
};
static FFuncParticleElementDelay FuncDelay;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementGravity : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementGravity* source = static_cast<FSsParticleElementGravity*>(ele);
		e->Particle.bUseGravity = true;
		e->Particle.Gravity = source->Gravity;
	}
};
static FFuncParticleElementGravity FuncGravity;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementPosition : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementPosition* source = static_cast<FSsParticleElementPosition*>(ele);
		e->Particle.bUseOffset = true;
		e->Particle.Offset = FVector2D(source->OffsetX.GetMinValue(), source->OffsetY.GetMinValue());
		e->Particle.Offset2 = FVector2D(source->OffsetX.GetMaxValue() - source->OffsetX.GetMinValue(), source->OffsetY.GetMaxValue() - source->OffsetY.GetMinValue());
	}
};
static FFuncParticleElementPosition FuncPosition;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementRotation: public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementRotation* source = static_cast<FSsParticleElementRotation*>(ele);
		e->Particle.bUseRotation = true;
		e->Particle.Rotation = source->Rotation.GetMinValue();
		e->Particle.Rotation2 = source->Rotation.GetMaxValue() - source->Rotation.GetMinValue();

		e->Particle.RotationAdd = source->RotationAdd.GetMinValue();
		e->Particle.RotationAdd2 = source->RotationAdd.GetMaxValue() - source->RotationAdd.GetMinValue();
	}
};
static FFuncParticleElementRotation FuncRotation;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementRotationTrans : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementRotationTrans* source = static_cast<FSsParticleElementRotationTrans*>(ele);
		e->Particle.bUseRotationTrans = true;
		e->Particle.RotationFactor = source->RotationFactor;
		e->Particle.EndLifeTimePer = source->EndLifeTimePer / 100.0f;
	}
};
static FFuncParticleElementRotationTrans FuncRotationTrans ;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementTransSpeed : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementTransSpeed* source = static_cast<FSsParticleElementTransSpeed*>(ele);
		e->Particle.bUseTransSpeed = true;
		e->Particle.TransSpeed = source->Speed.GetMinValue();
		e->Particle.TransSpeed2 = source->Speed.GetMaxValue() - source->Speed.GetMinValue();
	}
};
static FFuncParticleElementTransSpeed FuncTransSpeed;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementTangentialAcceleration : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementTangentialAcceleration* source = static_cast<FSsParticleElementTangentialAcceleration*>(ele);
		e->Particle.bUseTanAccel = true;
		e->Particle.TangentialAccel = source->Acceleration.GetMinValue();
		e->Particle.TangentialAccel2 = (source->Acceleration.GetMaxValue() - source->Acceleration.GetMinValue());
	}
};
static FFuncParticleElementTangentialAcceleration FuncTangentialAcceleration;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementInitColor : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementInitColor* source = static_cast<FSsParticleElementInitColor*>(ele);
		e->Particle.bUseColor = true;

		FSsU8Color color1 = source->Color.GetMinValue();
		FSsU8Color color2 = source->Color.GetMaxValue();

		GetRange(color1.A, color2.A, e->Particle.InitColor.A, e->Particle.InitColor2.A);
		GetRange(color1.R, color2.R, e->Particle.InitColor.R, e->Particle.InitColor2.R);
		GetRange(color1.G, color2.G, e->Particle.InitColor.G, e->Particle.InitColor2.G);
		GetRange(color1.B, color2.B, e->Particle.InitColor.B, e->Particle.InitColor2.B);
	}
};
static FFuncParticleElementInitColor FuncInitColor;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementTransColor : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementTransColor* source = static_cast<FSsParticleElementTransColor*>(ele);

		e->Particle.bUseTransColor = true;

		FSsU8Color color1 = source->Color.GetMinValue();
		FSsU8Color color2 = source->Color.GetMaxValue();

		GetRange(color1.A, color2.A, e->Particle.TransColor.A, e->Particle.TransColor2.A);
		GetRange(color1.R, color2.R, e->Particle.TransColor.R, e->Particle.TransColor2.R);
		GetRange(color1.G, color2.G, e->Particle.TransColor.G, e->Particle.TransColor2.G);
		GetRange(color1.B, color2.B, e->Particle.TransColor.B, e->Particle.TransColor2.B);
	}
};
static FFuncParticleElementTransColor FuncTransColor;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementAlphaFade : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementAlphaFade* source = static_cast<FSsParticleElementAlphaFade*>(ele);
		e->Particle.bUseAlphaFade = true;
		e->Particle.AlphaFade = source->Disprange.GetMinValue();
		e->Particle.AlphaFade2 = source->Disprange.GetMaxValue();
	}
};
static FFuncParticleElementAlphaFade FuncAlphaFade;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementSize : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementSize* source = static_cast<FSsParticleElementSize*>(ele);

		e->Particle.bUseInitScale = true;

		e->Particle.Scale.X = source->SizeX.GetMinValue();
		e->Particle.ScaleRange.X = source->SizeX.GetMaxValue() - source->SizeX.GetMinValue();

		e->Particle.Scale.Y = source->SizeY.GetMinValue();
		e->Particle.ScaleRange.Y = source->SizeY.GetMaxValue() - source->SizeY.GetMinValue();

		e->Particle.ScaleFactor = source->ScaleFactor.GetMinValue();
		e->Particle.ScaleFactor2 = source->ScaleFactor.GetMaxValue() - source->ScaleFactor.GetMinValue();
	}
};
static FFuncParticleElementSize FuncSize;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleElementTransSize : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleElementTransSize* source = static_cast<FSsParticleElementTransSize*>(ele);
		e->Particle.bUseTransScale = true;

		e->Particle.Transscale.X = source->SizeX.GetMinValue();
		e->Particle.TransscaleRange.X = source->SizeX.GetMaxValue() - source->SizeX.GetMinValue();

		e->Particle.Transscale.Y = source->SizeY.GetMinValue();
		e->Particle.TransscaleRange.Y = source->SizeY.GetMaxValue() - source->SizeY.GetMinValue();

		e->Particle.TransscaleFactor = source->ScaleFactor.GetMinValue();
		e->Particle.TransscaleFactor2 = source->ScaleFactor.GetMaxValue() - source->ScaleFactor.GetMinValue();
	}
};
static FFuncParticleElementTransSize FuncTransSize;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticlePointGravity : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticlePointGravity* source = static_cast<FSsParticlePointGravity*>(ele);
		e->Particle.bUsePGravity = true;
		e->Particle.GravityPos = source->Position;
		e->Particle.GravityPower = source->Power;
	}
};
static FFuncParticlePointGravity FuncPointGravity;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleTurnToDirectionEnabled : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		FSsParticleTurnToDirectionEnabled* source = static_cast<FSsParticleTurnToDirectionEnabled*>(ele);
		e->Particle.bUseTurnDirec = true;
		e->Particle.DirecRotAdd = source->Rotation;
	}
};
static FFuncParticleTurnToDirectionEnabled FuncTurnToDirectionEnabled;

//-----------------------------------------------------------------
//
//-----------------------------------------------------------------
class FFuncParticleInfiniteEmitEnabled : public FEffectFuncBase
{
public:
	virtual void InitializeEffect(FSsEffectElementBase* ele, FSsEffectEmitter* e) override
	{
		e->Emitter.bInfinite = true;
	}
};
static FFuncParticleInfiniteEmitEnabled FuncParticleInfiniteEmitEnabled;

//-------------------------------------------------------------------
//挙動反映クラスの呼び出しテーブル
//SsEffectFunctionTypeの順に並べること
//-------------------------------------------------------------------
static FEffectFuncBase* callTable[] =
{
	nullptr,
	&FuncBasic,
	&FuncRndSeedChange,
	&FuncDelay,
	&FuncGravity,
	&FuncPosition,
	&FuncRotation,
	&FuncRotationTrans,
	&FuncTransSpeed,
	&FuncTangentialAcceleration,
	&FuncInitColor,
	&FuncTransColor,
	&FuncAlphaFade,
	&FuncSize,
	&FuncTransSize,
	&FuncPointGravity,
	&FuncTurnToDirectionEnabled,
	&FuncParticleInfiniteEmitEnabled,
};


///----------------------------------------------------------------------------------------------------
//
///----------------------------------------------------------------------------------------------------
void FSsEffectFunctionExecuter::InitializeEffect(FSsEffectBehavior* beh, FSsEffectEmitter* e)
{
	for(int32 i = 0; i < beh->PList.Num(); ++i)
	{
		FEffectFuncBase* cf = callTable[beh->PList[i]->MyType];
		cf->InitializeEffect(beh->PList[i].Get(), e);
	}
}
