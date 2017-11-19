#include "SpriteStudio6PrivatePCH.h"

#include <stdio.h>
#include <cstdlib>

#include "../Loader/ssloader.h"

#include "ssplayer_animedecode.h"
#include "ssplayer_mesh.h"
#include "ssplayer_macro.h"
#include "ssplayer_matrix.h"


void	SsMeshPart::makeMesh()
{
	//パーツステートの初期化の際にターゲットセルが作られる、その際にマップもコピーする？

	size_t psize = targetCell->meshPointList.size();

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

	vertices_outer = new SsVector2[3 * psize];// //ツール用
	update_vertices_outer = new SsVector2[3 * psize];// //ツール用


	vertices = new float[3 * psize];
	colors = new float[4 * psize];
	uvs = new float[2 * psize];
	weightColors = new float[4 * psize];


	bindBoneInfo = new StBoneWeight[psize];
	memset(bindBoneInfo, 0, sizeof(StBoneWeight) * psize);


	SsVector2 offs; //中央
	offs.x = (-targetCell->size.x / 2.0f);
	offs.y = (targetCell->size.y / 2.0f);

	offs.x -= targetCell->pivot.x * targetCell->size.x;
	offs.y -= targetCell->pivot.y * targetCell->size.y;

	ver_size = targetCell->meshPointList.size();

	float txsizew = this->targetTexture->getWidth();
	float txsizeh = this->targetTexture->getHeight();

	float uvpixel_x = 1.0f / txsizew;
	float uvpixel_y = 1.0f / txsizeh;


	for (size_t i = 0; i < targetCell->meshPointList.size(); i++)
	{
		SsVector2& v = targetCell->meshPointList[i];
		vertices[i * 3 + 0] = v.x + offs.x;
		vertices[i * 3 + 1] = -v.y + offs.y;
		vertices[i * 3 + 2] = 0;


		colors[i * 4 + 0] = 1.0f;
		colors[i * 4 + 1] = 1.0f;
		colors[i * 4 + 2] = 1.0f;
		colors[i * 4 + 3] = 1.0f;
		uvs[i * 2 + 0] = (targetCell->pos.x + v.x) * uvpixel_x;
		uvs[i * 2 + 1] = (targetCell->pos.y + v.y) * uvpixel_y;
	}

	outter_vertexnum = targetCell->outerPoint.size();
	for (size_t i = 0; i < outter_vertexnum; i++)
	{
		SsVector2& v = targetCell->outerPoint[i];

		vertices_outer[i].x = v.x + offs.x;
		vertices_outer[i].y = -v.y + offs.y;
	}


	tri_size = targetCell->meshTriList.size();

	indices = new unsigned short[tri_size * 3];
	for (size_t i = 0; i < targetCell->meshTriList.size(); i++)
	{
		SsTriangle& t = targetCell->meshTriList[i];
		indices[i * 3 + 0] = t.idxPo1;
		indices[i * 3 + 1] = t.idxPo2;
		indices[i * 3 + 2] = t.idxPo3;
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
	float matrix[16];

	for (int i = 0; i < ver_size; i++)
	{
		StBoneWeight& info = bindBoneInfo[i];

		SsVector3 out;
		SsVector3 outtotal;

		draw_vertices[i * 3 + 0] = outtotal.x;
		draw_vertices[i * 3 + 1] = outtotal.y;
		draw_vertices[i * 3 + 2] = 0;

		if (info.bindBoneNum > 0 )
		{
			for (int n = 0; n < info.bindBoneNum; n++)
			{
				//outtotal.x = info.offset[n].x;
				//outtotal.y = info.offset[n].y;
				//outtotal.z = info.offset[n].z;

#if 1
				if (info.bone[n])
				{
					float w = info.weight[n] / 100.0f;
					MatrixTransformVector3(info.bone[n]->matrix, info.offset[n], out);
					out.x *= w;
					out.y *= w;
					//out.z *= w;

					outtotal.x += out.x;
					outtotal.y += out.y;
					outtotal.z = 0;
					//outtotal.z += out.z;
				}
#endif

			}

			draw_vertices[i * 3 + 0] = outtotal.x * 1.0f;
			draw_vertices[i * 3 + 1] = outtotal.y * 1.0f;
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
	meshList.clear();
	boneList.clear();
	jointList.clear();


	size_t num = bindAnime->getStateNum();
	SsPartState* indexState = bindAnime->getPartState();
	for (int i = 0; i < num; i++)
	{
		if (indexState[i].partType == SsPartType::mesh)
		{
			meshList.push_back(&indexState[i]);
		}
		if (indexState[i].partType == SsPartType::armature)
		{
			boneList.push_back(&indexState[i]);
		}
		if (indexState[i].partType == SsPartType::joint)
		{
			jointList.push_back(&indexState[i]);
		}
	}

	modelLoad();


}

void	SsMeshAnimator::update()
{
	if (bindAnime == 0)return;

	foreach(std::vector<SsPartState*>, meshList, it)
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

	SsModel* model = bindAnime->getMyModel();


	for (size_t i = 0; i < model->meshList.size(); i++)
	{
		std::vector<SsMeshBindInfo>& mvb = model->meshList[i]->meshVerticesBindArray;

		{
			SsPartState* target = this->meshList[i];
			SsMeshPart*		meshPart = target->meshPart;
			SsPart* pt = model->partList[target->index];	//fordebug
			size_t psize = meshPart->targetCell->meshPointList.size();
			//bindBoneInfo は　psiz分だけ生成されているので、mvb.size()が超えたら間違いがあると思われる
			if (meshPart->ver_size < (int)mvb.size())
			{
				DEBUG_PRINTF("ver_sizeを超えている : %s ver_size:%d mvb.size:%d \n", pt->name.c_str(), meshPart->ver_size, (int)mvb.size());
			}
		}

		for (size_t n = 0; n < mvb.size(); n++)
		{
			int bonenum = mvb[n].bindBoneNum;
			SsPartState* target = this->meshList[i];
			SsMeshPart*		meshPart = target->meshPart;

			if (meshPart->ver_size <= (int)n)
			{
				continue;	//テスト
			}

			for (int l = 0; l < bonenum; l++)
			{
				meshPart->bindBoneInfo[n].weight[l] = mvb[n].weight[l];
				meshPart->bindBoneInfo[n].offset[l] = mvb[n].offset[l];
				int bi = mvb[n].boneIndex[l];
				meshPart->bindBoneInfo[n].bone[l] = this->boneList[bi];
			}
			meshPart->bindBoneInfo[n].bindBoneNum = bonenum;

		}

	}


}
