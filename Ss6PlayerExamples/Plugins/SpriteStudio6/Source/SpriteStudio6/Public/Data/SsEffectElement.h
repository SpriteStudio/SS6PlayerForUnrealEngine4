#pragma once

#include "SsTypes.h"

#include "SsEffectElement.generated.h"


//---------------------------------------------------------------
UENUM()
namespace SsEffectPartType
{
	enum Type
	{
		EffectPartTypeEmiiter,
		EffectPartTypeParticle
	};
}

// 命令種別
UENUM()
namespace SsEffectFunctionType
{
	enum Type
	{
		Base,
		Basic,
		RndSeedChange,
		Delay,
		Gravity,
		Position,
		Rotation,
		TransRotation,
		TransSpeed,
		TangentialAcceleration,
		InitColor,
		TransColor,
		AlphaFade,
		Size,
		TransSize,
		PointGravity,
		TurnToDirectionEnabled,
		InfiniteEmitEnabled,
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsEffectFunctionType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsEffectFunctionType::Type>& out);

UENUM()
namespace SsVarianceValueRangeType
{
	enum Type
	{
		None,
		MinMax,
		PlusMinus
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsVarianceValueRangeType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsVarianceValueRangeType::Type>& out);


// 範囲値クラス(float)
USTRUCT()
struct SPRITESTUDIO6_API FSsVarianceValueFloat
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	TEnumAsByte<SsVarianceValueRangeType::Type> RangeType;

	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	float Value;

	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	float SubValue;

public:
	FSsVarianceValueFloat(){}
	FSsVarianceValueFloat(float v)
	{
		Value = SubValue = v;
		RangeType = SsVarianceValueRangeType::None;
	}
	FSsVarianceValueFloat(float v1, float v2)
	{
		Value = v1;
		SubValue = v2;
		RangeType = SsVarianceValueRangeType::MinMax;
	}

	float GetMinValue() const { return Value; }
	float GetMaxValue() const { return SubValue; }

	void SetPlusMinus(float v)
	{
		Value = -v;
		SubValue = v;
		RangeType = SsVarianceValueRangeType::PlusMinus;
	}
	void SetMinMax(float v_min , float v_max)
	{
		Value = v_min;
		SubValue = v_max;
		RangeType = SsVarianceValueRangeType::MinMax;
	}

	FSsVarianceValueFloat& operator=(float v) { Value = v; return *this; }
	operator float() { return Value; }

	void Serialize(FArchive& Ar)
	{
		Ar << RangeType;
		Ar << Value;
		Ar << SubValue;
	}
};

// 範囲値クラス(int32)
USTRUCT()
struct SPRITESTUDIO6_API FSsVarianceValueInt
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	TEnumAsByte<SsVarianceValueRangeType::Type> RangeType;

	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	int32 Value;

	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	int32 SubValue;

public:
	FSsVarianceValueInt(){}
	FSsVarianceValueInt(int32 v)
	{
		Value = SubValue = v;
		RangeType = SsVarianceValueRangeType::None;
	}
	FSsVarianceValueInt(int32 v1, int32 v2)
	{
		Value = v1;
		SubValue = v2;
		RangeType = SsVarianceValueRangeType::MinMax;
	}

	int32 GetMinValue() const { return Value; }
	int32 GetMaxValue() const { return SubValue; }

	void SetPlusMinus(int32 v)
	{
		Value = -v;
		SubValue = v;
		RangeType = SsVarianceValueRangeType::PlusMinus;
	}
	void SetMinMax(int32 v_min , int32 v_max)
	{
		Value = v_min;
		SubValue = v_max;
		RangeType = SsVarianceValueRangeType::MinMax;
	}

	FSsVarianceValueInt& operator=(int32 v) { Value = v; return *this; }
	operator int32() { return Value; }

	void Serialize(FArchive& Ar)
	{
		Ar << RangeType;
		Ar << Value;
		Ar << SubValue;
	}
};

// 範囲値クラス(SsU8Color)
USTRUCT()
struct SPRITESTUDIO6_API FSsVarianceValueColor
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	TEnumAsByte<SsVarianceValueRangeType::Type> RangeType;

	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	FSsU8Color Value;

	UPROPERTY(VisibleAnywhere, Category=SsVarianceValue)
	FSsU8Color SubValue;

public:
	FSsVarianceValueColor(){}
	FSsVarianceValueColor(FSsU8Color v)
	{
		Value = SubValue = v;
		RangeType = SsVarianceValueRangeType::None;
	}
	FSsVarianceValueColor(FSsU8Color v1, FSsU8Color v2)
	{
		Value = v1;
		SubValue = v2;
		RangeType = SsVarianceValueRangeType::MinMax;
	}

	FSsU8Color GetMinValue() const { return Value; }
	FSsU8Color GetMaxValue() const { return SubValue; }

	FSsVarianceValueColor& operator=(FSsU8Color v) { Value = v; return *this; }
	operator FSsU8Color() { return Value; }

	void Serialize(FArchive& Ar)
	{
		Ar << RangeType;
		Ar << Value.R;
		Ar << Value.G;
		Ar << Value.B;
		Ar << Value.A;
		Ar << SubValue.R;
		Ar << SubValue.G;
		Ar << SubValue.B;
		Ar << SubValue.A;
	}
};



USTRUCT()
struct SPRITESTUDIO6_API FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	virtual void Serialize(FArchive& Ar){}

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement, Transient)
	TEnumAsByte<SsEffectFunctionType::Type> MyType;

public:
	FSsEffectElementBase()
		: MyType(SsEffectFunctionType::Base)
	{}
	virtual ~FSsEffectElementBase(){}
};


//--------------------------------------------------------------------------------------
//パーティクルを構成する基本の値
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementBasic  : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	int32 MaximumParticle;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat Speed;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueInt Lifespan;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	float Angle;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	float AngleVariance;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	int32 Interval;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	int32 Lifetime;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	int32 AttimeCreate;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	int32 Priority;

public:
	FSsParticleElementBasic()
		: MaximumParticle(50)
		, Speed(5.0f, 5.0f)
		, Lifespan(30, 30)
		, Angle(0.0f)
		, AngleVariance(45.0f)
		, Interval(1)
		, Lifetime(30)
		, AttimeCreate(1)
		, Priority(64)
	{
		MyType = SsEffectFunctionType::Basic;
	}
};


//--------------------------------------------------------------------------------------
//遅れ
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementRndSeedChange : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	int32 Seed;

public:
	FSsParticleElementRndSeedChange()
		: Seed(0)
	{
		MyType = SsEffectFunctionType::RndSeedChange;
	}
};

//--------------------------------------------------------------------------------------
//遅れ
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementDelay : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	int32 DelayTime;

public:
	FSsParticleElementDelay()
		: DelayTime(0)
	{
		MyType = SsEffectFunctionType::Delay;
	}
};

//--------------------------------------------------------------------------------------
//重力への影響
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementGravity : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FVector2D Gravity;

public:
	FSsParticleElementGravity()
		: Gravity(0, -3.0f)
	{
		MyType = SsEffectFunctionType::Gravity;
	}
};

//--------------------------------------------------------------------------------------
//発生位置への影響
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementPosition : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat OffsetX;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat OffsetY;

public:
	FSsParticleElementPosition()
		: OffsetX(0,0)
		, OffsetY(0,0)
	{
		MyType = SsEffectFunctionType::Position;
	}
};

//--------------------------------------------------------------------------------------
//角度変化
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementRotation : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat Rotation;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat RotationAdd;

public:
	FSsParticleElementRotation()
		: Rotation(0 ,0)
		, RotationAdd(0, 0)
	{
		MyType = SsEffectFunctionType::Rotation;
	}
};

//--------------------------------------------------------------------------------------
//角度変化
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementRotationTrans : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	float RotationFactor;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	float EndLifeTimePer;

public:
	FSsParticleElementRotationTrans()
		: RotationFactor(0)
		, EndLifeTimePer(75)
	{
		MyType = SsEffectFunctionType::TransRotation;
	}
};

//--------------------------------------------------------------------------------------
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementTransSpeed : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat Speed;

public:
	FSsParticleElementTransSpeed()
		: Speed(0,0)
	{
		MyType = SsEffectFunctionType::TransSpeed;
	}
};

//--------------------------------------------------------------------------------------
//接戦加速度を与える
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementTangentialAcceleration : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat Acceleration;

public:
	FSsParticleElementTangentialAcceleration()
		: Acceleration(0, 0)
	{
		MyType = SsEffectFunctionType::TangentialAcceleration;
	}
};

//--------------------------------------------------------------------------------------
//頂点カラーを制御する
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementInitColor : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueColor Color;

public:
	FSsParticleElementInitColor()
		: Color(FSsU8Color(255,255,255,255), FSsU8Color(255,255,255,255))
	{
		MyType = SsEffectFunctionType::InitColor;
	}
};

//--------------------------------------------------------------------------------------
//頂点カラーを制御する
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementTransColor : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueColor Color;

public:
	FSsParticleElementTransColor()
		: Color(FSsU8Color(255,255,255,255) , FSsU8Color(255,255,255,255))
	{
		MyType = SsEffectFunctionType::TransColor;
	}
};

//--------------------------------------------------------------------------------------
//頂点アルファを制御する
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementAlphaFade : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat Disprange;

public:
	FSsParticleElementAlphaFade()
		: Disprange(25,75)
	{
		MyType = SsEffectFunctionType::AlphaFade;
	}
};

//--------------------------------------------------------------------------------------
//サイズ初期
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementSize : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat SizeX;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat SizeY;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat ScaleFactor;

public:
	FSsParticleElementSize()
		: SizeX(1.0f , 1.0f)
		, SizeY(1.0f , 1.0f)
		, ScaleFactor(1.0f , 1.0f)
	{
		MyType = SsEffectFunctionType::Size;
	}
};

//--------------------------------------------------------------------------------------
//サイズ変更
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleElementTransSize : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat SizeX;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat SizeY;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FSsVarianceValueFloat ScaleFactor;

public:
	FSsParticleElementTransSize()
		: SizeX(1.0f , 1.0f)
		, SizeY(1.0f , 1.0f)
		, ScaleFactor(1.0f , 1.0f)
	{
		MyType = SsEffectFunctionType::TransSize;
	}
};

//--------------------------------------------------------------------------------------
//重力点
USTRUCT()
struct SPRITESTUDIO6_API FSsParticlePointGravity : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	FVector2D Position;

	UPROPERTY(VisibleAnywhere, Category=SsEffectElement)
	float Power;

public:
	FSsParticlePointGravity()
		: Position(0 , 0)
		, Power(0.0f)
	{
		MyType = SsEffectFunctionType::PointGravity;
	}
};

//--------------------------------------------------------------------------------------
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleTurnToDirectionEnabled : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	UPROPERTY(VisibleAnywhere, Category = SsEffectElement)
	float Rotation;

public:
	FSsParticleTurnToDirectionEnabled()
		: Rotation(0.f)
	{
		MyType = SsEffectFunctionType::TurnToDirectionEnabled;
	}
};

//--------------------------------------------------------------------------------------
USTRUCT()
struct SPRITESTUDIO6_API FSsParticleInfiniteEmitEnabled : public FSsEffectElementBase
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar) override;

public:
	FSsParticleInfiniteEmitEnabled()
	{
		MyType = SsEffectFunctionType::InfiniteEmitEnabled;
	}
};
