#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerCellmap.h"

#include "Ss6Project.h"
#include "SsAnimePack.h"
#include "SsCellMap.h"

#include "SsPlayerAnimedecode.h"
#include "SsPlayerMatrix.h"


FSsCelMapLinker::FSsCelMapLinker()
	: CellMap(0)
	, Tex(0)
{
}

FSsCelMapLinker::FSsCelMapLinker(FSsCellMap* cellmap, FName filePath)
{
	CellMap = cellmap;
	size_t num = CellMap->Cells.Num();
	for ( size_t i = 0 ; i < num ; i++ )
	{
		if(!CellDic.Contains(CellMap->Cells[i].CellName))
		{
			CellDic.Add(CellMap->Cells[i].CellName);
		}
		CellDic[CellMap->Cells[i].CellName] = &(CellMap->Cells[i]);
	}

	Tex = cellmap->Texture;
	if(NULL == Tex->Resource)
	{
		Tex->UpdateResource();
	}
}

FSsCelMapLinker::~FSsCelMapLinker()
{
	CellDic.Empty();
}


void FSsCellMapList::Clear()
{
	CellMapDic.Empty();
	CellMapList.Empty();
}


void FSsCellMapList::SetCellMapPath(const FName& filepath)
{
	CellMapPath = filepath;
}


void FSsCellMapList::Set(USs6Project* proj , FSsAnimePack* animepack)
{
	Clear();
	SetCellMapPath( FName(*(proj->GetImageBasepath())) );

	for(int32 i = 0 ; i < animepack->CellmapNames.Num() ; i++)
	{
		int32 CellmapIndex = proj->FindCellMapIndex(animepack->CellmapNames[i]);
		if(0 <= CellmapIndex)
		{
			Add(&proj->CellmapList[CellmapIndex]);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(" Not found cellmap = %s"), *(animepack->CellmapNames[i].ToString()));
		}
	}

	for(int32 i = 0; i < animepack->Model.PartList.Num(); ++i)
	{
		if(SsPartType::Effect == animepack->Model.PartList[i].Type)
		{
			int32 EffectIndex = proj->FindEffectIndex(animepack->Model.PartList[i].RefEffectName);
			if(0 <= EffectIndex)
			{
				for(int32 j = 0; j < proj->EffectList[EffectIndex].EffectData.NodeList.Num(); ++j)
				{
					int32 CellmapIndex = proj->FindCellMapIndex(proj->EffectList[EffectIndex].EffectData.NodeList[j].Behavior.CellMapName);
					if(0 <= CellmapIndex)
					{
						Add(&(proj->CellmapList[CellmapIndex]));
					}
				}
			}
		}
	}

}

void FSsCellMapList::Add(FSsCellMap* cellmap)
{
	FName CellMapNameEx = FName( *(cellmap->CellMapName.ToString() + TEXT(".ssce")) );
	if(!CellMapDic.Contains(CellMapNameEx))
	{
		FSsCelMapLinker* linker = new FSsCelMapLinker(cellmap , this->CellMapPath);
		CellMapDic.Add(CellMapNameEx);
		CellMapDic[ CellMapNameEx ] = linker ;
		CellMapList.Add( linker );
	}
}

FSsCelMapLinker* FSsCellMapList::GetCellMapLink(const FName& name)
{
	if(name.IsNone())
	{
		return nullptr;
	}

	FSsCelMapLinker* l = CellMapDic[name];
	if(l)
	{
		return l;
	}

	return nullptr;
}


void GetCellValue(FSsCelMapLinker* l, FName& cellName, FSsCellValue& v)
{
	v.Cell = l->FindCell( cellName );

	v.FilterMode = l->CellMap->FilterMode;
	v.WrapMode = l->CellMap->WrapMode;

	if ( l->Tex )
	{
		v.Texture = l->Tex;
	}
	else
	{
		v.Texture = 0;
	}

	CalcUvs( &v );
}
void GetCellValue(FSsCellMapList* cellList, FName& cellMapName, FName& cellName, FSsCellValue& v)
{
	FSsCelMapLinker* l = cellList->GetCellMapLink( cellMapName );
	GetCellValue( l , cellName , v );
}
void GetCellValue(FSsCellMapList* cellList, int32 cellMapid, FName& cellName, FSsCellValue& v)
{
	FSsCelMapLinker* l = cellList->GetCellMapLink( cellMapid );
	GetCellValue( l , cellName , v );
}

void CalcUvs( FSsCellValue* cellv )
{
	//SsCellMap* map = cellv->cellmapl->cellMap;
	FSsCell* cell = cellv->Cell;
	if(cellv->Texture == nullptr)
	{
		return;
	}

	if ( cell == 0 )
	{
		cellv->Uvs[0].X = cellv->Uvs[0].Y = 0;
		cellv->Uvs[1].X = cellv->Uvs[1].Y = 0;
		cellv->Uvs[2].X = cellv->Uvs[2].Y = 0;
		cellv->Uvs[3].X = cellv->Uvs[3].Y = 0;
		return;
	}

	FVector2D wh;
	wh.X = (float)cellv->Texture->Resource->GetSizeX();
	wh.Y = (float)cellv->Texture->Resource->GetSizeY();

//	FVector2D wh = map->pixelSize;
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
		cellv->Uvs[0].X = cellv->Uvs[1].X = left;
		cellv->Uvs[2].X = cellv->Uvs[3].X = right;
		cellv->Uvs[1].Y = cellv->Uvs[3].Y = top;
		cellv->Uvs[0].Y = cellv->Uvs[2].Y = bottom;
	}
	else
	{
		// そのまま。頂点の順番は下記の通り
		// 01
		// 23
		cellv->Uvs[0].X = cellv->Uvs[2].X = left;
		cellv->Uvs[1].X = cellv->Uvs[3].X = right;
		cellv->Uvs[0].Y = cellv->Uvs[1].Y = top;
		cellv->Uvs[2].Y = cellv->Uvs[3].Y = bottom;
	}
}

