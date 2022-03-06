﻿#ifndef __SSEFFECTRENDER_V3__
#define __SSEFFECTRENDER_V3__


#include "xorshift32.h"

//SsVectorなど
//#include "SsUtility.h"

//#include "ISSEffectRender.h"

#include "ssplayer_cellmap.h"
#include "ssplayer_PartState.h"


//class FSsEffectModel;
class SsRenderEffectBase;
struct FSsEffectNode;
//class SsPartState;
class SsEffectRenderAtom;
struct FSsCell;


#define SEED_MAGIC (7573)
#define LIFE_EXTEND_SCALE (8)
#define LIFE_EXTEND_MIN	  (64)

#define LOOP_TYPE1 (0)
#define LOOP_TYPE2 (0)
#define LOOP_TYPE3 (1)


struct TimeAndValue
{
	float time;
	float value;
};


//v3.1
struct particleExistSt
{
	int  id;
	int	 cycle;
	int	 exist;
    int	 born;
	long stime;
	long endtime;
};


//v3.1
struct emitPattern
{
	int	  uid;
	int   life;
    int   cycle;
};

//最終描画用データ
struct particleDrawData
{
	int	id;
	int	pid;
	int	stime;		//生成された開始時間
	int lifetime;

	//描画用ワーク
	float	x;
	float	y;
	float	rot;
	float	direc;

    FSsU8Color color;
    FVector2f scale;
};


#if 0
//リングバッファだが実はもういらないかも
template<class mytype>
class particleRingBuffer
{
protected:

	mytype*		ar;
	int*		realIndex;
	size_t		bufsize;


public:
	particleRingBuffer() {
    	 resize(16);
	}
	particleRingBuffer(size_t size)
	{
           resize(size);
	}
	virtual ~particleRingBuffer()
	{
		delete[] ar;
		delete[] realIndex;
	}

	void clear() {
		memset(ar, 0, sizeof(mytype) *(bufsize + 1));
		memset(realIndex, 0, sizeof(int) *(bufsize + 1));
	}

	void store(int index, mytype* t)
	{
		ar[index % bufsize] = *t;
		realIndex[index % bufsize] = index;
	}

	void resize( size_t size )
	{
		bufsize = size*2;
		ar = new mytype[bufsize +1];
		realIndex = new int[bufsize + 1];
		clear();
	}

	mytype& load(int index)
	{
		return ar[index % bufsize];
	}

	int loadRealIndex(int index)
	{
		return realIndex[index];
	}
	size_t	getBufsize() { return bufsize; }

};

#endif


//エミッターが持つパラメータ
//エディタやファイルから入力される
struct emitterParameter
{
	int	life;
	int interval;
	int	emitnum;		//一度に排出される個数
	int emitmax;		//最大排出数
	int	particleLife;	//生成されるパーティクルの寿命
	int particleLife2;	//寿命最大値
    bool Infinite;      //無限発生

	int	loopStart;
	int loopEnd;
	int loopLen;
	int	loopGen;


	//テスト用デフォルト
	emitterParameter() : life(15),interval(1),emitnum(2),emitmax(32),particleLife(15),particleLife2(15),Infinite(false){}

};


//パーティクルが持つパラメータ
//エディタやファイルから入力される
struct particleParameter
{

	FVector2f 	scale;

	FSsU8Color   startcolor; //スタート時のカラー
	FSsU8Color   endcolor;   //終了時のカラー

	//初速
	float		speed;		//初速
	float		speed2;		//初速最大値


	float		angle;		  //向いてる角度
	float       angleVariance;//変差

	bool		useGravity;
	FVector2f	gravity;


	bool		useOffset;
	FVector2f   offset;
	FVector2f   offset2;

	bool        useRotation;
	float		rotation;
	float		rotation2;

	float		rotationAdd;
	float		rotationAdd2;

	bool		useRotationTrans;
	float		rotationFactor;
	float		endLifeTimePer;

	bool        useTanAccel;
	float		tangentialAccel;
	float		tangentialAccel2;

	bool        useColor;
	FSsU8Color   initColor;
	FSsU8Color   initColor2;

	bool		useTransColor;
	FSsU8Color   transColor;
	FSsU8Color   transColor2;

	bool        useInitScale;
	FVector2f   scaleRange;
	float		scaleFactor;
	float		scaleFactor2;

	bool        useTransScale;
	FVector2f   transscale;
	FVector2f   transscaleRange;
	float		transscaleFactor;
	float		transscaleFactor2;

	float		delay;

	bool		usePGravity;
	FVector2f	gravityPos;
	float		gravityPower;

	bool		useAlphaFade;
	float       alphaFade;
	float       alphaFade2;

	bool		useTransSpeed;
	float	   	transSpeed;
	float	   	transSpeed2;

	bool		useTurnDirec;
    float		direcRotAdd;

    bool		userOverrideRSeed;
	int			overrideRSeed;


	particleParameter(){}
};


//エミッタ動作クラス
class SsEffectEmitter
{
public:
	SsCellValue		 dispCell;


	int					priority;

	emitterParameter  	emitter;
	particleParameter   particle;
	xorshift32			rand;


	int					emitterSeed;
	int					seedOffset;

	//生成用のリングバッファ
	TArray<emitPattern>    	_emitpattern;
	TArray<int>				_offsetPattern;

    particleExistSt*     particleExistList;


	//事前計算バッファ
	//particleLifeSt*				particleList;
	int							particleIdMax;

	size_t						particleListBufferSize;
    unsigned long*              seedList;


	FVector2f   				position;
//	SsEffectEmitter*			_child;
	SsEffectEmitter*			_parent;

    int							_parentIndex;

	FSsCell*					refCell;    //描画用セル
	FSsEffectBehavior*           refData;	//データ更新用

	size_t						globaltime;
	size_t						seedTableLen;

	int							uid;

public:
	SsEffectEmitter() :
			seedOffset(0),
//			_emitpattern(0),
			particleExistList(0),
//			particleList(0),
			particleListBufferSize(180*100),  //生成出来るパーティクルの最大値			
			seedList(0),
			position(0, 0),
			_parentIndex(-1),
			globaltime(0)
	{
		emitterSeed = SEED_MAGIC;
	}
	virtual ~SsEffectEmitter()
	{
		delete [] particleExistList;
		delete [] seedList;

	}

	void	setSeedOffset( int offset ) { 
		seedOffset = offset; 
	}

//	const particleLifeSt*	getParticleDataFromID(int id) { return &particleList[id]; }

#if  LOOP_TYPE3

	int	getParticleIDMax() { return _offsetPattern.Num(); }

	const 	particleExistSt*	getParticleDataFromID(int id);
	void	updateEmitter( double time  , int slide );

#else

	int	getParticleIDMax() { return particleIdMax; }
	const particleLifeSt*	getParticleDataFromID(int id);
#endif


	int	getTimeLength() { return emitter.life + ( emitter.particleLife + emitter.particleLife2); }

	//現在時間から産出される位置を求める
	//time変数から求められる式とする
	void	updateParticle(float time, particleDrawData* p, bool recalc = false );

	//パーティクルの発生間隔を事前計算する
	//ここで出力が確定する

	void	precalculate2();


	// UE4 TArray::Sort() 用 
	bool operator < (const SsEffectEmitter& Other) const
	{
		return (priority != Other.priority) ? (priority < Other.priority) : (uid < Other.uid);
	}
};


class SsEffectRenderV2
{
public:

	//エフェクトのパラメータデータ
	FSsEffectModel*		effectData;

	//Modelに記載されているエミッタのリスト
	TArray<SsEffectEmitter*>   emmiterList;

	TArray<SsEffectEmitter*>   updateList;

	//ランダムシード
	uint32			mySeed;

	FVector			layoutPosition;
	FVector2f		layoutScale;

	float			nowFrame;
	float			targetFrame;
    float			secondNowFrame;

	size_t          effectTimeLength;

    bool			Infinite;	//無限に発生出来るかどうか

	SsPartState*	parentState;

	bool			isIntFrame;

	bool			m_isPlay;
	bool			m_isPause;
	bool			m_isLoop;

	int				seedOffset;

	SsCellMapList*	curCellMapManager;/// セルマップのリスト（アニメデコーダーからもらう

	bool		_isWarningData;
public:


protected:
//	void 	particleDraw(SsEffectEmitter* e , double t , SsEffectEmitter* parent = 0 , particleDrawData* plp = 0 );
	void	initEmitter( SsEffectEmitter* e , FSsEffectNode* node);

	void	clearEmitterList();

public:
	SsEffectRenderV2() : mySeed(0), effectTimeLength(0) ,isIntFrame(true),seedOffset(0){}
	virtual ~SsEffectRenderV2()
	{
		clearEmitterList();
	}

	virtual void    play(){ m_isPause = false;m_isPlay=true; }
	virtual void	stop(){ m_isPlay = false;}
	virtual void    pause(){m_isPause = true;m_isPlay=false;}
	virtual void	setLoop(bool flag){ m_isLoop = flag; }
	virtual bool	isplay(){return m_isPlay;}
	virtual bool	ispause(){return m_isPause;}
	virtual bool	isloop(){return m_isLoop;}

	virtual void	setEffectData(FSsEffectModel* data);


	virtual void	setSeed( uint32 seed )
	{
   		mySeed = seed * SEED_MAGIC;
	}


	virtual void	setFrame( float frame )
	{
    	nowFrame = frame;
	}

    virtual float	getFrame(){ return nowFrame; }

	virtual void	update();
//	virtual void	draw();

	virtual void    reload();

    virtual size_t  getEffectTimeLength();

//	virtual int		getVersion(){ return EFFECTRENDERVERSION_V2; }

	virtual  void	setParentAnimeState( SsPartState* state ){ parentState = state; }

	virtual int	getCurrentFPS();

	void	setCellmapManager( SsCellMapList* plist ) { curCellMapManager = plist; }

	bool	getPlayStatus(void){
		return(m_isPlay);
	}

/*	void	drawSprite(
			SsCellValue*		dispCell,
			FVector2f	_position,
			FVector2f _size,
			float     _rotation,
			float	  direction,
			SsFColor	_color,
			SsRenderBlendType::Type blendType
		);
*/
	
	void	setSeedOffset( int offset ) { 
		if ( effectData->IsLockRandSeed )
		{
			seedOffset = 0;
		}else{
			seedOffset = offset; 
		}
	}

	virtual bool	isInfinity(){ return Infinite; }
    virtual bool	isWarning(){ return _isWarningData; }

};


#endif