#pragma once

#include "SsTypes.generated.h"


//===============================================================
// Declare Type
//===============================================================
/// 矩形
template <typename T>
class TSsRect
{
public:
	T X, Y, W, H;

	TSsRect(): X(0), Y(0), W(0), H(0)  {}
	TSsRect(T ax, T ay, T aw, T ah): X(ax), Y(ay), W(aw), H(ah) {}
	TSsRect(const TSsRect& r): X(r.X), Y(r.Y), W(r.W), H(r.H) {}

	bool operator ==(const TSsRect& r) const {return X == r.x && Y == r.y && W == r.w && H == r.h;}
	bool operator !=(const TSsRect& r) const {return !(*this == r);}
};
typedef TSsRect<int> SsIRect;


/// カラー
template <typename T>
class TSsColor
{
public:
	T R, G, B, A;

	TSsColor(): R(0), G(0), B(0), A(0) {}
	TSsColor(T ar, T ag, T ab, T aa): R(ar), G(ag), B(ab), A(aa) {}
	TSsColor(const TSsColor& s): R(s.R), G(s.G), B(s.B), A(s.A) {}

	void FromARGB(uint32 c);
	void FromBGRA(uint32 c);

	uint32 ToARGB() const;

	bool operator ==(const TSsColor& rhs) const
	{
		return R == rhs.r
			&& G == rhs.g
			&& B == rhs.b
			&& A == rhs.a;
	}
};


/// rgba 小数版
template<> inline TSsColor<float>::TSsColor(): R(0.5f), G(0.5f), B(0.5f), A(1.f) {}
template<> inline void TSsColor<float>::FromARGB(uint32 c)
{
	A = (float)(c >> 24) / 255.f;
	R = (float)((c >> 16) & 0xff) / 255.f;
	G = (float)((c >> 8) & 0xff) / 255.f;
	B = (float)(c & 0xff) / 255.f;
}
template<> inline void TSsColor<float>::FromBGRA(uint32 c)
{
	B = (float)(c >> 24) / 255.f;
	G = (float)((c >> 16) & 0xff) / 255.f;
	R = (float)((c >> 8) & 0xff) / 255.f;
	A = (float)(c & 0xff) / 255.f;
}
template<> inline uint32 TSsColor<float>::ToARGB() const
{
	uint32 c = (uint8)(A * 255) << 24 | (uint8)(R * 255) << 16 | (uint8)(G * 255) << 8 | (uint8)(B * 255);
	return c;
}


template<> inline TSsColor<uint32>::TSsColor(): R(255), G(255), B(255), A(255) {}
template<> inline void TSsColor<uint32>::FromARGB(uint32 c)
{
	A = (c >> 24);
	R = ((c >> 16) & 0xff);
	G = ((c >> 8) & 0xff);
	B = (c & 0xff);
}
template<> inline void TSsColor<uint32>::FromBGRA(uint32 c)
{
	B = (c >> 24) ;
	G = ((c >> 16) & 0xff) ;
	R = ((c >> 8) & 0xff) ;
	A = (c & 0xff) ;
}
template<> inline uint32 TSsColor<uint32>::ToARGB() const
{
	uint32 c = (uint8)(A) << 24 | (uint8)(R) << 16 | (uint8)(G) << 8 | (uint8)(B);
	return c;
}


template<> inline TSsColor<uint8>::TSsColor(): R(255), G(255), B(255), A(255) {}
template<> inline void TSsColor<uint8>::FromARGB(uint32 c)
{
	A = (c >> 24);
	R = ((c >> 16) & 0xff);
	G = ((c >> 8) & 0xff);
	B = (c & 0xff);
}
template<> inline void TSsColor<uint8>::FromBGRA(uint32 c)
{
	B = (c >> 24) ;
	G = ((c >> 16) & 0xff) ;
	R = ((c >> 8) & 0xff) ;
	A = (c & 0xff) ;
}
template<> inline uint32 TSsColor<uint8>::ToARGB() const
{
	uint32 c = (uint8)(A) << 24 | (uint8)(R) << 16 | (uint8)(G) << 8 | (uint8)(B);
	return c;
}

/// floatでのカラー値定義
typedef TSsColor<float> SsFColor;

/// unsigned intでのカラー値定義
typedef TSsColor<uint32> SsColor;

/// uisigned charでのカラー値定義
//typedef TSsColor<uint8> SsU8Color;
USTRUCT()
struct FSsU8Color
{
	GENERATED_USTRUCT_BODY()

	FSsU8Color()
		: R(0), G(0), B(0), A(0)
	{}
	FSsU8Color(uint8 rr, uint8 gg, uint8 bb, uint8 aa)
		: R(rr), G(gg), B(bb), A(aa)
	{}

	inline uint32 ToARGB() const
	{
		return A << 24 | R << 16 | G << 8 | B;
	}

	UPROPERTY(EditAnyWhere, Category="SsU8Color")
	uint8 R;

	UPROPERTY(EditAnyWhere, Category="SsU8Color")
	uint8 G;

	UPROPERTY(EditAnyWhere, Category="SsU8Color")
	uint8 B;

	UPROPERTY(EditAnyWhere, Category="SsU8Color")
	uint8 A;
};


struct FToLower
{
	char operator()(char c) { return tolower(c); }
};

/// 与えられた文字列をカラー値に変換するための関数
inline void ConvertStringToSsColor(const FString& str , SsColor& out)
{
	out.A = FParse::HexDigit(str[0]) * 16 + FParse::HexDigit(str[1]);
	out.R = FParse::HexDigit(str[2]) * 16 + FParse::HexDigit(str[3]);
	out.G = FParse::HexDigit(str[4]) * 16 + FParse::HexDigit(str[5]);
	out.B = FParse::HexDigit(str[6]) * 16 + FParse::HexDigit(str[7]);
}


/// 曲線補間計算用パラメータ
USTRUCT()
struct FSsCurve
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category=SsCurve)
	float StartTime;		///< 始点キーの時間から制御点の時間へのオフセット値。Ｘ軸に当たる。

	UPROPERTY(VisibleAnywhere, Category=SsCurve)
	float StartValue;		///< 始点キーの値から	〃	。Ｙ軸	〃

	UPROPERTY(VisibleAnywhere, Category=SsCurve)
	float EndTime;		///< 終点キーの時間から制御点の時間へのオフセット値。Ｘ軸に当たる。

	UPROPERTY(VisibleAnywhere, Category=SsCurve)
	float EndValue;		///< 終点キーの値から	〃	。Ｙ軸	〃


	UPROPERTY(VisibleAnywhere, Category=SsCurve)
	float StartKeyTime;	///< [ワークパラメータ] 始点キーの時間 計算時のみ使用

	UPROPERTY(VisibleAnywhere, Category=SsCurve)
	float EndKeyTime;		///< [ワークパラメータ] 終点キーの時間 計算時のみ使用


	UPROPERTY(VisibleAnywhere, Category=SsCurve)
	bool SyncStartEnd;		///< [編集用パラメータ]カーブエディタでの編集時に始点・終点ハンドルを同期して動かすか？

	FSsCurve() : StartTime(0.f), StartValue(0.f), EndTime(0.f), EndValue(0.f), StartKeyTime(0.f), EndKeyTime(0.f){}

};



//---------------------------------------------------------------
/// ソートモード
UENUM()
namespace SsPartsSortMode
{
	enum Type
	{
		Prio,			///< 描画順は優先度で制御する。優先度を表示し、Ｚ座標を隠す。
		Z,				///< 描画順はＺ座標で制御する。Ｚ座標を表示し、優先度を隠す。
		Num,

		Invalid = 254,
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsPartsSortMode::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsPartsSortMode::Type>& out);

//---------------------------------------------------------------
/// Animation Part Type
UENUM()
namespace SsPartType
{
	enum Type
	{
		Null,			///< null。領域を持たずSRT情報のみ。ただし円形の当たり判定は設定可能。
		Normal,			///< 通常パーツ。領域を持つ。画像は無くてもいい。
		Text,			///< テキスト(予約　未実装）
		Instance,		///< インスタンス。他アニメ、パーツへの参照。シーン編集モードの代替になるもの
		Effect,			///< エフェクト
		Num,

		Invalid = 254
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsPartType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsPartType::Type>& out);


//---------------------------------------------------------------
/// 当たり判定形状
UENUM()
namespace SsBoundsType
{
	enum Type 
	{
		None,			///< 当たり判定として使わない。
		Quad,			///< 自在に変形する四辺形。頂点変形など適用後の４角を結んだ領域。最も重い。
		Aabb,			///< 回転しない全体を囲む矩形で交差判定
		Circle,			///< 真円の半径で距離により判定する
		CircleSmin,		///< 真円の半径で距離により判定する (スケールはx,yの最小値をとる）
		CircleSmax,		///< 真円の半径で距離により判定する (スケールはx,yの最大値をとる）
		Num,

		Invalid = 254
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsBoundsType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsBoundsType::Type>& out);


//---------------------------------------------------------------
/// 継承タイプ
UENUM()
namespace SsInheritType
{
	enum Type
	{
		Parent,			///< 親の継承方法をそのまま引き継ぐ
		Self,			///< 自身がアトリビュート別に持つ継承方法を使う
		Num,

		Invalid = 254
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsInheritType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsInheritType::Type>& out);

//---------------------------------------------------------------
/// ブレンドタイプ
UENUM()
namespace SsBlendType
{
	enum Type
	{
		Mix,			///< 0 ブレンド（ミックス）
		Mul,			///< 1 乗算
		Add,			///< 2 加算
		Sub,			///< 3 減算
		Effect,			///    エフェクト用カラーブレンドタイプ
		Num,

		Invalid = 254
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsBlendType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsBlendType::Type>& out);

///カラーブレンドキーが使用されている際のカラー適用範囲の定義
UENUM()
namespace SsColorBlendTarget
{
	enum Type
	{
		Whole,	///< 単色。全体にかける。
		Vertex,	///< 頂点単位
		Num,

		Invalid = 254
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsColorBlendTarget::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsColorBlendTarget::Type>& out);

///補間モードの定義
UENUM()
namespace SsInterpolationType
{
	enum Type 
	{
		None,			///< なし
		Linear,			///< 線形
		Hermite,		///< エルミート
		Bezier,			///< ベジェ
		Acceleration,	///< 加速度
		Deceleration,	///< 減速度
		Num,

		Invalid = 254
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsInterpolationType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsInterpolationType::Type>& out);

/// テクスチャラップモード
UENUM()
namespace SsTexWrapMode
{
	enum Type
	{
		Clamp,			/// クランプする
		Repeat,			/// リピート
		Mirror,			/// ミラー
		Num,

		Invalid = 254
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsTexWrapMode::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsTexWrapMode::Type>& out);

/// テクスチャフィルターモード 画素補間方法
UENUM()
namespace SsTexFilterMode
{
	enum Type
	{
		Nearest,	///< ニアレストネイバー
		Linear,		///< リニア、バイリニア
		Num,

		Invalid = 254
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsTexFilterMode::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsTexFilterMode::Type>& out);

/// アトリビュートの種類
UENUM()
namespace SsAttributeKind
{
	enum Type
	{
		Cell=0,		///< [CELL]参照セル
		Posx,		///< [POSX]位置.X
		Posy,		///< [POSY]位置.Y
		Posz,		///< [POSZ]位置.Z
		Rotx,		///< [ROTX]回転.X
		Roty,		///< [ROTY]回転.Y
		Rotz,		///< [ROTZ]回転.Z
		Sclx,		///< [SCLX]スケール.X
		Scly,		///< [SCLY]スケール.Y
		Alpha,		///< [ALPH]不透明度
		Prio,		///< [PRIO]優先度
		Fliph,		///< [FLPH]左右反転(セルの原点を軸にする)
		Flipv,		///< [FLPV]上下反転(セルの原点を軸にする)
		Hide,		///< [HIDE]非表示
		Color,		///< [VCOL]カラーブレンド
		Vertex,		///< [VERT]頂点変形
		Pivotx,		///< [PVTX]原点オフセット.X
		Pivoty,		///< [PVTY]原点オフセット.Y
		Anchorx,	///< [ANCX]アンカーポイント.X
		Anchory,	///< [ANCY]アンカーポイント.Y
		Sizex,		///< [SIZX]表示サイズ.X
		Sizey,		///< [SIZY]表示サイズ.Y
		Imgfliph,	///< [IFLH]イメージ左右反転(常にイメージの中央を原点とする)
		Imgflipv,	///< [IFLV]イメージ上下反転(常にイメージの中央を原点とする)
		Uvtx,		///< [UVTX]UVアニメ.移動.X
		Uvty,		///< [UVTY]UVアニメ.移動.Y
		Uvrz,		///< [UVRZ]UVアニメ.回転
		Uvsx,		///< [UVSX]UVアニメ.スケール.X
		Uvsy,		///< [UVSY]UVアニメ.スケール.Y
		Boundr,		///< [BNDR]当たり判定用の半径
		User,		///< [USER]ユーザーデータ
		Instance,	///< [IPRM]インスタンスパーツパラメータ
		Effect,		///< [EFCT]エフェクトパラメータ
		Num,

		Invalid = 254	///< 無効値。旧データからの変換時など
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsAttributeKind::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsAttributeKind::Type>& out);

UENUM()
namespace SsKeyValueType
{
	enum Type
	{
		Bool = 0,
		Float,
		Int,
		String,
		Cellmap,
		VertexAnime,
		ColorAnime,
		UserData,
		Instance,

		Unkown = 254
	};
}

/// カラーブレンドキーのカラー値
struct FSsColorBlendValue
{
	SsColor	Rgba;	///カラー値
	float	Rate;	///反映率

	FSsColorBlendValue(): Rate(0){}

};

/// カラーブレンド使用時のブレンドタイプとカラー値
struct FSsColorAnime
{
	SsColorBlendTarget::Type	Target;		//ブレンドの適用方法  単色(全体) , 頂点単位 
	SsBlendType::Type			BlendType;	//ブレンド種別 (mix　乗算　加算　減算）
	FSsColorBlendValue			Color;		//単色。全体の場合に使用されるカラー値
	FSsColorBlendValue			Colors[4];	//頂点単位の場合使用されるカラー値

	FSsColorBlendValue&		GetColors(int index){ return Colors[index];}
	int							GetTargetToInt(){ return (int)Target;}
	int							GetBlendTypeToInt(){ return (int)BlendType;}
	FSsColorAnime()
		: Target( SsColorBlendTarget::Invalid )
		, BlendType( SsBlendType::Invalid )
		{}

};


// エフェクトのノードタイプ
UENUM()
namespace SsEffectNodeType
{
	enum Type
	{
		Root = 0,
		Emmiter,
		Particle,
		Num,

		Invalid = 254,
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsEffectNodeType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsEffectNodeType::Type>& out);


//エフェクトのブレンドタイプ
UENUM()
namespace SsRenderBlendType
{
	enum Type
	{
		Mix,
		Add,
		Num,

		Invalid = 254,
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsRenderBlendType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsRenderBlendType::Type>& out);

TEnumAsByte<SsBlendType::Type> SsRenderBlendTypeToBlendType(TEnumAsByte<SsRenderBlendType::Type> n);


class FSsEffectAttr
{
public:
	int32	StartTime;		///<開始フレーム
	float	Speed;			///<再生速度
	bool	Independent;	///<独立動作
	int32	LoopFlag;		///<ループ時の動作フラグをビット対応でまとめたもの
	bool	AttrInitialized;
	int32	CurKeyframe;	///<キーが配置されたフレーム

	FSsEffectAttr()
		: StartTime(0)
		, Speed(1.f)
		, Independent(false)
		, LoopFlag(0)
		, AttrInitialized(false)
		, CurKeyframe(0)
	{}

	void Init()
	{
		StartTime = 0;
		Speed = 1.f;
		Independent = false;
		LoopFlag = 0;
		AttrInitialized = false;
		CurKeyframe = 0;
	}
};


///頂点変形キーの４頂点変形値
struct FSsVertexAnime
{
	FVector2D	Offsets[4];	///< 各頂点の移動オフセット値
	FVector2D&	GetOffsets(int index){ return Offsets[index];}

	FSsVertexAnime()
	{
		for(int i = 0; i < 4; ++i)
		{
			Offsets[i] = FVector2D(0.f, 0.f);
		}
	}
};

//参照セル値
struct FSsRefCell
{
	int		Mapid;
	FName	Name;
};
class FSsUserDataAnime
{
public:
	bool			UseInteger;	///<整数が使用されているか
	bool			UsePoint;	///<座標データが使用されているか
	bool			UseRect;	///<矩形データが使用されているか
	bool			UseString;	///<文字列データが使用されているか 

	int				Integer;	///< 整数
	FVector2D		Point;		///< 座標
	SsIRect			Rect;		///< 矩形
	FString			String;		///< 文字列

	FSsUserDataAnime() : 
		UseInteger(false),
		UsePoint(false),
		UseRect(false),
		UseString(false){}
};

class FSsInstanceAttr
{
public:
	bool	Infinity;		///無限ループフラグ
	bool	Reverse;		///逆再生フラグ
	bool	Pingpong;		///往復再生フラグ
	bool	Independent;	///独立動作フラグ
	int		LoopFlag;		///ループフラグをビット対応でまとめたもの
	int		LoopNum;		///ループ回数　無限ループフラグ=trueの時には無効
	FName	StartLabel;		///再生開始位置 ラベル名称
	int		StartOffset;	///再生開始位置 ラベル名称からのオフセット
	FName	EndLabel;		///再生終了位置 ラベル名称
	int		EndOffset;		///再生終了位置 ラベル名称からのオフセット
	float	Speed;			///再生スピード
	int		StartFrame;		///ラベル位置とオフセット位置を加えた実際のフレーム数
	int		EndFrame;		///ラベル位置とオフセット位置を加えた実際のフレーム数


	//テンポラリ <-エディタ用計算値の可能性もあるので後で整理
	int			CurKeyframe; //この値があるキーフレーム値 (計算値）
	float		LiveFrame;	//再生時間の累積

	FSsInstanceAttr():
		Infinity( false ),
		Reverse( false ),
		Pingpong( false ),
		Independent( false ),
		LoopFlag(0),
		LoopNum( 1 ),
		StartLabel("_start"),
		StartOffset(0),
		EndLabel("_end"),
		EndOffset(0),
		Speed(1.0f),
		StartFrame(0),
		EndFrame(0),
		CurKeyframe( 0 ),
		LiveFrame(0.f)
	{}
};

// インスタンスアトリビュートのループフラグ 
namespace SsInstanceLoopFlag
{
	enum
	{
		INSTANCE_LOOP_FLAG_INFINITY    = 1 << 0,
		INSTANCE_LOOP_FLAG_REVERSE     = 1 << 1,
		INSTANCE_LOOP_FLAG_PINGPONG    = 1 << 2,
		INSTANCE_LOOP_FLAG_INDEPENDENT = 1 << 3,
	};
}

// エフェクトアトリビュートのループフラグ 
namespace SsEffectLoopFlag
{
	enum
	{
		EFFECT_LOOP_FLAG_INFINITY = 1 << 0,
	};
}

//---------------------------------------------------------------
// 描画用の頂点情報
struct FSsRenderVertex
{
	FVector2D Position;	// 0.0f～1.0f (アニメーションのCanvasSizeに対しての座標. 実際の値は範囲外もあり得る) 
	FVector2D TexCoord;
	FColor Color;
	float ColorBlendRate;
};

// 描画用のパーツ情報
struct FSsRenderPart
{
	int32 PartIndex;
	FSsRenderVertex Vertices[4];
	UTexture* Texture;
	SsBlendType::Type AlphaBlendType;
	SsBlendType::Type ColorBlendType;
};

// マテリアル付き描画用のパーツ情報
struct FSsRenderPartWithMaterial : public FSsRenderPart
{
	UMaterialInterface* Material;
};

// SlateBrush付き描画用のパーツ情報
struct FSsRenderPartWithSlateBrush : public FSsRenderPart
{
	TSharedPtr<struct FSlateMaterialBrush> Brush;
};

