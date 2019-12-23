#include "SsEffectFile.h"


void FSsEffectNode::Serialize(FArchive& Ar)
{
	Behavior.Serialize(Ar);
}

void FSsEffectModel::Serialize(FArchive& Ar)
{
	if(Ar.IsLoading())
	{
		BuildTree();
	}
	for(int32 i = 0; i < NodeList.Num(); ++i)
	{
		NodeList[i].Serialize(Ar);
	}
}

void FSsEffectModel::BuildTree()
{
	if(0 < NodeList.Num())
	{
		Root = &(NodeList[0]);
		for(int32 i = 1; i < NodeList.Num(); ++i)
		{
			int32 parentIndex = NodeList[i].ParentIndex;
			if(parentIndex >= 0)
			{
				NodeList[parentIndex].AddChildEnd(&(NodeList[i]));
			}
		}
	}
}

void FSsEffectFile::Serialize(FArchive& Ar)
{
	EffectData.Serialize(Ar);
}

