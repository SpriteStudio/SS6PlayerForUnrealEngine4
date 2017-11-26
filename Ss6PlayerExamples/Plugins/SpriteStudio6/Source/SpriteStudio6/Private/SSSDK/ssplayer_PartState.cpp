#include "SpriteStudio6PrivatePCH.h"

//#include "../Loader/ssloader.h"
#include "ssplayer_animedecode.h"
#include "ssplayer_PartState.h"
#include "ssplayer_effect2.h"

SsPartState::SsPartState() : refAnime(0), index(-1), parent(nullptr), noCells(false), alphaBlendType(SsBlendType::Invalid),	refEffect(0) {
	init();
	effectValue.attrInitialized = false;
	meshPart = 0;

}

SsPartState::~SsPartState(){
	destroy();
}


void	SsPartState::destroy()
{
	if ( refAnime )	
	{
		delete refAnime;
		refAnime = 0;
	}
	if ( refEffect )
	{
		delete refEffect;
		refEffect = 0;
	}
}

void	SsPartState::init()
{
	memset( vertices , 0 , sizeof( vertices ) );
	memset( colors , 0 , sizeof( colors ) );
	memset( uvs , 0 , sizeof( uvs ) );
	memset( matrix , 0 , sizeof( matrix ) );
	//cell = 0;
	position = FVector( 0.0f , 0.0f, 0.0f );
	rotation = FVector( 0.0f , 0.0f , 0.0f );
	scale = FVector2D( 1.0f , 1.0f );
	localscale = FVector2D(1.0f, 1.0f);

	alpha = 1.0f;
	localalpha = 1.0f;
	prio = 0;
	hFlip = false;
	vFlip = false;
	hide = false;

	pivotOffset = FVector2D(0, 0);
	anchor = FVector2D( 0 , 0 );
	size = FVector2D( 1 , 1 );

	imageFlipH = false;
	imageFlipV = false;
	uvTranslate = FVector2D(0, 0);
	uvRotation = 0;
	uvScale = FVector2D( 1 , 1 );

	boundingRadius = 0;

	is_parts_color = false;
	is_color_blend = false;
	is_vertex_transform = false;
	inheritRates = 0; 

	is_localAlpha = false;
	
	effectValue.independent = false;
	effectValue.loopflag = 0;
	effectValue.speed = 1.0f;
	effectValue.startTime = 0;
	effectValue.curKeyframe = 0;

	effectseed = 0;
	effectTime = 0;

	instanceValue.infinity = false;
	instanceValue.reverse = false;
	instanceValue.pingpong = false;
	instanceValue.independent = false;
	instanceValue.loopflag = 0;
	instanceValue.loopNum = 1;
	instanceValue.startLabel = "_start";
	instanceValue.startOffset = 0;
	instanceValue.endLabel = "_end";
	instanceValue.endOffset = 0;
	instanceValue.curKeyframe = 0;
	instanceValue.speed = 1.0f;
	instanceValue.startFrame = 0;
	instanceValue.endFrame = 0;
//	instanceValue.liveFrame = 0.0f;	//加算値なので初期化してはいけない
	
	masklimen = 0;

//	partType = SsPartType::normal;
//	maskInfluence = false;

}


void	SsPartState::reset()
{
	effectValue.independent = false;
	effectValue.attrInitialized = false;
	effectValue.speed = 1.0f;
	effectValue.startTime = 0;

}