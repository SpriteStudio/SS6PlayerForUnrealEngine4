#pragma once

#include "SsTypes.h"
#include "SsValue.h"

#include "SsAttribute.generated.h"


//アニメーション中のキーフレームの内容を表現するクラス
USTRUCT()
struct SPRITESTUDIO6_API FSsKeyframe
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	UPROPERTY(VisibleAnywhere, Category=SsKeyframe)
	int32	Time;	///< 時間

	UPROPERTY(VisibleAnywhere, Category=SsKeyframe)
	TEnumAsByte<SsInterpolationType::Type>	IpType;	///< 補間タイプ

	UPROPERTY(VisibleAnywhere, Category=SsKeyframe)
	FSsCurve	Curve;	///< 曲線補間計算用パラメータ

	UPROPERTY(VisibleAnywhere, Category=SsKeyframe)
	FSsValue	Value;	///< 値
public:
	FSsKeyframe()
		: Time(0)
		, IpType(SsInterpolationType::Invalid)
	{}
};


//タグ毎に存在するキーフレームを格納するクラス
USTRUCT()
struct SPRITESTUDIO6_API FSsAttribute	//Tag毎に存在する
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	//キーフレームデータ : Value（タグによって異なるの組み)
	UPROPERTY(VisibleAnywhere, Category=SsAttribute)
	TEnumAsByte<SsAttributeKind::Type> Tag;

	UPROPERTY(VisibleAnywhere, Category=SsAttribute)
	TArray<FSsKeyframe> Key;

public:
	bool isEmpty() const
	{
		return 0 == Key.Num();
	}

	const FSsKeyframe* FirstKey() const;

	///時間から左側のキーを取得
	const FSsKeyframe* FindLeftKey(int time) const;

	//時間から右側のキーを取得する
	const FSsKeyframe* FindRightKey(int time) const;

private:
//	int32 GetLowerBoundKeyIndex(int32 Time) const;	// std::map::lower_bound()代替
//	int32 GetUpperBoundKeyIndex(int32 Time) const;	// std::map::upper_bound()代替

};

void GetSsPartsColorValue(const FSsKeyframe* key , SsPartsColorAnime& v);
void GetSsColorValue(const FSsKeyframe* key , SsColorAnime& v);
void GetSsVertexAnime(const FSsKeyframe* key , FSsVertexAnime& v);
void GetSsRefCell(const FSsKeyframe* key , SsRefCell& v);
void GetSsUserDataAnime(const FSsKeyframe* key , SsUserDataAnime& v);
void GetSsInstparamAnime(const FSsKeyframe* key , SsInstanceAttr& v);
void GetSsEffectParamAnime(const FSsKeyframe* key , SsEffectAttr& v);
void GetSsDeformAnime(const FSsKeyframe* key, SsDeformAttr& v);

bool SPRITESTUDIO6_API StringToPoint2(const FString& str , FVector2f& point);
bool SPRITESTUDIO6_API StringToIRect(const FString& str , SsIRect& rect);
bool SPRITESTUDIO6_API StringToTriangle(const FString& str, FSsTriangle& tri);
