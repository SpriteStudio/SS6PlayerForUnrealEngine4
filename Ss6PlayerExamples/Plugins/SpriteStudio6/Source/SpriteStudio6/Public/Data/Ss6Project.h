#pragma once

#include "SsTypes.h"
#include "SsAnimePack.h"
#include "SsCellMap.h"
#include "SsEffectFile.h"
#include "SsSequence.h"

#include "Ss6Project.generated.h"



/// プロジェクトファイルの設定が記載されているデータです。
/// 以下のタグはエディタ編集用のデータなので読み飛ばします。
//	編集時設定のためよまない
//	<ssaxImport>						//!< .ssax インポート設定
//	<copyWhenImportImageIsOutside>		//!< プロジェクトフォルダ外に画像ファイルがある場合にインポート先にコピーする。
//	<exportAnimeFileFormat>				//!< エクスポート時のアニメーションファイルのフォーマット 
//	<exportCellMapFileFormat>			//!< エクスポート時のセルマップファイルのフォーマット
//	<exportCellMap>						//!< セルマップをエクスポートする
//	<copyImageWhenExportCellmap>		//!< セルマップのエクスポート時に画像ファイルをエクスポート先にコピーする
//	<ssaxExport>						//!< .ssax エクスポート設定
//	<player>							//!< 再生対象のプレイヤー。これにより使える機能に制限をかける。
//	<strictVer4>						//!< Ver4互換
//	<availableAttributes>				//!< 使用するアトリビュート
//	<defaultSetAttributes>				//!< 新規キー作成でデフォルトでキーが打たれるアトリビュート
USTRUCT()
struct SPRITESTUDIO6_API FSs6ProjectSetting
{
	GENERATED_USTRUCT_BODY()

public:
	//他のメンバーもあるが、必要最低限のものを読み込む

	//アニメーションファイルの相対指定等
	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	FString		AnimeBaseDirectory;			//!< アニメーションデータの読み書き基準ディレクトリ。

	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	FString		CellMapBaseDirectory;		//!< セルマップデータの読み書き基準ディレクトリ。

	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	FString		ImageBaseDirectory;			//!< 画像データの読み込み元先基準ディレクトリ。

	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	FString		EffectBaseDirectory;		//!< エフェクトデータの読み込み元先基準ディレクトリ。

	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	FString		ExportBaseDirectory;		//!< エクスポート先の基準ディレクトリ。

	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	bool		QueryExportBaseDirectory;	//!< エクスポート先の基準ディレクトリ指定をさせるか？ 

	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	TEnumAsByte<SsTexWrapMode::Type>	WrapMode;			//!< テクスチャのラップモード

	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	TEnumAsByte<SsTexFilterMode::Type>	FilterMode;			//!< テクスチャのフィルタモード

	UPROPERTY(VisibleAnywhere, Category=SsProjectSetting)
	int32 VertexAnimeFloat;									//!< 頂点変形アトリビュートの少数対応
};


/// XMLドキュメントとなっているsspjファイルのデータ保持を提供するクラスです。
///以下はエディタ情報のため読み飛ばします。
/// animeSettings   デフォルトのアニメーション設定 
/// texPackSettings デフォルトのテクスチャパッキング設定
UCLASS()
class SPRITESTUDIO6_API USs6Project : public UObject
{
	GENERATED_UCLASS_BODY()

	virtual void Serialize(FArchive& Ar) override;

public:
	UFUNCTION(BlueprintPure, Category=SsProject)
	TArray<FName> GetAnimPackNames() const;

	UFUNCTION(BlueprintPure, Category=SsProject)
	TArray<FName> GetAnimationNames(FName InAnimPackName) const;

public:
	UPROPERTY(VisibleAnywhere, Category=SsProject)
	FString				Version;

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	FSs6ProjectSetting	Settings;			//!< プロジェクト設定

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	TArray<FName>		CellmapNames;		//!< セルマップファイルのリスト

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	TArray<FName>		AnimepackNames;		//!< アニメファイルのリスト

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	TArray<FName>		EffectFileNames;	//!< エフェクトファイルのリスト

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	TArray<FName>		SequencePackNames;	//!< シーケンスファイルのリスト

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	TArray<FSsAnimePack>	AnimeList;		//!< アニメーションのリスト

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	TArray<FSsCellMap>		CellmapList;	//!< セルマップリスト

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	TArray<FSsEffectFile>	EffectList;		//!< エフェクトリスト

	UPROPERTY(VisibleAnywhere, Category=SsProject)
	TArray<FSsSequencePack>	SequenceList;	//!< シーケンスのリスト

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Instanced, Category = Reimport)
	class UAssetImportData* AssetImportData;
#endif

	//ロード時に作成されるワーク
	FString		ProjFilepath;	///プロジェクトファイルのパス


public:	
	///プロジェクトの設定情報の取得
	const FSs6ProjectSetting& GetProjectSetting(){ return Settings; }

	// アニメパック名からアニメーションインデックスを取得する
	int32 FindAnimePackIndex(const FName& AnimePackName) const;

	// セルマップ名からセルマップインデックスを取得する
	int32 FindCellMapIndex(const FName& CellMapName) const;

	// アニメーション名からインデックスを取得する
	bool FindAnimationIndex(const FName& InAnimPackName, const FName& InAnimationName, int32& OutAnimPackIndex, int32& OutAnimationIndex) const;

	// インデックスからアニメーションを取得する
	const FSsAnimation* FindAnimation(int32 AnimPackIndex, int32 AnimationIndex) const;

	// アニメパック名とアニメ名からアニメーションを取得する
	const FSsAnimation* FindAnimation(const FName& InAnimPackName, const FName& InAnimationName) const;

	// 名前からセルマップを取得する
	const FSsCellMap* FindCellMap(const FName& InCellMapName) const;

	// 名前からアニメパックを取得する
	const FSsAnimePack* FindAnimationPack(const FName& InAnimePackName) const;

	// エフェクト名からインデックスを取得する
	int32 FindEffectIndex(const FName& EffectName) const;

	// 名前からエフェクトを取得する
	const FSsEffectFile* FindEffect(const FName& EffectName) const;


	
	///自身のファイルパスを設定する
	void SetFilepath(const FString& path){ ProjFilepath = path; }

	///自身の読み込み元のファイルパスを取得する
	const FString& GetFilepath() const { return ProjFilepath; }

	///ssceデータの読み込み元の基準パスを取得する。 
	FString GetSsceBasepath() const;

	///ssaeデータの読み込み元の基準パスを取得する。 
	FString GetSsaeBasepath() const;

	///エフェクトデータの読み込み元の基準パスを取得する。 
	FString GetSseeBasepath() const;

	///テクスチャデータの読み込み元の基準パスを取得する。 
	FString GetImageBasepath() const;

	///最大描画パーツ数を計算
	void CalcMaxVertexAndIndexNum(uint32& OutMaxVertexNum, uint32& OutMaxIndexNum) const;

	///マスクパーツを含むか
	bool ContainsMaskParts() const;
};
