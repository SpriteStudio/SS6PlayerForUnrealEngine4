#ifndef __SSPLAYER_CELLMAP__
#define __SSPLAYER_CELLMAP__

#include "Ss6Project.h"


class SsAnimeDecoder;
class SsCelMapLinker;

///パーツが使用するセルの情報
struct SsCellValue
{
	FSsCell*					cell;		///参照しているセル
	UTexture*					texture;	///テクスチャ
	FVector2f					uvs[5];		///使用するUV座標
	SsTexWrapMode::Type			wrapMode;	///< テクスチャのラップモード
	SsTexFilterMode::Type		filterMode;	///< テクスチャのフィルタモード

	SsCellValue() :  
		cell(0) ,  
		texture(0)
		{}
};

class SsCelMapLinker
{
public:
	FSsCellMap*			cellMap;
	UTexture*	tex;

	TMap<FName,FSsCell*>	CellDic;

public:
	SsCelMapLinker()
		: cellMap(0) , tex(0)
	{}

	SsCelMapLinker(FSsCellMap* cellmap ,FString filePath )
	{

		cellMap = cellmap;
		size_t num = cellMap->Cells.Num();
		for ( size_t i = 0 ; i < num ; i++ )
		{
			CellDic.Add(cellMap->Cells[i].CellName, &cellMap->Cells[i]);
		}
/*
		if (!SSTextureFactory::isExist() )
		{
			puts( "SSTextureFactory not created yet." );
			throw;
		}

		tex = SSTextureFactory::create();

		//SsString fullpath = filePath + cellmap->imagePath;

		std::string fullpath = getFullPath( filePath , path2dir( cellmap->imagePath ) );
		fullpath = fullpath + path2file( cellmap->imagePath );
		fullpath = nomarizeFilename(fullpath);

		if ( !tex->Load( fullpath.c_str() ) )
		{
			delete tex;
			tex = 0;
		}
*/
		tex = cellmap->Texture;
		if((nullptr != tex) && (nullptr == tex->GetResource()))
		{
			tex->UpdateResource();
		}
	}

	virtual ~SsCelMapLinker()
	{
		CellDic.Empty();

//		if ( tex )
//			delete tex;
	}

	FSsCell*	findCell( const FName& name )
	{
		FSsCell** ppCell = CellDic.Find(name);
		return (nullptr != ppCell) ? *ppCell : nullptr;
	}
	
};

//プロジェクト全体で保持しているセルマップ
//現在はprojectのセルマップの列挙とssaeの列挙は同一
class	SsCellMapList
{
private:
	//同名セルマップは上書き
	TMap<FName,SsCelMapLinker*>		CellMapDic;
	TArray<SsCelMapLinker*>			CellMapList;//添え字参照用

///	typedef std::map<SsString,SsCelMapLinker*>::iterator CellMapDicItr;
	FString	CellMapPath;

private:
	void	addIndex(FSsCellMap* cellmap);
	void	addMap(FSsCellMap* cellmap);

public:
	SsCellMapList(){}
	virtual ~SsCellMapList()
	{
		clear();
	}

	void	clear();
	size_t	size(){ return CellMapList.Num(); }

	void	setCellMapPath(  const FString& filepath );

	//projectとanimepackからアニメーションの再生に必要なセルマップのリストを作成する
	//アニメパックのセルリストに登載されている順にセルマップを読み込みインデックス化する
	//SsProjectを介してセルを検索しているのはセルがそこにアレイで確保されているから
	//もし既に読み込み済みだったりする場合は、アニメパックのセルＩＤ順にセルマップを登録すればいい
	void	set(USs6Project* proj , FSsAnimePack* animepack );

	SsCelMapLinker*	getCellMapLink( const FName& name );
	SsCelMapLinker*	getCellMapLink( int index )
	{	
		if (CellMapList.Num() <= index) return 0;
		return CellMapList[index];
	}
	

};



//void getCellValue( int cellMapid , SsString& cellName , SsCellValue& v );
void getCellValue( SsCellMapList* cellList, int cellMapid , FName& cellName , SsCellValue& v );
void getCellValue( SsCellMapList* cellList, FName& cellMapName , FName& cellName , SsCellValue& v );

void calcUvs( SsCellValue* cellv, const FVector2f TexturePixelSize );

#endif
