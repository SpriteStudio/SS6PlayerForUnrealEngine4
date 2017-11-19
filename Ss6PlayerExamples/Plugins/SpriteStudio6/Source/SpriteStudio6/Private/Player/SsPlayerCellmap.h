#pragma once

class FSsAnimeDecoder;
class FSsCelMapLinker;
struct FSsCellMap;
struct FSsCell;

///パーツが使用するセルの情報
struct FSsCellValue
{
	FSsCell*					Cell;		///参照しているセル
	UTexture*					Texture;	///テクスチャ
	FVector2D					Uvs[5];		///使用するUV座標
	SsTexWrapMode::Type			WrapMode;	///< テクスチャのラップモード
	SsTexFilterMode::Type		FilterMode;	///< テクスチャのフィルタモード

	FSsCellValue()
		: Cell(0)
		, Texture(0)
		, WrapMode(SsTexWrapMode::Clamp)
		, FilterMode(SsTexFilterMode::Nearest)
	{
		for(int i = 0; i < 5; ++i)
		{
			Uvs[i].X = Uvs[i].Y = 0.f;
		}
	}
};

class FSsCelMapLinker
{
public:
	FSsCellMap*		CellMap;
	UTexture*		Tex;

	TMap<FName,FSsCell*>	CellDic;

public:
	FSsCelMapLinker();
	FSsCelMapLinker(FSsCellMap* cellmap, FName filePath);
	virtual ~FSsCelMapLinker();

	FSsCell* FindCell(const FName& name)
	{
		if(name.IsNone())
		{
			return NULL;
		}
		FSsCell** Ret = CellDic.Find(name);
		return (nullptr == Ret) ? NULL : *Ret;
	}
};

//プロジェクト全体で保持しているセルマップ
//現在はprojectのセルマップの列挙とssaeの列挙は同一
class	FSsCellMapList
{
private:
	//同名セルマップは上書き
	TMap<FName,FSsCelMapLinker*>		CellMapDic;
	TArray<FSsCelMapLinker*>			CellMapList;//添え字参照用

	FName	CellMapPath;

private:
	void	Add(FSsCellMap* cellmap);

public:
	FSsCellMapList(){}
	virtual ~FSsCellMapList()
	{
		for(auto itr = CellMapDic.CreateIterator(); itr; ++itr)
		{
			delete (*itr).Value;
		}
		CellMapDic.Empty();
	}

	void	Clear();
	size_t	Size(){ return CellMapList.Num(); }

	void	SetCellMapPath(const FName& filepath);

	//projectとanimepackからアニメーションの再生に必要なセルマップのリストを作成する
	//アニメパックのセルリストに登載されている順にセルマップを読み込みインデックス化する
	//SsProjectを介してセルを検索しているのはセルがそこにアレイで確保されているから
	//もし既に読み込み済みだったりする場合は、アニメパックのセルＩＤ順にセルマップを登録すればいい
	void	Set(class USs6Project* proj , struct FSsAnimePack* animepack );

	FSsCelMapLinker*	GetCellMapLink( const FName& name );
	FSsCelMapLinker*	GetCellMapLink( int index )
	{	
		return CellMapList[index];
	}
	

};

void GetCellValue(FSsCellMapList* cellList, int32 cellMapid, FName& cellName, FSsCellValue& v);
void GetCellValue(FSsCellMapList* cellList, FName& cellMapName, FName& cellName, FSsCellValue& v);

void CalcUvs( FSsCellValue* cellv );

