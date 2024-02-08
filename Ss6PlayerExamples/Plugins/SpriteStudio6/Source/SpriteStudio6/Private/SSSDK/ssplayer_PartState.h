﻿#ifndef __SSPLAYER_PARTSTATE__
#define __SSPLAYER_PARTSTATE__

//#include "../Loader/ssloader.h"
//#include "../Helper/ssHelper.h"

#include "ssplayer_cellmap.h"


class SsAnimeDecoder;
//class SsEffectRenderer;
class SsEffectRenderV2;
class SsMeshPart;



///パーツの状態を保持するクラスです。
struct SsPartState
{
	int				index;				///パーツのインデックスと一対一になるID

	float			vertices[3 * 5];	///< 座標
//	float			colors[4 * 4];		///< カラー (４頂点分）
	float			uvs[2 * 5];			///< UV		(４隅＋中央)
	float			matrix[4 * 4];		///< 行列
	float			matrixLocal[4 * 4];	///< ローカル行列


	SsPartState*	parent;			/// 親へのポインタ
	float* 			inheritRates;	///< 継承設定の参照先。inheritType がparentだと親のを見に行く。透過的に遡るのでルートから先に設定されている必要がある。

	FVector3f	position;		///< 位置。あくまで親パーツ基準のローカル座標
	FVector3f	rotation;		///< 回転角。degree
	FVector2f		scale;			///< スケール	
	FVector2f		localscale;		///< ローカルスケール	
	float			alpha;			///< 不透明度 0~1
	float			localalpha;		///< ローカル不透明度 0~1
	int				prio;			///< 優先度
//	bool			hFlip;			///< 水平反転　※Ver6では非対応
//	bool			vFlip;			///< 垂直反転　※Ver6では非対応
	bool			hide;			///< 非表示にする
	FVector2f		pivotOffset;	///< 原点のオフセット。旧SSの原点は左上基準にしたこの値に相当する。0,0が中央+0.5,+0.5が右上になる。参照セルがある場合はセルの原点に＋する＝オフセット扱いになる。
//	FVector2f		anchor;			///< アンカーポイント。親パーツのどの位置に引っ付けるか？0,0が中央+0.5,+0.5が右上になる。　※Ver6では非対応
	FVector2f		size;			///< 表示サイズ	
	bool			imageFlipH;		///　セル画像を水平反転するか
 	bool			imageFlipV;		///	 セル画像を垂直反転するか
	FVector2f		uvTranslate;	///< UV 平行移動
	float			uvRotation;		///< UV 回転
	FVector2f		uvScale;		///< UV スケール
	float			boundingRadius;	///< 当たり判定用の円の半径

	SsCellValue		cellValue;		///< セルアニメの値
	SsPartsColorAnime partsColorValue;///< カラーアニメの値
//	SsColorAnime	colorValue;		///< カラーアニメの値
	FSsVertexAnime	vertexValue;	///< 頂点アニメの値
	SsEffectAttr	effectValue;	///< エフェクトの値
	int				effectTime;
	float			effectTimeTotal;
//	int				effectseed;

	bool			noCells;				/// セル参照が見つからない
	bool			is_parts_color;			/// パーツカラーが使用される 
//	bool			is_color_blend;			/// カラーブレンドが使用される (描画コストが高いシェーダが使われるためフラグ化)　※Ver6では非対応
	bool			is_vertex_transform;	/// 頂点変形が使用される (描画コストが高いシェーダが使われるためフラグ化)
	bool			is_localAlpha;			/// ローカル不透明度を使用している
	bool			is_defrom;				/// デフォームアトリビュートを使用している

	SsInstanceAttr	instanceValue;

	SsBlendType::Type	alphaBlendType;
		
	SsAnimeDecoder*		refAnime;
	SsCellMapList*		refCellMapList;
	SsEffectRenderV2*	refEffect;

//	//V4互換計算用
//	FVector3f		_temp_position;
//	FVector3f		_temp_rotation;
//	FVector2f		_temp_scale;

	SsPartType::Type partType;

	int				masklimen;
	bool			maskInfluence;

	SsMeshPart*		meshPart;

	SsDeformAttr	deformValue;

	FSsPart*		part;

	SsPartState();

	virtual ~SsPartState();
	void	destroy();
	void	init();
	bool	inherits_(SsAttributeKind::Type kind) const {return inheritRates[(int)kind] != 0.f;}
	void	reset();

	// UE4 TArray::Sort() 用 
	bool operator < (const SsPartState& Other) const
	{
		return (prio != Other.prio) ? (prio < Other.prio) : (index < Other.index);
	}
};


#endif
