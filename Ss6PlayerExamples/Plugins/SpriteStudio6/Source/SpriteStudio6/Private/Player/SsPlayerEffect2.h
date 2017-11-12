#pragma once

#include "xorshift32.h"
#include "SsPlayerCellmap.h"

struct FSsPartState;
struct FSsEffectBehavior;
struct FSsEffectModel;
struct FSsEffectNode;

#define SS_EFFECT_SEED_MAGIC (7573)
#define SS_EFFECT_LIFE_EXTEND_SCALE (8)
#define SS_EFFECT_LIFE_EXTEND_MIN   (64)

struct FTimeAndValue
{
	float Time;
	float Value;
};

struct FParticleExistSt
{
	int32	Id;
	int32	Cycle;
	int32	Exist;
	int32	Born;
	int32	STime;
	int32	EndTime;
};

struct FEmitPattern
{
	int32	Uid;		// std::sortとTArray::Sortのアルゴリズムの違いを吸収するための、ソート用第２比較値 
	int32	Life;
	int32	Cycle;

	// ソート用 
	bool operator < (const FEmitPattern& Other) const
	{
		return (Life != Other.Life) ? (Life < Other.Life) : (Uid < Other.Uid);
	}
};

//最終描画用データ
struct FParticleDrawData
{
	int32	Id;
	int32	PId;
	int32	STime;		//生成された開始時間
	int32	Lifetime;

	//描画用ワーク
	float	X;
	float	Y;
	float	Rot;
	float	Direc;

	FSsU8Color	Color;
	FVector2D	Scale;
};

//エミッターが持つパラメータ
//エディタやファイルから入力される
struct FEmitterParameter
{
	int32	Life;
	int32	Interval;
	int32	Emitnum;		//一度に排出される個数
	int32	Emitmax;		//最大排出数
	int32	ParticleLife;	//生成されるパーティクルの寿命
	int32	ParticleLife2;	//寿命最大値
	bool	bInfinite;		//無限発生

	int32	LoopStart;
	int32	LoopEnd;
	int32	LoopLen;
	int32	LoopGen;


	//テスト用デフォルト
	FEmitterParameter()
		: Life(15)
		, Interval(1)
		, Emitnum(2)
		, Emitmax(32)
		, ParticleLife(15)
		, ParticleLife2(15)
		, bInfinite(false)
	{}
};

//パーティクルが持つパラメータ
//エディタやファイルから入力される
struct FParticleParameter
{

	FVector2D	Scale;

	FSsU8Color	StartColor;	//スタート時のカラー
	FSsU8Color	EndColor;	//終了時のカラー

	//初速
	float		Speed;		//初速
	float		Speed2;		//初速最大値

	float		Angle;			//向いてる角度
	float		AngleVariance;	//変差

	bool		bUseGravity;
	FVector2D	Gravity;

	bool		bUseOffset;
	FVector2D	Offset;
	FVector2D	Offset2;

	bool		bUseRotation;
	float		Rotation;
	float		Rotation2;

	float		RotationAdd;
	float		RotationAdd2;

	bool		bUseRotationTrans;
	float		RotationFactor;
	float		EndLifeTimePer;

	bool		bUseTanAccel;
	float		TangentialAccel;
	float		TangentialAccel2;

	bool		bUseColor;
	FSsU8Color	InitColor;
	FSsU8Color	InitColor2;

	bool		bUseTransColor;
	FSsU8Color	TransColor;
	FSsU8Color	TransColor2;

	bool		bUseInitScale;
	FVector2D	ScaleRange;
	float		ScaleFactor;
	float		ScaleFactor2;

	bool		bUseTransScale;
	FVector2D	Transscale;
	FVector2D	TransscaleRange;
	float		TransscaleFactor;
	float		TransscaleFactor2;

	float		Delay;

	bool		bUsePGravity;
	FVector2D	GravityPos;
	float		GravityPower;

	bool		bUseAlphaFade;
	float		AlphaFade;
	float		AlphaFade2;

	bool		bUseTransSpeed;
	float		TransSpeed;
	float		TransSpeed2;

	bool		bUseTurnDirec;
	float		DirecRotAdd;

	bool		bUserOverrideRSeed;
	int32		OverrideRSeed;
};

//エミッタ動作クラス
struct FSsEffectEmitter
{
public:
	FSsCellValue	DispCell;

	int32	Priority;

	FEmitterParameter	Emitter;
	FParticleParameter	Particle;
	xorshift32			Rand;

	int32	EmitterSeed;
	int32	SeedOffset;

	//生成用のリングバッファ
	TArray<FEmitPattern>	EmitPattern;
	TArray<int32>			OffsetPattern;

	FParticleExistSt*		ParticleExistList;

	//事前計算バッファ
	int32	ParticleIdMax;

	size_t		ParticleListBufferSize;
	uint32*		SeedList;


	FVector2D			Position;
	FSsEffectEmitter*	Parent;

	int32	ParentIndex;

	FSsCell*			RefCell;	//描画用セル
	FSsEffectBehavior*	RefData;	//データ更新用

	size_t	GlobalTime;
	size_t	SeedTableLen;

	int32	Uid;

public:
	FSsEffectEmitter()
		: SeedOffset(0)
		, ParticleExistList(0)
		, ParticleListBufferSize(180 * 100)	//生成出来るパーティクルの最大値
		, SeedList(nullptr)
		, Position(0.f, 0.f)
		, Parent(nullptr)
		, ParentIndex(-1)
		, GlobalTime(0)
	{
		EmitterSeed = SS_EFFECT_SEED_MAGIC;
	}
	virtual ~FSsEffectEmitter()
	{
		delete [] ParticleExistList;
		delete [] SeedList;

	}

	void SetSeedOffset(int32 Offset)
	{
		SeedOffset = Offset; 
	}

	int32 GetParticleIDMax(){ return OffsetPattern.Num(); }
	const FParticleExistSt* GetParticleDataFromID(int32 Id);
	void UpdateEmitter(double Time, int32 Slide);

	int32 GetTimeLength(){ return Emitter.Life + (Emitter.ParticleLife + Emitter.ParticleLife2); }

	//現在時間から産出される位置を求める
	//time変数から求められる式とする
	void UpdateParticle(float InTime, FParticleDrawData* ParticleDrawData, bool bRecalc=false);

	//パーティクルの発生間隔を事前計算する
	//ここで出力が確定する

	void Precalculate2();


	// ソート用 
	bool operator < (const FSsEffectEmitter& Other) const
	{
		return (Priority != Other.Priority) ? (Priority < Other.Priority) : (Uid < Other.Uid);
	}
};

class FSsEffectRenderV2
{
public:
	//エフェクトのパラメータデータ
	FSsEffectModel*		EffectData;

	//Modelに記載されているエミッタのリスト
	TArray<FSsEffectEmitter*>	EmitterList;
	TArray<FSsEffectEmitter*>	UpdateList;

	//ランダムシード
	uint32		MySeed;

	FVector		LayoutPosition;
	FVector2D	LayoutScale;

	float	NowFrame;
	float	TargetFrame;
	float	SecondNowFrame;

	int32	EffectTimeLength;

	bool	bInfinite;	//無限に発生出来るかどうか

	FSsPartState*	ParentState;

	bool	bIsIntFrame;

	bool	bIsPlay;
	bool	bIsPause;
	bool	bIsLoop;

	int32	SeedOffset;

	FSsCellMapList*	CurCellMapManager;/// セルマップのリスト（アニメデコーダーからもらう

	bool bIsWarningData;

private:
	void InitEmitter(FSsEffectEmitter* Emitter, FSsEffectNode* Node);
	void ClearEmitterList();

public:
	FSsEffectRenderV2()
		: MySeed(0)
		, EffectTimeLength(0)
		, bIsIntFrame(true)
		, SeedOffset(0)
		, bIsWarningData(false)
	{}
	virtual ~FSsEffectRenderV2()
	{
		ClearEmitterList();
	}

	void Play(){ bIsPause=false; bIsPlay=true; }
	void Stop(){ bIsPlay=false; }
	void Pause(){ bIsPause=true; bIsPlay=false; }
	void SetLoop(bool Flag){ bIsLoop=Flag; }
	bool IsPlay(){ return bIsPlay; }
	bool IsPause(){ return bIsPause; }
	bool IsLoop(){ return bIsLoop; }

	void SetEffectData(FSsEffectModel* Data);

	void SetSeed(uint32 Seed)
	{
		MySeed = Seed * SS_EFFECT_SEED_MAGIC;
	}
	void SetFrame(float Frame)
	{
		NowFrame = Frame;
	}
	float GetFrame(){ return NowFrame; }

	void Update();
	void Reload();

	void CreateRenderParts(TArray<FSsRenderPart>& OutRenderParts, FSsPartState* State, const FVector2D& CurAnimeCanvasSize, const FVector2D& CurAnimePivot);
	void CreateRenderPart(TArray<FSsRenderPart>& OutRenderParts, FSsPartState* State, const FVector2D& CurAnimeCanvasSize, const FVector2D& CurAnimePivot
		,FSsEffectEmitter* Emitter, float Time, FSsEffectEmitter* Parent = nullptr, FParticleDrawData* DrawData = nullptr);

	void SetParentAnimeState(FSsPartState* State){ ParentState = State; }

	int32 GetCurrentFPS();

	void SetCellmapManager(FSsCellMapList* PList){ CurCellMapManager=PList; }

	bool GetPlayStatus()
	{
		return bIsPlay;
	}

	void SetSeedOffset(int32 Offset);
};

