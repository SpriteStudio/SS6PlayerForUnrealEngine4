#include "SpriteStudio6PrivatePCH.h"
#include "SsEffectElement.h"

//---------------------------------------------------------------
//相互変換 SsEffectFunctionType
FString	__EnumToString_( TEnumAsByte<SsEffectFunctionType::Type> n )
{
	if(SsEffectFunctionType::Base) return "Base";
	if(SsEffectFunctionType::Basic) return "Basic";
	if(SsEffectFunctionType::RndSeedChange) return "RndSeedChange";
	if(SsEffectFunctionType::Delay) return "Delay";
	if(SsEffectFunctionType::Gravity) return "Gravity";
	if(SsEffectFunctionType::Position) return "Position";
	if(SsEffectFunctionType::Rotation) return "Rotation";
	if(SsEffectFunctionType::TransRotation) return "TransRotation";
	if(SsEffectFunctionType::TransSpeed) return "TransSpeed";
	if(SsEffectFunctionType::TangentialAcceleration) return "TangentialAcceleration";
	if(SsEffectFunctionType::InitColor) return "InitColor";
	if(SsEffectFunctionType::TransColor) return "TransColor";
	if(SsEffectFunctionType::AlphaFade) return "AlphaFade";
	if(SsEffectFunctionType::Size) return "Size";
	if(SsEffectFunctionType::TransSize) return "TransSize";
	if(SsEffectFunctionType::PointGravity) return "PointGravity";
	if(SsEffectFunctionType::TurnToDirectionEnabled) return "TurnToDirectionEnabled";

	return "Base";
}

void	__StringToEnum_( FString n , TEnumAsByte<SsEffectFunctionType::Type> &out )
{
	out = SsEffectFunctionType::Base;
	if (n == "Base") out = SsEffectFunctionType::Base;
	if (n == "Basic") out = SsEffectFunctionType::Basic;
	if (n == "RndSeedChange") out = SsEffectFunctionType::RndSeedChange;
	if (n == "Delay") out = SsEffectFunctionType::Delay;
	if (n == "Gravity") out = SsEffectFunctionType::Gravity;
	if (n == "Position") out = SsEffectFunctionType::Position;
	if (n == "Rotation") out = SsEffectFunctionType::Rotation;
	if (n == "TransRotation") out = SsEffectFunctionType::TransRotation;
	if (n == "TransSpeed") out = SsEffectFunctionType::TransSpeed;
	if (n == "TangentialAcceleration") out = SsEffectFunctionType::TangentialAcceleration;
	if (n == "InitColor") out = SsEffectFunctionType::InitColor;
	if (n == "TransColor") out = SsEffectFunctionType::TransColor;
	if (n == "AlphaFade") out = SsEffectFunctionType::AlphaFade;
	if (n == "Size") out = SsEffectFunctionType::Size;
	if (n == "TransSize") out = SsEffectFunctionType::TransSize;
	if (n == "PointGravity") out = SsEffectFunctionType::PointGravity;
	if (n == "TurnToDirectionEnabled") out = SsEffectFunctionType::TurnToDirectionEnabled;
}

//---------------------------------------------------------------
//相互変換 SsVarianceValueRangeType
FString	__EnumToString_( TEnumAsByte<SsVarianceValueRangeType::Type> n )
{
	if(SsVarianceValueRangeType::None) return "None";
	if(SsVarianceValueRangeType::MinMax) return "MinMax";
	if(SsVarianceValueRangeType::PlusMinus) return "PlusMinus";

	return "None";
}

void	__StringToEnum_( FString n , TEnumAsByte<SsVarianceValueRangeType::Type> &out )
{
	out = SsVarianceValueRangeType::None;
	if (n == "None") out = SsVarianceValueRangeType::None;
	if (n == "MinMax") out = SsVarianceValueRangeType::MinMax;
	if (n == "PlusMinus") out = SsVarianceValueRangeType::PlusMinus;
}


//---------------------------------------------------------------

void FSsParticleElementBasic::Serialize(FArchive& Ar)
{
	Ar << MaximumParticle;
	Speed.Serialize(Ar);
	Lifespan.Serialize(Ar);
	Ar << Angle;
	Ar << AngleVariance;
	Ar << Interval;
	Ar << Lifetime;
	Ar << AttimeCreate;
	Ar << Priority;
}

void FSsParticleElementRndSeedChange::Serialize(FArchive& Ar)
{
	Ar << Seed;
}

void FSsParticleElementDelay::Serialize(FArchive& Ar)
{
	Ar << DelayTime;
}

void FSsParticleElementGravity::Serialize(FArchive& Ar)
{
	Ar << Gravity;
}

void FSsParticleElementPosition::Serialize(FArchive& Ar)
{
	OffsetX.Serialize(Ar);
	OffsetY.Serialize(Ar);
}

void FSsParticleElementRotation::Serialize(FArchive& Ar)
{
	Rotation.Serialize(Ar);
	RotationAdd.Serialize(Ar);
}

void FSsParticleElementRotationTrans::Serialize(FArchive& Ar)
{
	Ar << RotationFactor;
	Ar << EndLifeTimePer;
}

void FSsParticleElementTransSpeed::Serialize(FArchive& Ar)
{
	Speed.Serialize(Ar);
}

void FSsParticleElementTangentialAcceleration::Serialize(FArchive& Ar)
{
	Acceleration.Serialize(Ar);
}

void FSsParticleElementInitColor::Serialize(FArchive& Ar)
{
	Color.Serialize(Ar);
}

void FSsParticleElementTransColor::Serialize(FArchive& Ar)
{
	Color.Serialize(Ar);
}

void FSsParticleElementAlphaFade::Serialize(FArchive& Ar)
{
	Disprange.Serialize(Ar);
}

void FSsParticleElementSize::Serialize(FArchive& Ar)
{
	SizeX.Serialize(Ar);
	SizeY.Serialize(Ar);
	ScaleFactor.Serialize(Ar);
}

void FSsParticleElementTransSize::Serialize(FArchive& Ar)
{
	SizeX.Serialize(Ar);
	SizeY.Serialize(Ar);
	ScaleFactor.Serialize(Ar);
}

void FSsParticlePointGravity::Serialize(FArchive& Ar)
{
	Ar << Position;
	Ar << Power;
}

void FSsParticleTurnToDirectionEnabled::Serialize(FArchive& Ar)
{
	if(2 <= Ar.CustomVer(SSPJ_GUID))
	{
		Ar << Rotation;
	}
}

void FSsParticleInfiniteEmitEnabled::Serialize(FArchive& Ar)
{
}
