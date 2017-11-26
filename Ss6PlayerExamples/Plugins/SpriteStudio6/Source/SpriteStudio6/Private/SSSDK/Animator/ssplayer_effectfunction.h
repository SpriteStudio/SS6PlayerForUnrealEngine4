#ifndef __SSPLAYER_EFFECTFUNCTION__
#define __SSPLAYER_EFFECTFUNCTION__

//#include "ssplayer_effect.h"
#include "ssplayer_effect2.h"

class	SsEffectFunctionExecuter
{
public:
	SsEffectFunctionExecuter(){}
	virtual ~SsEffectFunctionExecuter(){}

//	static void	initalize( FSsEffectBehavior* beh , SsEffectRenderEmitter* emmiter);
//	static void	updateEmmiter( FSsEffectBehavior* beh , SsEffectRenderEmitter* emmiter);
//	static void	initializeParticle( FSsEffectBehavior* beh , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle );
//	static void	updateParticle( FSsEffectBehavior* beh , SsEffectRenderEmitter* e , SsEffectRenderParticle* particle );


	//新バージョン
	static void	initializeEffect( FSsEffectBehavior* beh ,  SsEffectEmitter* emmiter);


};






#endif
