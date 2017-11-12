#include "SpriteStudio6EdPrivatePCH.h"
#include "SsLoader.h"

#include "SsProject.h"
#include "SsAnimePack.h"
#include "SsCellMap.h"
#include "SsArchiver.h"





USsProject* FSsLoader::LoadSsProject(UObject* InParent, FName InName, EObjectFlags Flags, const uint8*& Buffer, size_t Size)
{
	XMLDocument xml;
	if( XML_SUCCESS != xml.Parse((const char*)Buffer, Size) )
	{
		return NULL;
	}

	SsXmlIArchiver ar(&xml, "SpriteStudioProject");

	USsProject* Proj = NewObject<USsProject>(InParent, InName, Flags);
	SerializeSsProject(*Proj, &ar);

	return Proj;
}

bool FSsLoader::LoadSsAnimePack(FSsAnimePack* AnimePack, const uint8*& Buffer, size_t Size)
{
	XMLDocument xml;
	if( XML_SUCCESS != xml.Parse((const char*)Buffer, Size) )
	{
		return false;
	}

	SsXmlIArchiver ar(&xml, "SpriteStudioAnimePack");
	SerializeSsAnimePack(*AnimePack, &ar);

	return true;
}

bool FSsLoader::LoadSsCellMap(FSsCellMap* CellMap, const uint8*& Buffer, size_t Size)
{
	XMLDocument xml;
	if( XML_SUCCESS != xml.Parse((const char*)Buffer, Size) )
	{
		return false;
	}

	SsXmlIArchiver ar(&xml, "SpriteStudioCellMap");
	SerializeSsCellMap(*CellMap, &ar);

	return true;
}

bool FSsLoader::LoadSsEffectFile(struct FSsEffectFile* EffectFile, const uint8*& Buffer, size_t Size)
{
	XMLDocument xml;
	if( XML_SUCCESS != xml.Parse((const char*)Buffer, Size) )
	{
		return false;
	}

	SsXmlIArchiver ar(&xml, "SpriteStudioEffect");
	SerializeSsEffectFile(*EffectFile, &ar);

	return true;
}
