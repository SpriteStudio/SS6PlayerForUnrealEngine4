#include "SpriteStudio6PrivatePCH.h"
#include "SsEffectBehavior.h"


void FSsEffectBehavior::Serialize(FArchive& Ar)
{
	int32 Cnt = 0;
	if(Ar.IsLoading())
	{
		Ar << Cnt;

		for(int32 i = 0; i < Cnt; ++i)
		{
			TEnumAsByte<SsEffectFunctionType::Type> Type;
			FSsEffectElementBase* NewElement = NULL;

			Ar << Type;
			switch(Type)
			{
			case SsEffectFunctionType::Base:					NewElement = new FSsEffectElementBase();						break;
			case SsEffectFunctionType::Basic:					NewElement = new FSsParticleElementBasic();						break;
			case SsEffectFunctionType::RndSeedChange:			NewElement = new FSsParticleElementRndSeedChange();				break;
			case SsEffectFunctionType::Delay:					NewElement = new FSsParticleElementDelay();						break;
			case SsEffectFunctionType::Gravity:					NewElement = new FSsParticleElementGravity();					break;
			case SsEffectFunctionType::Position:				NewElement = new FSsParticleElementPosition();					break;
			case SsEffectFunctionType::Rotation:				NewElement = new FSsParticleElementRotation();					break;
			case SsEffectFunctionType::TransRotation:			NewElement = new FSsParticleElementRotationTrans();				break;
			case SsEffectFunctionType::TransSpeed:				NewElement = new FSsParticleElementTransSpeed();				break;
			case SsEffectFunctionType::TangentialAcceleration:	NewElement = new FSsParticleElementTangentialAcceleration();	break;
			case SsEffectFunctionType::InitColor:				NewElement = new FSsParticleElementInitColor();					break;
			case SsEffectFunctionType::TransColor:				NewElement = new FSsParticleElementTransColor();				break;
			case SsEffectFunctionType::AlphaFade:				NewElement = new FSsParticleElementAlphaFade();					break;
			case SsEffectFunctionType::Size:					NewElement = new FSsParticleElementSize();						break;
			case SsEffectFunctionType::TransSize:				NewElement = new FSsParticleElementTransSize();					break;
			case SsEffectFunctionType::PointGravity:			NewElement = new FSsParticlePointGravity();						break;
			case SsEffectFunctionType::TurnToDirectionEnabled:	NewElement = new FSsParticleTurnToDirectionEnabled();			break;
			case SsEffectFunctionType::InfiniteEmitEnabled:		NewElement = new FSsParticleInfiniteEmitEnabled();				break;
			}
			if(NewElement)
			{
				NewElement->Serialize(Ar);
				PList.Add(MakeShareable(NewElement));
			}
		}
	}
	else if(Ar.IsSaving())
	{
		Cnt = PList.Num();
		Ar << Cnt;

		for(int32 i = 0; i < Cnt; ++i)
		{
			Ar << PList[i]->MyType;
			PList[i]->Serialize(Ar);
		}
	}

}
