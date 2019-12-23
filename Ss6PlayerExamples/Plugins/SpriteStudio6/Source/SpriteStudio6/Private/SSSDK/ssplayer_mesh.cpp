#include "ssplayer_mesh.h"

#include <stdio.h>
#include <cstdlib>

//#include "../Loader/ssloader.h"

#include "ssplayer_animedecode.h"
#include "ssplayer_macro.h"
#include "ssplayer_matrix.h"


void	SsMeshPart::makeMesh()
{
	//パーツステートの初期化の際にターゲットセルが作られる、その際にマップもコピーする？
	if (targetCell == 0)return;
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
	if (offset_world_vertices) delete[] offset_world_vertices;

	draw_vertices = new float[3 * psize];
	offset_world_vertices = new float[3 * psize];

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

	float txsizew = targetTexture ? targetTexture->GetSurfaceWidth()  : 1.f;
	float txsizeh = targetTexture ? targetTexture->GetSurfaceHeight() : 1.f;

	float uvpixel_x = 1.0f / txsizew;
	float uvpixel_y = 1.0f / txsizeh;


	for (int i = 0; i < targetCell->MeshPointList.Num(); i++)
	{
		FVector2D& v = targetCell->MeshPointList[i];
		vertices[i * 3 + 0] = v.X + offs.X;
		vertices[i * 3 + 1] = -v.Y + offs.Y;
		vertices[i * 3 + 2] = 0;
		offset_world_vertices[i * 3 + 0] = 0;
		offset_world_vertices[i * 3 + 1] = 0;
		offset_world_vertices[i * 3 + 2] = 0;

		colors[i * 4 + 0] = 1.0f;
		colors[i * 4 + 1] = 1.0f;
		colors[i * 4 + 2] = 1.0f;
		colors[i * 4 + 3] = 1.0f;
		uvs[i * 2 + 0] = (targetCell->Pos.X + v.X) * uvpixel_x;
		uvs[i * 2 + 1] = (targetCell->Pos.Y + v.Y) * uvpixel_y;

		draw_vertices[i * 3 + 0] = vertices[i * 3];
		draw_vertices[i * 3 + 1] = vertices[i * 3 + 1];
		draw_vertices[i * 3 + 2] = vertices[i * 3 + 2];
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
	for (int i = 0; i < targetCell->MeshTriList.Num(); i++)
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

	if (offset_world_vertices) delete[] offset_world_vertices;
	offset_world_vertices = 0;

	myPartState = 0;
}


void    SsMeshPart::updateTransformMesh()
{
	if(nullptr == bindBoneInfo)
	{

		return;
	}

//	float matrix[16];
	for (int i = 0; i < ver_size; i++)
	{
		StBoneWeight& info = bindBoneInfo[i];

		FVector out(FVector::ZeroVector);
		FVector outtotal(FVector::ZeroVector);

		SsPartState* matrixState = myPartState;

		//デフォームアトリビュートを使用している
		if (myPartState->is_defrom == true)
		{
			// キーからデフォームアトリビュートを取り出して
			FVector offset(0, 0, 0);

			int size1 = myPartState->deformValue.verticeChgList.Num();
			int size2 = (int)getVertexNum();
			if (size1 == size2)
			{
				// キーからデフォームアトリビュートを取り出して
				offset = getOffsetWorldVerticesFromKey(i);
			}
			// 描画用デフォームアトリビュートを更新
			setOffsetWorldVertices(i, offset);
		}


		if (info.bindBoneNum == 0)
		{
			//バインドされていないメッシュの場合

			this->isBind = false;
			//MatrixTransformVector3(matrixState->matrix, info.offset[n], out);

			//デフォームオフセットを加える
			if(myPartState->is_defrom == true )
			{ 
				draw_vertices[i * 3 + 0] = vertices[i * 3 + 0] + getOffsetLocalVertices(i).X;
				draw_vertices[i * 3 + 1] = vertices[i * 3 + 1] + getOffsetLocalVertices(i).Y;
				draw_vertices[i * 3 + 2] = 0;
			}
		}
		else
		{
			this->isBind = true;
			for (int n = 0; n < info.bindBoneNum; n++)
			{
				if (info.bone[n])
				{
					if (info.bindBoneNum > 0) matrixState = info.bone[n];
					float w = info.weight[n] / 100.0f;
					MatrixTransformVector3(matrixState->matrix, info.offset[n], out);
					out.X *= w;
					out.Y *= w;

					outtotal.X += out.X;
					outtotal.Y += out.Y;
					outtotal.Z = 0;
				}
			}
			//デフォームオフセットを加える
			if (myPartState->is_defrom == true)
			{
				//SsOpenGLMatrix mtx;
				FMatrix mtx;

				// ボーンにより影響を受けた座標(ローカル座標)
				FVector   out2;
				//mtx.pushMatrix(myPartState->matrix);
				FMemory::Memcpy(mtx.M, myPartState->matrix, sizeof(float)*16);

				//mtx.inverseMatrix();
				//mtx.TransformVector3(outtotal, out2);
				mtx = mtx.Inverse();
				out2 = mtx.TransformVector(outtotal);

				// デフォームによる影響(ローカル座標)
				FVector   vec;
				vec.X = getOffsetLocalVertices(i).X;
				vec.Y = getOffsetLocalVertices(i).Y;
				vec.Z = 0.0f;

				outtotal = FVector(out2.X + vec.X, out2.Y + vec.Y, out2.Z + vec.Z);

				// ワールド座標に変換
				//mtx.pushMatrix(myPartState->matrix);
				//mtx.TransformVector3(outtotal, out2);
				FMemory::Memcpy(mtx.M, myPartState->matrix, sizeof(float)*16);
				out2 = mtx.TransformVector(outtotal);

				outtotal = out2;
			}

			draw_vertices[i * 3 + 0] = outtotal.X * 1.0f;
			draw_vertices[i * 3 + 1] = outtotal.Y * 1.0f;
			draw_vertices[i * 3 + 2] = 0;
		}

	}
}

// デフォームアトリビュート
// ワールド座標を取得
FVector SsMeshPart::getOffsetWorldVerticesFromKey(int index)
{
	FVector out1, out2;

	{
		//SsOpenGLMatrix mtx;
		FMatrix mtx;
		FVector   vec;
		vec.X = vertices[index * 3 + 0] + myPartState->deformValue.verticeChgList[index].X;
		vec.Y = vertices[index * 3 + 1] + myPartState->deformValue.verticeChgList[index].Y;
		vec.Z = 0.0f;

		//mtx.pushMatrix(myPartState->matrix);
		//mtx.TransformVector3(vec, out1);
		FMemory::Memcpy(mtx.M, myPartState->matrix, sizeof(float)*16);
		out1 = mtx.TransformVector(vec);
	}

	{
		//SsOpenGLMatrix mtx;
		FMatrix mtx;
		FVector   vec;
		vec.X = vertices[index * 3 + 0];
		vec.Y = vertices[index * 3 + 1];
		vec.Z = 0.0f;

		//mtx.pushMatrix(myPartState->matrix);
		//mtx.TransformVector3(vec, out2);
		FMemory::Memcpy(mtx.M, myPartState->matrix, sizeof(float)*16);
		out2 = mtx.TransformVector(vec);
	}


	FVector offset;

	offset.X = out1.X - out2.X;
	offset.Y = out1.Y - out2.Y;
	offset.Z = out1.Z - out2.Z;

	return offset;
}

// デフォームアトリビュート取得
// ワールド座標を設定（バインドがある場合）
void	SsMeshPart::setOffsetWorldVertices(int index, const FVector & v)
{
	offset_world_vertices[index * 3 + 0] = v.X;
	offset_world_vertices[index * 3 + 1] = v.Y;
	offset_world_vertices[index * 3 + 2] = v.Z;
}

// デフォームアトリビュート取得
// ローカル座標系を取得（バインドがない場合）
FVector2D SsMeshPart::getOffsetLocalVertices(int index)
{
	FVector out1, out2;

	{
		//SsOpenGLMatrix mtx;
		FMatrix mtx;
		FVector   vec;
		vec.X = vertices[index * 3 + 0];
		vec.Y = vertices[index * 3 + 1];
		vec.Z = 0.0f;

		//mtx.pushMatrix(myPartState->matrix);
		//mtx.TransformVector3(vec, out1);
		FMemory::Memcpy(mtx.M, myPartState->matrix, sizeof(float)*16);
		out1 = mtx.TransformVector(vec);
	}

	{
		//SsOpenGLMatrix mtx;
		FMatrix mtx;
		FVector   vec;
		vec.X = out1.X + offset_world_vertices[index * 3 + 0];
		vec.Y = out1.Y + offset_world_vertices[index * 3 + 1];
		vec.Z = out1.Z + offset_world_vertices[index * 3 + 2];

		//mtx.pushMatrix(myPartState->matrix);
		//mtx.inverseMatrix();
		//mtx.TransformVector3(vec, out2);
		FMemory::Memcpy(mtx.M, myPartState->matrix, sizeof(float)*16);
		mtx = mtx.Inverse();
		out2 = mtx.TransformVector(vec);
	}

	FVector2D offset;

	offset.X = out2.X - vertices[index * 3 + 0];
	offset.Y = out2.Y - vertices[index * 3 + 1];

	return offset;
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
	animeboneList.Empty();
	jointList.Empty();


	int num = bindAnime->getStateNum();
	SsPartState* indexState = bindAnime->getPartState();
	for (int i = 0; i < num; i++)
	{
		if (indexState[i].partType == SsPartType::Mesh)
		{
			meshList.Add(&indexState[i]);
		}
		if (indexState[i].partType == SsPartType::Armature)
		{
			animeboneList.Add(&indexState[i]);
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
		if (meshPart && meshPart->vertices)
			meshPart->updateTransformMesh();
	}

}


void	SsMeshAnimator::copyToSsMeshPart(FSsMeshBind* src , SsMeshPart* dst , const TMap<int32, SsPartState*>& boneIdxListLocal )
{

	int bnum = (int)boneIdxListLocal.Num();
	bool isbind = false;	//バインドするボーンが存在するか？

	for (size_t i = 0; i < src->MeshVerticesBindArray.Num(); i++)
	{
		FSsMeshBindInfo & bi = src->MeshVerticesBindArray[i];

		if (dst->getVertexNum() > (int) i)
		{
			int cntBone = 0;
			for (int n = 0; n < bi.BindBoneNum; n++)
			{
				dst->bindBoneInfo[i].offset[n] = bi.Offset[n];
				dst->bindBoneInfo[i].weight[n] = bi.Weight[n];

				//
				if (boneIdxListLocal.Contains(bi.BoneIndex[n]))
				{
					dst->bindBoneInfo[i].bone[n] = boneIdxListLocal[bi.BoneIndex[n]];
					isbind = true;	//バインドするボーンがある
					cntBone++;
				}

			}
			dst->bindBoneInfo[i].bindBoneNum = cntBone;

		}


	}


}


void	SsMeshAnimator::modelLoad()
{
	if (bindAnime == 0)return;
	if (meshList.Num() == 0) return;
	if (animeboneList.Num() == 0) return;
	if (jointList.Num() == 0) return;

	FSsModel* model = bindAnime->getMyModel();

	TMap<FName, int32>& boneListRef = model->BoneList;

	TMap<int32, SsPartState*> boneIdxList;
	
	for( size_t i = 0; i < animeboneList.Num(); i++)
	{
		int idx = boneListRef[animeboneList[i]->part->PartName];
		boneIdxList.Add(idx, animeboneList[i]);
	}

	if (meshList.Num() == model->MeshList.Num() )
	{
		for (size_t i = 0; i < model->MeshList.Num(); i++)
		{
			copyToSsMeshPart(&model->MeshList[i], meshList[i]->meshPart, boneIdxList);

		}

	}


}
