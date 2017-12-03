#pragma once

#include "SsTypes.h"
#include "SsAttribute.h"

#include "SsAnimePack.generated.h"


/// アニメーション再生設定情報です。
USTRUCT()
struct SPRITESTUDIO6_API FSsAnimationSettings
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsAnimationSettings)
	int32	Fps;			//!< 再生FPS

	UPROPERTY(VisibleAnywhere, Category=SsAnimationSettings)
	int32	FrameCount;		//!< フレーム数

	UPROPERTY(VisibleAnywhere, Category=SsAnimationSettings)
	FVector2D	CanvasSize;				//!< キャンバスサイズ(元基準枠)。ビューポートのサイズとイコールではない。

	UPROPERTY(VisibleAnywhere, Category=SsAnimationSettings)
	FVector2D	Pivot;					//!< キャンバスの原点。0,0 が中央。-0.5, +0.5 が左上

	UPROPERTY(VisibleAnywhere, Category=SsAnimationSettings)
	int32	StartFrame;		//!< アニメーションの開始フレーム

	UPROPERTY(VisibleAnywhere, Category=SsAnimationSettings)
	int32 EndFrame;			//!< アニメーションの終了フレーム
};



//パーツ一つあたりの情報を保持するデータです。
USTRUCT()
struct SPRITESTUDIO6_API FSsPart
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsPart)
	FName	PartName;

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	int32	ArrayIndex;		//!< ツリーを配列に展開した時のインデックス

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	int32	ParentIndex;	//!< 親パーツのインデックス

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	TEnumAsByte<SsPartType::Type>		Type;			//!< 種別

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	TEnumAsByte<SsBoundsType::Type>		BoundsType;		//!< 当たり判定として使うか？使う場合はその形状。

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	TEnumAsByte<SsInheritType::Type>	InheritType;	//!< アトリビュート値の継承方法

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	TEnumAsByte<SsBlendType::Type>		AlphaBlendType;	//!< αブレンドの演算式

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	int32	Show;			//!< [編集用データ] パーツの表示・非常時

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	int32	Locked;			//!< [編集用データ] パーツのロック状態

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	FName	ColorLabel;		//!< カラーラベル

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	bool	MaskInfluence;	//!< マスクの影響を受けるかどうか

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	float InheritRates[(int32)SsAttributeKind::Num];	///< 親の値の継承率。SS4との互換性のため残されているが0 or 1


	UPROPERTY(VisibleAnywhere, Category=SsPart)
	FName	RefAnimePack;   ///< 参照アニメ名

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	FName	RefAnime;       ///< 参照アニメ名

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	FName	RefEffectName;	///< 割り当てたパーティクル名


	UPROPERTY(VisibleAnywhere, Category=SsPart)
	int32		BoneLength;		//!< ボーンの長さ

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	FVector2D	BonePosition;	//!< ボーンの座標

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	float		BoneRotation;	//!< ボーンの角度

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	FVector2D	WeightPosition;	//!< ウェイトの位置

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	float		WeightImpact;	//!< ウェイトの強さ


	UPROPERTY(VisibleAnywhere, Category=SsPart)
	int32		IKDepth;		//!< IK深度

	UPROPERTY(VisibleAnywhere, Category=SsPart)
	TEnumAsByte<SsIkRotationArrow::Type>	IKRotationArrow;	//!< 回転方向


public:
	FSsPart()
		: PartName(TEXT("")), ArrayIndex(0), ParentIndex(0), Show(0), Locked(0), MaskInfluence(true)
	{
		BoneLength = 0;
		BonePosition = FVector2D::ZeroVector;
		BoneRotation = 0.f;
		WeightPosition = FVector2D::ZeroVector;
		WeightImpact = 0.f;
		IKDepth = 0;
		IKRotationArrow = SsIkRotationArrow::Arrowfree;

		for (int i = 0; i < (int32)SsAttributeKind::Num; ++i)
		{
			InheritRates[i] = 1.f;
		}

		// イメージ反転は継承しない
		InheritRates[(int32)SsAttributeKind::Imgfliph] = 0.f;
		InheritRates[(int32)SsAttributeKind::Imgflipv] = 0.f;
	}
};


#define SSMESHPART_BONEMAX	(128)

USTRUCT()
struct SPRITESTUDIO6_API FSsMeshBindInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()	// 要素数が多くDetailsウィンドウが極端に重くなってしまうため、VisibleAnywhereを付けない 
	int32	Weight[SSMESHPART_BONEMAX];

	UPROPERTY()	// 要素数が多くDetailsウィンドウが極端に重くなってしまうため、VisibleAnywhereを付けない 
	FName	BoneName[SSMESHPART_BONEMAX];

	UPROPERTY()	// 要素数が多くDetailsウィンドウが極端に重くなってしまうため、VisibleAnywhereを付けない 
	int32	BoneIndex[SSMESHPART_BONEMAX];

	UPROPERTY()	// 要素数が多くDetailsウィンドウが極端に重くなってしまうため、VisibleAnywhereを付けない 
	FVector	Offset[SSMESHPART_BONEMAX];

	UPROPERTY(VisibleAnywhere, Category=SsMeshBindInfo)
	int32	BindBoneNum;

	FSsMeshBindInfo()
	{
		for(int32 i = 0; i < SSMESHPART_BONEMAX; ++i)
		{
			Weight[i] = 0;
			BoneIndex[i] = 0;
			Offset[i] = FVector::ZeroVector;
		}
		BindBoneNum = 0;
	}
};

//メッシュ1枚毎の情報
USTRUCT()
struct SPRITESTUDIO6_API FSsMeshBind
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category=SsMeshBind)
	FString	MeshName;

	UPROPERTY(VisibleAnywhere, Category=SsMeshBind)
	TArray<FSsMeshBindInfo>	MeshVerticesBindArray;
};


//アニメーションを構成するパーツをリスト化したものです。
USTRUCT()
struct SPRITESTUDIO6_API FSsModel
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsModel)
	TArray<FSsPart>	PartList;	//!<格納されているパーツのリスト

	struct FSsAnimation*	SetupAnimation;	//< 参照するセットアップアニメ

	UPROPERTY(VisibleAnywhere, Category=SsModel)
	TArray<FSsMeshBind>		MeshList;

	UPROPERTY(VisibleAnywhere, Category=SsModel)
	TMap<FName, int32>		BoneList;
};


USTRUCT()
struct SPRITESTUDIO6_API FSsPartAnime
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	UPROPERTY(VisibleAnywhere, Category=SsPartAnime)
	FName PartName;

	UPROPERTY(VisibleAnywhere, Category=SsPartAnime)
	TArray<FSsAttribute> Attributes;
};


/// ラベル。ループ先などに指定する
USTRUCT()
struct SPRITESTUDIO6_API FSsLabel
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsLabel)
	FName		LabelName;	///< 名前 [変数名変更禁止]

	UPROPERTY(VisibleAnywhere, Category=SsLabel)
	int32		Time;		///< 設置された時間(フレーム) [変数名変更禁止]
};


USTRUCT()
struct SPRITESTUDIO6_API FSsAnimation
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	UPROPERTY(VisibleAnywhere, Category=SsAnimation)
	FName	AnimationName;					/// アニメーションの名称

	UPROPERTY(VisibleAnywhere, Category=SsAnimation)
	bool	OverrideSettings;				/// このインスタンスが持つ設定を使いanimePack の設定を参照しない。FPS, frameCount は常に自身の設定を使う。

	UPROPERTY(VisibleAnywhere, Category=SsAnimation)
	FSsAnimationSettings	Settings;		/// 設定情報

	UPROPERTY()	// 要素数が多くDetailsウィンドウが極端に重くなってしまうため、VisibleAnywhereを付けない
	TArray<FSsPartAnime>	PartAnimes;		///	パーツ毎のアニメーションキーフレームが格納されるリスト

	UPROPERTY(VisibleAnywhere, Category=SsAnimation)
	TArray<FSsLabel>		Labels;			/// アニメーションが持つラベルのリストです。

	UPROPERTY(VisibleAnywhere, Category=SsAnimation)
	bool	IsSetup;						///< セットアップアニメか？
};

/**
*@class SsAnimePack
*@brief パーツを組み合わせた構造とその構造を使用するアニメーションを格納するデータです。
パーツの組み合わせ構造をSsModel、Modelを使用するアニメデータをSsAnimationで定義しています。
*/
USTRUCT()
struct SPRITESTUDIO6_API FSsAnimePack
{
	GENERATED_USTRUCT_BODY()

	void Serialize(FArchive& Ar);

public:
	UPROPERTY(VisibleAnywhere, Category=SsAnimePack)
	FString					Version;

	UPROPERTY(VisibleAnywhere, Category=SsAnimePack)
	FSsAnimationSettings	Settings;		//!< 設定情報 

	UPROPERTY(VisibleAnywhere, Category=SsAnimePack)
	FName					AnimePackName;	//!< アニメーションパック名称

	UPROPERTY(VisibleAnywhere, Category=SsAnimePack)
	FSsModel				Model;			//!< パーツ情報の格納先

	UPROPERTY(VisibleAnywhere, Category=SsAnimePack)
	TArray<FName>			CellmapNames;	//!< 使用されているセルマップの名称		

	UPROPERTY(VisibleAnywhere, Category=SsAnimePack)
	TArray<FSsAnimation>	AnimeList;		//!< 格納されている子アニメーションのリスト

	// アニメーション名からインデックスを取得する
	int32 FindAnimationIndex(const FName& Name) const;

	// 名前からアニメーションを取得する
	const FSsAnimation* FindAnimation(const FName& Name) const;
};
