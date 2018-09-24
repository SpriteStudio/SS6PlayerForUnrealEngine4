#pragma once
#ifndef __SSPLAYER_MESH__
#define __SSPLAYER_MESH__


class ISSTexture;

#define SSMESHPART_BONEMAX	(128)
#define SSMESHPART_CHECKRANGE (4)

struct StBoneWeight
{
	int		   		weight[SSMESHPART_BONEMAX];
	float		   	weight_f[SSMESHPART_BONEMAX];

	SsPartState*    bone[SSMESHPART_BONEMAX];
	FVector			offset[SSMESHPART_BONEMAX];

	int				bindBoneNum;

	float			length[SSMESHPART_BONEMAX];    //temp
	float			lengthtotal;  				   //temp

	int	getBoneNum() { return bindBoneNum; }

};

class SsMeshPart
{
public:


	float			*vertices;			//[3 * 10];///< 座標
	float			*colors;			//[4 * 10];	///< カラー
	float			*weightColors;		//[4 * 10];	///< ウェイト色分けカラー
	float			*uvs;				//[2 * 10];		///< UV
	unsigned short	*indices;
	int				indices_num;
	int				tri_size;
	int				ver_size;
	float			*draw_vertices;		//[3 * 10];///< 座標

	float			*offset_world_vertices;	// 描画に使われるデフォームアトリビュート

	//ツール用テンポラリワーク [editer]
	FVector2D*					vertices_outer;
	FVector2D*					update_vertices_outer;
	size_t						outter_vertexnum;

public:
	StBoneWeight*   	bindBoneInfo;
	FSsCell*  			targetCell;
	UTexture*			targetTexture;

	SsPartState*	   	myPartState;

	//テンポラリ [editor]
	bool				isBind;


public:
	SsMeshPart() :
		vertices(0), colors(0), weightColors(0), uvs(0), indices(0),
		draw_vertices(0), offset_world_vertices(0), vertices_outer(0), update_vertices_outer(0),
		bindBoneInfo(0), targetCell(0), myPartState(0), isBind(false)
	{
	}


	SsMeshPart(SsPartState* s) :
		vertices(0), colors(0), weightColors(0), uvs(0), indices(0),
		draw_vertices(0), offset_world_vertices(0), vertices_outer(0), update_vertices_outer(0),
		bindBoneInfo(0), targetCell(0), isBind(false)
	{
		myPartState = s;
	}

	~SsMeshPart()
	{
		Cleanup();
	}

	void	Cleanup();
	void	makeMesh();

	int		getVertexNum() { return ver_size; }
	StBoneWeight*	getVerticesWeightInfo(int index) {
		if (index > getVertexNum())return 0;
		return &bindBoneInfo[index];
	}

	void    updateTransformMesh();            //再生時用　（バインドされたボーンに沿って変形を行う）

	//デフォーム関連
	FVector getOffsetWorldVerticesFromKey(int index);
	void	setOffsetWorldVertices(int index, const FVector & v);
	FVector2D getOffsetLocalVertices(int index);


/*
	void	renderVertex();
	void	renderMesh(float alpha, bool renderTexture);
	void	renderBoneWeightColor(float alpha, bool renderTexture);
	void	update_matrix(float * matrix);  //バインド前（セットアップモード用のマトリクスアップデート)

	void		calcVerticesPos(SsAnimeState* state);
	SsVector3   getWorldVertexPoint(int index) { return calc_world_vertices[index]; }
*/
//	Editer用
//	void    verticesWeightColorCalc();
//	void	draw_world_vertices();
//	void	draw_world_vertices_once(int index, SsFColor c);
//	int 	isTouchVertex(float mx, float my);
//	bool  	isInPoint(float x, float y);
//	void    bindBoneSmoth(std::vector<SsPartState*>& list);

};



class SsPart;
class SsMeshPart;
//class SsAnimeState;

class   SsMeshAnimator
{
private:
	void	modelLoad();

public:
	SsAnimeDecoder* bindAnime;

	TArray<SsPartState*>    	meshList;
	TArray<SsPartState*>    	boneList;
	TArray<SsPartState*>    	jointList;

public:
	SsMeshAnimator();
	virtual ~SsMeshAnimator() {}

	void	setAnimeDecoder(SsAnimeDecoder* s);

	void	update();
	void	makeMeshBoneList();
	void	copyToSsMeshPart(FSsMeshBind* src, SsMeshPart* dst, TArray<SsPartState*>& boneList);

};




#endif
