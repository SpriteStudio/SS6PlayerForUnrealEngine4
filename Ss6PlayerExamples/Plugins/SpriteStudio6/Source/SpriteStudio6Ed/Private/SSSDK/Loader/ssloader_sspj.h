#ifndef __SSLOADER_SSPJ__
#define __SSLOADER_SSPJ__


#include "sstypes.h"
#include "ssarchiver.h"
#include "ssstring_uty.h"

#define SPRITESTUDIO6_SSPJVERSION "2.00.00"

/// プロジェクトファイルの設定が記載されているデータです。
/// 以下のタグはエディタ編集用のデータなので読み飛ばします。
//	編集時設定のためよまない
//	<ssaxImport>						//!< .ssax インポート設定
//	<copyWhenImportImageIsOutside>	//!< プロジェクトフォルダ外に画像ファイルがある場合にインポート先にコピーする。
//	<exportAnimeFileFormat>			//!< エクスポート時のアニメーションファイルのフォーマット 
//	<exportCellMapFileFormat>			//!< エクスポート時のセルマップファイルのフォーマット
//	<exportCellMap>					//!< セルマップをエクスポートする
//	<copyImageWhenExportCellmap>		//!< セルマップのエクスポート時に画像ファイルをエクスポート先にコピーする
//	<ssaxExport>						//!< .ssax エクスポート設定
//	<player>					//!< 再生対象のプレイヤー。これにより使える機能に制限をかける。
//	<strictVer4>				//!< Ver4互換
//	<availableAttributes>		//!< 使用するアトリビュート
//	<defaultSetAttributes>	//!< 新規キー作成でデフォルトでキーが打たれるアトリビュート
class	SsProjectSetting
{
public:
	//他のメンバーもあるが、必要最低限のものを読み込む

	//アニメーションファイルの相対指定等
	SsString					animeBaseDirectory;			//!< アニメーションデータの読み書き基準ディレクトリ。
	SsString					cellMapBaseDirectory;		//!< セルマップデータの読み書き基準ディレクトリ。
	SsString					imageBaseDirectory;			//!< 画像データの読み込み元先基準ディレクトリ。
	SsString					effectBaseDirectory;


	SsString					exportBaseDirectory;		//!< エクスポート先の基準ディレクトリ。
	bool						queryExportBaseDirectory;	//!< エクスポート先の基準ディレクトリ指定をさせるか？ 
	SsTexWrapMode::_enum		wrapMode;					//!< テクスチャのラップモード
	SsTexFilterMode::_enum		filterMode;					//!< テクスチャのフィルタモード


	SsProjectSetting() {}
	
	///シリアライズのための宣言です。
	SSSERIALIZE_BLOCK
	{
		SSAR_DECLARE(animeBaseDirectory);
		SSAR_DECLARE(cellMapBaseDirectory);
		SSAR_DECLARE(imageBaseDirectory);
		SSAR_DECLARE(effectBaseDirectory);

		SSAR_DECLARE(exportBaseDirectory);
		SSAR_DECLARE(queryExportBaseDirectory);
		SSAR_DECLARE_ENUM( wrapMode );
		SSAR_DECLARE_ENUM( filterMode );
		
	}
};


class SsAnimation;
class SsAnimePack;
class SsCellMap;
class SsEffectFile;


typedef std::vector<SsAnimePack*> SsAnimePackList;
typedef std::vector<SsAnimePack*>::iterator SsAnimePackListItr;

typedef std::vector<SsCellMap*> SsSsCellMapList;
typedef std::vector<SsCellMap*>::iterator SsSsCellMapListItr;


typedef std::vector<SsEffectFile*> SsEffectFileList;
typedef std::vector<SsEffectFile*>::iterator SsEffectFileListItr;

/// XMLドキュメントとなっているsspjファイルのデータ保持を提供するクラスです。
///以下はエディタ情報のため読み飛ばします。
/// animeSettings   デフォルトのアニメーション設定 
/// texPackSettings デフォルトのテクスチャパッキング設定
class SsProject
{
public:

	SsString				version;
	SsProjectSetting		settings;			//!< プロジェクト設定
	std::vector<SsString>	cellmapNames;		//!< セルマップファイルのリスト
	std::vector<SsString>	animepackNames;		//!< アニメファイルのリスト
	std::vector<SsString>	effectFileNames;	//!< エフェクトファイルのリスト


	SsAnimePackList		animeList;		//!< アニメーションのリスト	
	SsSsCellMapList		cellmapList;	//!< セルマップリスト
	SsEffectFileList	effectfileList;

	//ロード時に作成されるワーク
	SsString	m_proj_filepath;	///プロジェクトファイルのパス


	SsProject(){}
	virtual ~SsProject();

	///プロジェクトの設定情報の取得
	const SsProjectSetting&	getProjectSetting(){ return settings;} 

	///アニメパックの数量を取得する
	const size_t getAnimePackNum(){ return animepackNames.size(); }

	///セルマップデータの数量を取得
	const size_t getCellMapNum(){ return cellmapNames.size(); }

	///セルマップデータの数量を取得
	const size_t getEffectFileNum(){ return effectFileNames.size(); }


	///アニメパックデータのコンテナを取得する
	SsAnimePackList&	getAnimePackList(){ return animeList;}

	///セルマップデータのコンテナを取得する
	SsSsCellMapList&	getCellMapList(){ return cellmapList;}

	//エフェクトファイルのリスト
	SsEffectFileList&	getEffectFileList(){ return effectfileList;}


	//アニメパック名とアニメ名からアニメーションを取得する
	SsAnimation*		findAnimation( SsString& animePackName , SsString& AnimeName );

	SsAnimePack*		findAnimationPack( SsString& animePackName );

	//エフェクトの名前からエフェクトを取得する
	SsEffectFile*		findEffect( SsString& effectName );

	



	SsCellMap* findCellMap( SsString& str );
	SsCellMap* getCellMap( int index );


	///シリアライズのための宣言です。
	SSSERIALIZE_BLOCK
	{
		SSAR_DECLARE_ATTRIBUTE(version);
		SSAR_STRUCT_DECLARE( settings );
		SSAR_DECLARE( cellmapNames );
		SSAR_DECLARE( animepackNames );
		SSAR_DECLARE( effectFileNames );

	}

public:

	///自身のファイルパスを設定する
	void	setFilepath( const SsString& path ) { m_proj_filepath = path; }

	///自身の読み込み元のファイルパスを取得する
	const	SsString&	getFilepath(){ return m_proj_filepath; }

	///ssceデータの読み込み元の基準パスを取得する。
	SsString	getSsceBasepath();

	///ssaeデータの読み込み元の基準パスを取得する。
	SsString	getSsaeBasepath();

	SsString	getSseeBasepath();

	SsString	getImageBasepath();


	///AnimePack(ssae)のファイル名をパス付きで取得する
	SsString	getAnimePackFilePath( size_t index ) { 
		if ( animepackNames.size() <= index ) return "";
		return getSsaeBasepath() + animepackNames[index];
	}


	///CellMap(ssce)のファイル名をパス付きで取得する
	SsString	getCellMapFilePath( size_t index ) { 
		if ( cellmapNames.size() <= index ) return "";
        SsString str = getSsceBasepath();


		std::string f = cellmapNames[index];
		
		std::string ret = getFullPath( str , path2dir( cellmapNames[index] ) );
		ret = ret + path2file( cellmapNames[index] );
		ret = nomarizeFilename(ret);

        //str = str + cellmapNames[index];
		//return str ;
		return ret;
	}

	SsString	getEffectFilePath( size_t index ) { 
		if ( effectFileNames.size() <= index ) return "";
		return getSseeBasepath() + effectFileNames[index];
	}

	SsString	getCelMapFileOriginalPath( size_t index ) { 
		if ( cellmapNames.size() <= index ) return "";
		return cellmapNames[index] ;
	}

};

///sspjのローダークラスです。
class ssloader_sspj
{
private:

public:
	ssloader_sspj(){}
	virtual ~ssloader_sspj(){}
	static SsProject*	Load(const std::string& filename );

};



#endif
