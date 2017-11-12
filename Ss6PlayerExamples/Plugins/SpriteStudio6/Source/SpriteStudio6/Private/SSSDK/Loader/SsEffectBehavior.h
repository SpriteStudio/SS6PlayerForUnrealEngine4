#ifndef __SSEFFECTBEHAVIOR__
#define __SSEFFECTBEHAVIOR__

#include "SsEffectElement.h"
#include "sstypes.h"
#include "ssvalue.h"

class SsEffectElementBase;
class SsEffectRenderer;

class SsEffectBehavior
{
public:
	std::vector<SsEffectElementBase*> plist;

public:
    SsCell*							refCell;
	SsString						CellName;
	SsString						CellMapName;
	SsRenderBlendType::_enum		BlendType;

public:
	SsEffectBehavior() : refCell(0),BlendType(SsRenderBlendType::invalid) {}
	virtual ~SsEffectBehavior(){}

	SSSERIALIZE_BLOCK
	{
		SSAR_DECLARE(CellName);
		SSAR_DECLARE(CellMapName);
		SSAR_DECLARE_ENUM( BlendType );

		if ( ar->getxml() )
			EffectElementLoader( ar );
	}

	SsEffectElementBase*	Factory(const char* name , XMLElement* e );

	void	EffectElementLoader(ISsXmlArchiver* ar);
	void	setup();
/*
	//エフェクトデータのセットアップ
	void	initalize( SsEffectRenderEmitter* emmiter){}
	void	updateEmmiter(SsEffectRenderEmitter* emmiter){}
	void	initializeParticle( SsEffectRenderEmitter* e , SsEffectRenderParticle* particle ){}
	void	updateParticle( SsEffectRenderEmitter* e , SsEffectRenderParticle* particle ){}
*/


};






#endif
