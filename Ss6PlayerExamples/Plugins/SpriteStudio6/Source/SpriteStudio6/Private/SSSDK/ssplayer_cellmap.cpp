#include "SpriteStudio6PrivatePCH.h"


#include <stdio.h>
#include <cstdlib>

//#include "../Loader/ssloader.h"

#include "ssplayer_animedecode.h"
#include "ssplayer_matrix.h"
//#include "ssplayer_render.h"

//#include "../Helper/DebugPrint.h"


void	SsCellMapList::clear()
{
	if (CellMapDic.Num() > 0)
	{
		for(auto itr = CellMapDic.CreateIterator(); itr; ++itr)
		{
			if (itr->Value != NULL)
			{
				delete itr->Value;
				itr->Value = NULL;
				continue;
			}
		}
	}

	CellMapDic.Empty();
	CellMapList.Empty();
}


void	SsCellMapList::setCellMapPath(  const FString& filepath )
{
	CellMapPath = filepath;
}

void	SsCellMapList::set(USs6Project* proj , FSsAnimePack* animepack )
{
	clear();
	setCellMapPath( proj->GetImageBasepath() );

	for ( size_t i = 0 ; i < animepack->CellmapNames.Num() ; i++ )
	{
		FSsCellMap* cell = const_cast<FSsCellMap*>(proj->FindCellMap( animepack->CellmapNames[i] ));
		if ( cell==0 )
		{
			UE_LOG(LogSpriteStudio, Warning, TEXT(" Not found cellmap = %s"), *(animepack->CellmapNames[i].ToString()));
		}else{
			addIndex( cell );
		}
	}

	for ( size_t i = 0 ; i < proj->CellmapNames.Num() ; i++ )
	{
		FSsCellMap* cell = const_cast<FSsCellMap*>(proj->FindCellMap( proj->CellmapNames[i] ));
		if ( cell==0 )
		{
			UE_LOG(LogSpriteStudio, Warning, TEXT(" Not found cellmap = %s"), *(animepack->CellmapNames[i].ToString()));
		}else{
			addMap( cell );
		}
	}


}
void	SsCellMapList::addMap(FSsCellMap* cellmap)
{
	SsCelMapLinker* linker = new SsCelMapLinker(cellmap , this->CellMapPath );
	CellMapDic.Add(FName(*(cellmap->CellMapName.ToString()+TEXT(".ssce"))), linker);

}

void	SsCellMapList::addIndex(FSsCellMap* cellmap)
{
	SsCelMapLinker* linker = new SsCelMapLinker(cellmap , this->CellMapPath );
	CellMapList.Add( linker );

}

SsCelMapLinker*	SsCellMapList::getCellMapLink( const FName& name )
{

	SsCelMapLinker** linker = CellMapDic.Find(name);
	if ( nullptr != linker )
	{
		return *linker;
	}else{

#if 0
		std::vector<SsString> slist;
		split_string( name , '.' , slist );
		
		std::map<SsString,SsCelMapLinker*>::iterator itr = CellMapDic.find(slist[0]);
		if ( itr != CellMapDic.end() )
		{
			return itr->second;
		}else{
			DEBUG_PRINTF( "CellMapName not found : %s " , name.c_str() );
		}
#endif

/*		for ( auto itr = CellMapDic.CreateConstIterator(); itr; ++itr)
		{
			if ( itr->Value->cellMap->loadFilepath == name )
			{
				return itr->second;
			}
		}
*/

	}

	return 0;

}


void getCellValue( SsCelMapLinker* l, FName& cellName , SsCellValue& v )
{
	v.cell = l->findCell( cellName );

	v.filterMode = l->cellMap->FilterMode;
	v.wrapMode = l->cellMap->WrapMode;

	if ( l->tex )
	{
		v.texture = l->tex;
	}
	else
	{
		v.texture = 0;
	}

	calcUvs( &v );
}

void getCellValue( SsCellMapList* cellList, FName& cellMapName , FName& cellName , SsCellValue& v )
{
	SsCelMapLinker* l = cellList->getCellMapLink( cellMapName );
	getCellValue( l , cellName , v );


}

void getCellValue( SsCellMapList* cellList, int cellMapid , FName& cellName , SsCellValue& v )
{
	SsCelMapLinker* l = cellList->getCellMapLink( cellMapid );
	getCellValue( l , cellName , v );


}

void calcUvs( SsCellValue* cellv )
{
	//SsCellMap* map = cellv->cellmapl->cellMap;
	FSsCell* cell = cellv->cell;
	if ( cellv->texture == 0 ) return ;

//	if ( cell == 0 || map == 0)
	if ( cell == 0 )
//	if ( ( cell == 0 ) || ( cellv->texture == 0 ) )	//koizumi change
	{
		cellv->uvs[0].X = cellv->uvs[0].Y = 0;
		cellv->uvs[1].X = cellv->uvs[1].Y = 0;
		cellv->uvs[2].X = cellv->uvs[2].Y = 0;
		cellv->uvs[3].X = cellv->uvs[3].Y = 0;
		return;
	}

	FVector2D wh;
	wh.X = (float)cellv->texture->GetSurfaceWidth();
	wh.Y = (float)cellv->texture->GetSurfaceHeight();

//	SsVector2 wh = map->pixelSize;
	// 右上に向かって＋になる
	float left = cell->Pos.X / wh.X;
	float right = (cell->Pos.X + cell->Size.X) / wh.X;


	// LB->RB->LT->RT 順
	// 頂点をZ順にしている都合上UV値は上下逆転させている
	float top = cell->Pos.Y / wh.Y;
	float bottom = ( cell->Pos.Y + cell->Size.Y) / wh.Y;

	if (cell->Rotated)
	{
		// 反時計回りに９０度回転されているため起こして描画されるようにしてやる。
		// 13
		// 02
		cellv->uvs[0].X = cellv->uvs[1].X = left;
		cellv->uvs[2].X = cellv->uvs[3].X = right;
		cellv->uvs[1].Y = cellv->uvs[3].Y = top;
		cellv->uvs[0].Y = cellv->uvs[2].Y = bottom;
	}
	else
	{
		// そのまま。頂点の順番は下記の通り
		// 01
		// 23
		cellv->uvs[0].X = cellv->uvs[2].X = left;
		cellv->uvs[1].X = cellv->uvs[3].X = right;
		cellv->uvs[0].Y = cellv->uvs[1].Y = top;
		cellv->uvs[2].Y = cellv->uvs[3].Y = bottom;
	}
}