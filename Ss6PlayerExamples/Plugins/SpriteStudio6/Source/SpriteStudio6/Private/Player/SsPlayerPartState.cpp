#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerPartState.h"

#include "SsPlayerAnimedecode.h"
#include "SsPlayerEffect2.h"

FSsPartState::FSsPartState()
	: Index(-1)
	, Parent(nullptr)
	, NoCells(false)
	, AlphaBlendType(SsBlendType::Invalid)
	, RefAnime(nullptr)
	, RefEffect(nullptr)
{
	Init();
	EffectValue.AttrInitialized = false;
}

FSsPartState::~FSsPartState()
{
	Destroy();
}


void FSsPartState::Destroy()
{
	if(RefAnime)
	{
		delete RefAnime;
		RefAnime = nullptr;
	}
	if(RefEffect)
	{
		delete RefEffect;
		RefEffect = nullptr;
	}
}

void FSsPartState::Init()
{
	memset( Vertices , 0 , sizeof( Vertices ) );
	memset( Colors , 0 , sizeof( Colors ) );
	memset( Uvs , 0 , sizeof( Uvs ) );
	memset( Matrix , 0 , sizeof( Matrix ) );
	//cell = 0;
	Position = FVector( 0.0f , 0.0f, 0.0f );
	Rotation = FVector( 0.0f , 0.0f , 0.0f );
	Scale = FVector2D( 1.0f , 1.0f );

	Alpha = 1.0f;
	Prio = 0;
	HFlip = false;
	VFlip = false;
	Hide = false;

	PivotOffset = FVector2D(0, 0);
	Anchor = FVector2D(0, 0);
	Size = FVector2D(1.f, 1.f);

	ImageFlipH = false;
	ImageFlipV = false;

	UvTranslate = FVector2D(0, 0);
	UvRotation = 0;
	UvScale = FVector2D(1, 1);

	BoundingRadius = 0;

	IsColorBlend = false;
	IsVertexTransform = false;
	InheritRates = 0;

	EffectValue.Independent = false;
	EffectValue.LoopFlag = 0;
	EffectValue.Speed = 1.f;
	EffectValue.StartTime = 0;
	EffectValue.CurKeyframe = 0;

	EffectSpeed = 0;
	EffectTime = 0;

	InstanceValue.Infinity = false;
	InstanceValue.Reverse = false;
	InstanceValue.Pingpong = false;
	InstanceValue.Independent = false;
	InstanceValue.LoopFlag = 0;
	InstanceValue.LoopNum = 1;
	InstanceValue.StartLabel = FName("_start");
	InstanceValue.StartOffset = 0;
	InstanceValue.EndLabel = FName("_end");
	InstanceValue.EndOffset = 0;
	InstanceValue.CurKeyframe = 0;
	InstanceValue.Speed = 1.f;
	InstanceValue.StartFrame = 0;
	InstanceValue.EndFrame = 0;
	//InstanceValue.LiveFrame = 0.0f;	//加算値なので初期化してはいけない 
}

void FSsPartState::Reset()
{
	EffectValue.Independent = false;
	EffectValue.AttrInitialized = false;
	EffectValue.Speed = 1.f;
	EffectValue.StartTime = 0;
}


