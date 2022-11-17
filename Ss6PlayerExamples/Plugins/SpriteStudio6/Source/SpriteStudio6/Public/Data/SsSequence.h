#pragma once

#include "SsTypes.h"
#include "SsSequence.generated.h"

class USs6Project;


USTRUCT()
struct SPRITESTUDIO6_API FSsSequenceItem
{
	GENERATED_USTRUCT_BODY()

public:
	// 参照アニメパック名 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	FName RefAnimPack;

	// 参照アニメ名 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	FName RefAnime;

	// 繰り返し再生回数 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	int32 RepeatCount = 0;
};

USTRUCT()
struct SPRITESTUDIO6_API FSsSequence
{
	GENERATED_USTRUCT_BODY()

public:
	// シーケンスの名称 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	FName SequenceName;

	// このシーケンスがもつ固有の番号 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	int32 Id = 0;

	// このシーケンスのタイプ 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	TEnumAsByte<SsSequenceType::Type> Type = SsSequenceType::Invalid;

	// このシーケンスがもつアイテムのリスト 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	TArray<FSsSequenceItem> List;


	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	int32 SequenceFPS = 0;

	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	int32 SequenceFrameCount = 0;

public:
	bool CalcSequenceFpsAndFrameCount(const USs6Project* OwnerProject, int32& OutFPS, int32& OutFrameCount);
	bool GetAnimationBySequenceFrame(const USs6Project* OwnerProject, int32 SequenceFrame, int32& OutAnimePackIndex, int32& OutAnimationIndex, int32& OutItemIndex, int32& OutRepeatCount, int32& OutAnimationFrame);
};

/**
*@class SsSequencePack
*@brief アニメーションを組み合わせた構造とその構造を使用するシーケンスを格納するデータです。
アニメーションの組み合わせ構造をSsSequenceで定義しています。
*/
USTRUCT()
struct SPRITESTUDIO6_API FSsSequencePack
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	FString Version;

	// シーケンスパック名称 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	FName SequencePackName;

	// 格納されている子シーケンスのリスト 
	UPROPERTY(VisibleAnywhere, Category=SsSequence)
	TArray<FSsSequence> SequenceList;

public:
	// シーケンス名からシーケンスを取得する 
	const FSsSequence* FindSequence(const FName& InName) const;

	// シーケンス名からシーケンスインデックスを取得する 
	int32 FindSequenceIndex(const FName& InName) const;

};
