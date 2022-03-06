#pragma once

#include "SsTypes.h"

#include "SsCellMap.generated.h"


///パーツに使用される画素の矩形範囲を示した構造です。
USTRUCT()
struct SPRITESTUDIO6_API FSsCell 
{
	GENERATED_USTRUCT_BODY()

public:
	//--------- ランタイム用データとして保存すべきもの
	UPROPERTY(VisibleAnywhere, Category=SsCell)
	FName		CellName;		///< セル名称

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	FVector2f	Pos;			///< 左上の座標

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	FVector2f	Size;			///< WHピクセルサイズ

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	FVector2f	Pivot;			///< 原点。size /2 が中央=0,0になる。

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	bool		Rotated;		///< 左方向に９０度回転されている。uvs の割り当てが変わる。

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	FVector2f	ParentSize;		//親テクスチャのサイズ

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	bool		IsMesh;


	UPROPERTY(VisibleAnywhere, Category=SsCell)
	TArray<FVector2f>	InnerPoint;

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	TArray<FVector2f>	OuterPoint;

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	TArray<FVector2f>	MeshPointList;		//ポイントリスト

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	TArray<FSsTriangle>	MeshTriList;		//トライアングルリスト

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	TEnumAsByte<SsMeshDivType::Type>	DivType;

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	int32	DivW;

	UPROPERTY(VisibleAnywhere, Category=SsCell)
	int32	DivH;
};



//!セルマップデータを表現するためのクラスです。
/*!
@class SsCellMap
@breif セルマップは１つのテクスチャファイルとそのテクスチャ内でパーツとして使用する矩形範囲を示したセルをコンテナとして保持するための構造です。<BR>
<BR>
このデータコンテナはエディット用として下記を読み飛ばします。<BR>
imagePathAtImport;///< インポート時の参照元画像のフルパス<BR>
packInfoFilePath;	///< パック情報ファイル。TexturePacker 等のデータをインポートした場合のみ有効<BR>
texPackSettings;	///< パック時の参照情報<BR>
*/
USTRUCT()
struct SPRITESTUDIO6_API FSsCellMap
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	FString		Version;

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	FName		FileName;			///< セルマップのファイルネーム

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	FName		CellMapName;		///< このセルマップの名称です。

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	FName		CellMapNameEx;		///< 拡張子付きセルマップ名

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	FString		ImagePath;			///< 参照画像ファイルパス。プロジェクトの画像基準相対

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	UTexture*	Texture = nullptr;

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	FVector2f	PixelSize;			///< 画像のピクセルWHサイズ

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	bool		OverrideTexSettings;///< テクスチャ設定をプロジェクトの設定ではなく下記設定を使う

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	TEnumAsByte<SsTexWrapMode::Type>	WrapMode;			///< テクスチャのラップモード

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	TEnumAsByte<SsTexFilterMode::Type>	FilterMode;			///< テクスチャのフィルタモード

	UPROPERTY(VisibleAnywhere, Category=SsCellMap)
	TArray<FSsCell>		Cells;
};


