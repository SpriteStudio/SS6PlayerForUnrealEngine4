#include "SpriteStudio6PrivatePCH.h"

#include <stdio.h>
#include <cstdlib>

//#include "../Loader/ssloader.h"

#include "ssplayer_animedecode.h"
#include "ssplayer_mesh.h"
#include "ssplayer_macro.h"
#include "ssplayer_matrix.h"


void	SsMeshPart::makeMesh()
{
	//パーツステートの初期化の際にターゲットセルが作られる、その際にマップもコピーする？

	size_t psize = targetCell->MeshPointList.Num();

	if (vertices) delete[] vertices;
	if (colors) delete[] colors;
	if (uvs) delete[] uvs;

	if (indices) delete[] indices;
	if (draw_vertices) delete[] draw_vertices;
	if (update_vertices_outer) delete[]update_vertices_outer;
	if (vertices_outer) delete[]vertices_outer;
	if (bindBoneInfo) delete[]bindBoneInfo;
	if (weightColors) delete[]weightColors;

	draw_vertices = new float[3 * psize];

	vertices_outer = new FVector2D[3 * psize];// //ツール用
	update_vertices_outer = new FVector2D[3 * psize];// //ツール用


	vertices = new float[3 * psize];
	colors = new float[4 * psize];
	uvs = new float[2 * psize];
	weightColors = new float[4 * psize];


	bindBoneInfo = new StBoneWeight[psize];
	memset(bindBoneInfo, 0, sizeof(StBoneWeight) * psize);


	FVector2D offs; //中央
	offs.X = (-targetCell->Size.X / 2.0f);
	offs.Y = (targetCell->Size.Y / 2.0f);

	offs.X -= targetCell->Pivot.X * targetCell->Size.X;
	offs.Y -= targetCell->Pivot.Y * targetCell->Size.Y;

	ver_size = targetCell->MeshPointList.Num();

	float txsizew = this->targetTexture->GetSurfaceWidth();
	float txsizeh = this->targetTexture->GetSurfaceHeight();

	float uvpixel_x = 1.0f / txsizew;
	float uvpixel_y = 1.0f / txsizeh;


	for (size_t i = 0; i < targetCell->MeshPointList.Num(); i++)
	{
		FVector2D& v = targetCell->MeshPointList[i];
		vertices[i * 3 + 0] = v.X + offs.X;
		vertices[i * 3 + 1] = -v.Y + offs.Y;
		vertices[i * 3 + 2] = 0;


		colors[i * 4 + 0] = 1.0f;
		colors[i * 4 + 1] = 1.0f;
		colors[i * 4 + 2] = 1.0f;
		colors[i * 4 + 3] = 1.0f;
		uvs[i * 2 + 0] = (targetCell->Pos.X + v.X) * uvpixel_x;
		uvs[i * 2 + 1] = (targetCell->Pos.Y + v.Y) * uvpixel_y;
	}

	outter_vertexnum = targetCell->OuterPoint.Num();
	for (size_t i = 0; i < outter_vertexnum; i++)
	{
		FVector2D& v = targetCell->OuterPoint[i];

		vertices_outer[i].X = v.X + offs.X;
		vertices_outer[i].Y = -v.Y + offs.Y;
	}


	tri_size = targetCell->MeshTriList.Num();

	indices = new unsigned short[tri_size * 3];
	for (size_t i = 0; i < targetCell->MeshTriList.Num(); i++)
	{
		FSsTriangle& t = targetCell->MeshTriList[i];
		indices[i * 3 + 0] = t.IdxPo1;
		indices[i * 3 + 1] = t.IdxPo2;
		indices[i * 3 + 2] = t.IdxPo3;
	}
}


void	SsMeshPart::Cleanup()
{
	if (vertices) delete[] vertices;
	vertices = 0;

	if (colors) delete[] colors;
	colors = 0;

	if (weightColors)  delete[] weightColors;
	weightColors = 0;

	if (uvs) delete[] uvs;
	uvs = 0;

	if (indices) delete[] indices;
	indices_num = 0;

	if (draw_vertices) delete[] draw_vertices;
	draw_vertices = 0;

	if (vertices_outer) delete[] vertices_outer;
	vertices_outer = 0;

	if (update_vertices_outer) delete[] update_vertices_outer;
	update_vertices_outer = 0;

	if (bindBoneInfo) delete[] bindBoneInfo;
	bindBoneInfo = 0;

	myPartState = 0;
}


void    SsMeshPart::updateTransformMesh()
{
//	float matrix[16];

	for (int i = 0; i < ver_size; i++)
	{
		StBoneWeight& info = bindBoneInfo[i];

		FVector out;
		FVector outtotal;

		draw_vertices[i * 3 + 0] = outtotal.X;
		draw_vertices[i * 3 + 1] = outtotal.Y;
		draw_vertices[i * 3 + 2] = 0;

		if (info.bindBoneNum > 0 )
		{
			for (int n = 0; n < info.bindBoneNum; n++)
			{
				//outtotal.X = info.offset[n].X;
				//outtotal.Y = info.offset[n].Y;
				//outtotal.z = info.offset[n].z;

#if 1
				if (info.bone[n])
				{
					float w = info.weight[n] / 100.0f;
					MatrixTransformVector3(info.bone[n]->matrix, info.offset[n], out);
					out.X *= w;
					out.Y *= w;
					//out.z *= w;

					outtotal.X += out.X;
					outtotal.Y += out.Y;
					outtotal.Z = 0;
					//outtotal.z += out.z;
				}
#endif

			}

			draw_vertices[i * 3 + 0] = outtotal.X * 1.0f;
			draw_vertices[i * 3 + 1] = outtotal.Y * 1.0f;
			draw_vertices[i * 3 + 2] = 0;
		}
	}
}

//-----------------------------------------------------------

SsMeshAnimator::SsMeshAnimator() : bindAnime (0)
{

}

void	SsMeshAnimator::setAnimeDecoder(SsAnimeDecoder* s)
{
	bindAnime = s;
}


void	SsMeshAnimator::makeMeshBoneList()
{
	if (bindAnime == 0)return;
	meshList.Empty();
	boneList.Empty();
	jointList.Empty();


	size_t num = bindAnime->getStateNum();
	SsPartState* indexState = bindAnime->getPartState();
	for (int i = 0; i < num; i++)
	{
		if (indexState[i].partType == SsPartType::Mesh)
		{
			meshList.Add(&indexState[i]);
		}
		if (indexState[i].partType == SsPartType::Armature)
		{
			boneList.Add(&indexState[i]);
		}
		if (indexState[i].partType == SsPartType::Joint)
		{
			jointList.Add(&indexState[i]);
		}
	}

	modelLoad();


}

void	SsMeshAnimator::update()
{
	if (bindAnime == 0)return;

	for(auto it = meshList.CreateConstIterator(); it; ++it)
	{
		SsPartState* state = (*it);

		SsMeshPart* meshPart = (*it)->meshPart;
		if (meshPart)
			meshPart->updateTransformMesh();
	}

}

void	SsMeshAnimator::modelLoad()
{
	if (bindAnime == 0)return;

	FSsModel* model = bindAnime->getMyModel();


	for (size_t i = 0; i < model->MeshList.Num(); i++)
	{
		TArray<FSsMeshBindInfo>& mvb = model->MeshList[i].MeshVerticesBindArray;

		{
			SsPartState* target = this->meshList[i];
			SsMeshPart*		meshPart = target->meshPart;
			FSsPart* pt = &(model->PartList[target->index]);	//fordebug
			size_t psize = meshPart->targetCell->MeshPointList.Num();
			//bindBoneInfo は　psiz分だけ生成されているので、mvb.size()が超えたら間違いがあると思われる
			if (meshPart->ver_size < (int)mvb.Num())
			{
				UE_LOG(LogSpriteStudio, Warning, TEXT("ver_sizeを超えている : %s ver_size:%d mvb.size:%d \n"), *(pt->PartName.ToString()), meshPart->ver_size, (int)mvb.Num());
			}
		}

		for (size_t n = 0; n < mvb.Num(); n++)
		{
			int bonenum = mvb[n].BindBoneNum;
			SsPartState* target = this->meshList[i];
			SsMeshPart*		meshPart = target->meshPart;

			if (meshPart->ver_size <= (int)n)
			{
				continue;	//テスト
			}

			for (int l = 0; l < bonenum; l++)
			{
				meshPart->bindBoneInfo[n].weight[l] = mvb[n].Weight[l];
				meshPart->bindBoneInfo[n].offset[l] = mvb[n].Offset[l];
				int bi = mvb[n].BoneIndex[l];
				meshPart->bindBoneInfo[n].bone[l] = this->boneList[bi];
			}
			meshPart->bindBoneInfo[n].bindBoneNum = bonenum;

		}

	}


}
