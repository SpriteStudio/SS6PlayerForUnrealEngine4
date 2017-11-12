#pragma once

#include "SsPlayerCellmap.h"

class FSsAnimeDecoder;
class FSsEffectRenderV2;


///パーツの状態を保持するクラスです。
struct FSsPartState
{
	int				Index;				///パーツのインデックスと一対一になるID

	float			Vertices[3 * 5];	///< 座標
	float			Colors[4 * 4];		///< カラー (４頂点分）
	float			Uvs[2 * 5];			///< UV		(４隅＋中央)
	float			Matrix[4 * 4];		///< 行列


	FSsPartState*	Parent;			/// 親へのポインタ
	float* 			InheritRates;	///< 継承設定の参照先。inheritType がparentだと親のを見に行く。透過的に遡るのでルートから先に設定されている必要がある。

	FVector			Position;		///< 位置。あくまで親パーツ基準のローカル座標
	FVector			Rotation;		///< 回転角。degree
	FVector2D		Scale;			///< スケール	
	float			Alpha;			///< 不透明度 0~1
	int				Prio;			///< 優先度
	bool			HFlip;			///< 水平反転
	bool			VFlip;			///< 垂直反転
	bool			Hide;			///< 非表示にする
	FVector2D		PivotOffset;	///< 原点のオフセット。旧SSの原点は左上基準にしたこの値に相当する。0,0が中央+0.5,+0.5が右上になる。参照セルがある場合はセルの原点に＋する＝オフセット扱いになる。
	FVector2D		Anchor;			///< アンカーポイント。親パーツのどの位置に引っ付けるか？0,0が中央+0.5,+0.5が右上になる。
	FVector2D		Size;			///< 表示サイズ	
	bool			ImageFlipH;		///　セル画像を水平反転するか
 	bool			ImageFlipV;		///	 セル画像を垂直反転するか
	FVector2D		UvTranslate;	///< UV 平行移動
	float			UvRotation;		///< UV 回転
	FVector2D		UvScale;		///< UV スケール
	float			BoundingRadius;	///< 当たり判定用の円の半径

	FSsCellValue	CellValue;		///< セルアニメの値
	FSsColorAnime	ColorValue;		///< カラーアニメの値
	FSsVertexAnime	VertexValue;	///< 頂点アニメの値
	FSsEffectAttr	EffectValue;	///< エフェクトの値
	int32			EffectTime;
	float			EffectTimeTotal;
	int32			EffectSpeed;

	bool			NoCells;				/// セル参照が見つからない
	bool			IsColorBlend;			/// カラーブレンドが使用される (描画コストが高いシェーダが使われるためフラグ化)
	bool			IsVertexTransform;	/// 頂点変形が使用される (描画コストが高いシェーダが使われるためフラグ化)

	FSsInstanceAttr	InstanceValue;


	SsBlendType::Type	AlphaBlendType;
		
	FSsAnimeDecoder*	RefAnime;
	FSsEffectRenderV2*	RefEffect;

	FSsPartState();

	virtual ~FSsPartState();
	void	Destroy();
	void	Init();
	bool	Inherits_(SsAttributeKind::Type kind) const {return InheritRates[(int)kind] != 0.f;}
	void	Reset();


	// ソート用. ssplayer_animedecode.h SsPartStateLessより 
	bool operator < (const FSsPartState& Other) const
	{
		if (Prio == Other.Prio)
			return Index < Other.Index;
		return Prio < Other.Prio;
	}
};


