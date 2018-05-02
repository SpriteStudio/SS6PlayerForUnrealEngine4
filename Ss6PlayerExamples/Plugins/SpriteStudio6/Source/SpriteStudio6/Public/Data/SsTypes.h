#pragma once

#include "SsTypes.generated.h"


//===============================================================
// Declare Type
//===============================================================
/// 矩形
template <typename T>
class SsTRect
{
public:
	T	x, y, w, h;

	SsTRect(): x(0), y(0), w(0), h(0)  {}
	SsTRect(T ax, T ay, T aw, T ah): x(ax), y(ay), w(aw), h(ah) {}
	SsTRect(const SsTRect& r): x(r.x), y(r.y), w(r.w), h(r.h) {}

	bool	operator ==(const SsTRect& r) const {return x == r.x && y == r.y && w == r.w && h == r.h;}
	bool	operator !=(const SsTRect& r) const {return !(*this == r);}
};
typedef SsTRect<int> SsIRect;


/// カラー
template <typename T>
class SsTColor
{
public:
	T	r, g, b, a;

	SsTColor(): r(0), g(0), b(0), a(0) {}
	SsTColor(T ar, T ag, T ab, T aa): r(ar), g(ag), b(ab), a(aa) {}
	SsTColor(const SsTColor& s): r(s.r), g(s.g), b(s.b), a(s.a) {}

	void	fromARGB(uint32 c);
	void	fromBGRA(uint32 c);

	uint32		toARGB() const;

	bool	operator ==(const SsTColor& rhs) const
	{
		return r == rhs.r
			&& g == rhs.g
			&& b == rhs.b
			&& a == rhs.a;
	}

private:
};


/// rgba 小数版
template<> inline SsTColor<float>::SsTColor(): r(0.5f), g(0.5f), b(0.5f), a(1.f) {}
template<> inline void SsTColor<float>::fromARGB(uint32 c)
{
	a = (float)(c >> 24) / 255.f;
	r = (float)((c >> 16) & 0xff) / 255.f;
	g = (float)((c >> 8) & 0xff) / 255.f;
	b = (float)(c & 0xff) / 255.f;
}
template<> inline void SsTColor<float>::fromBGRA(uint32 c)
{
	b = (float)(c >> 24) / 255.f;
	g = (float)((c >> 16) & 0xff) / 255.f;
	r = (float)((c >> 8) & 0xff) / 255.f;
	a = (float)(c & 0xff) / 255.f;
}
template<> inline uint32 SsTColor<float>::toARGB() const
{
	uint32 c = (uint8)(a * 255) << 24 | (uint8)(r * 255) << 16 | (uint8)(g * 255) << 8 | (uint8)(b * 255);
	return c;
}


template<> inline SsTColor<uint32>::SsTColor(): r(255), g(255), b(255), a(255) {}
template<> inline void SsTColor<uint32>::fromARGB(uint32 c)
{
	a = (c >> 24);
	r = ((c >> 16) & 0xff);
	g = ((c >> 8) & 0xff);
	b = (c & 0xff);
}
template<> inline void SsTColor<uint32>::fromBGRA(uint32 c)
{
	b = (c >> 24) ;
	g = ((c >> 16) & 0xff) ;
	r = ((c >> 8) & 0xff) ;
	a = (c & 0xff) ;
}
template<> inline uint32 SsTColor<uint32>::toARGB() const
{
	uint32 c = (uint8)(a) << 24 | (uint8)(r) << 16 | (uint8)(g) << 8 | (uint8)(b);
	return c;
}


template<> inline SsTColor<uint8>::SsTColor(): r(255), g(255), b(255), a(255) {}
template<> inline void SsTColor<uint8>::fromARGB(uint32 c)
{
	a = (c >> 24);
	r = ((c >> 16) & 0xff);
	g = ((c >> 8) & 0xff);
	b = (c & 0xff);
}
template<> inline void SsTColor<uint8>::fromBGRA(uint32 c)
{
	b = (c >> 24) ;
	g = ((c >> 16) & 0xff) ;
	r = ((c >> 8) & 0xff) ;
	a = (c & 0xff) ;
}
template<> inline uint32 SsTColor<uint8>::toARGB() const
{
	uint32 c = (uint8)(a) << 24 | (uint8)(r) << 16 | (uint8)(g) << 8 | (uint8)(b);
	return c;
}

/// floatでのカラー値定義
typedef SsTColor<float> SsFColor;

/// unsigned intでのカラー値定義
typedef SsTColor<uint32> SsColor;

/// uisigned charでのカラー値定義
//typedef SsTColor<uint8> SsU8Color;
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
	out.a = FParse::HexDigit(str[0]) * 16 + FParse::HexDigit(str[1]);
	out.r = FParse::HexDigit(str[2]) * 16 + FParse::HexDigit(str[3]);
	out.g = FParse::HexDigit(str[4]) * 16 + FParse::HexDigit(str[5]);
	out.b = FParse::HexDigit(str[6]) * 16 + FParse::HexDigit(str[7]);
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
namespace SsPartsSortMode
{
	enum _enum
	{
		invalid = -1, 
		prio,			///< 描画順は優先度で制御する。優先度を表示し、Ｚ座標を隠す。
		z,				///< 描画順はＺ座標で制御する。Ｚ座標を表示し、優先度を隠す。
		num
	};
}
FString __EnumToString_(SsPartsSortMode::_enum n);
void __StringToEnum_(FString n , SsPartsSortMode::_enum& out);

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
		Armature,		///< ボーンパーツ
		Effect,			///< エフェクト
		Mesh,			///< メッシュパーツ
		MoveNode,		///< 動作起点
		Constraint,		///<コンストレイント
		Mask,			///< マスク
		Joint,			///< メッシュとボーンの関連付けパーツ
		BonePoint,		///< ボーンポイント
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
		MulAlpha,		///< 4 α乗算
		Screen,			///< 5 スクリーン
		Exclusion,		///< 6 除外
		Invert,			///< 7 反転

		// 以下、Playerでの処理分岐用 
		Effect,			///    エフェクト用カラーブレンドタイプ
		MixVertex,		///    カラーブレンドMixで頂点毎指定の場合
		Mask,			///    マスクパーツ用

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
		Losclx,		///< [LSCX]ローカルスケール.X
		Loscly,		///< [LSCY]ローカルスケール.Y
		Alpha,		///< [ALPH]不透明度
		Loalpha,	///< [LALP]ローカル不透明度
		Prio,		///< [PRIO]優先度
		Fliph,		///< [FLPH]左右反転(セルの原点を軸にする)
		Flipv,		///< [FLPV]上下反転(セルの原点を軸にする)
		Hide,		///< [HIDE]非表示
		PartsColor,	///< [PCOL]パーツカラー
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
		Mask,		///< [MASK]マスク閾値
		User,		///< [USER]ユーザーデータ
		Instance,	///< [IPRM]インスタンスパーツパラメータ
		Effect,		///< [EFCT]エフェクトパラメータ
		Num,

		Invalid = 254	///< 無効値。旧データからの変換時など
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsAttributeKind::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsAttributeKind::Type>& out);


/// カラーブレンドキーのカラー値
struct SsColorBlendValue
{
	SsColor		rgba;	///カラー値
	float		rate;	///反映率

	SsColorBlendValue(): rate(0){}

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


///パーツカラー使用時のブレンドタイプとカラー値
struct SsPartsColorAnime
{
	SsColorBlendTarget::Type	target;		//ブレンドの適用方法  単色(全体) , 頂点単位 
	SsBlendType::Type			blendType;	//ブレンド種別 (mix　乗算　加算　減算）
	SsColorBlendValue			color;		//単色。全体の場合に使用されるカラー値
	SsColorBlendValue			colors[4];	//頂点単位の場合使用されるカラー値

	SsColorBlendValue&			getColors(int index) { return colors[index]; }
	int							getTargetToInt() { return (int)target; }
	int							getBlendTypeToInt() { return (int)blendType; }
	SsPartsColorAnime() :
		target(SsColorBlendTarget::Invalid),
		blendType(SsBlendType::Invalid) {}

};

/// カラーブレンド使用時のブレンドタイプとカラー値
struct SsColorAnime
{
	SsColorBlendTarget::Type	target;		//ブレンドの適用方法  単色(全体) , 頂点単位 
	SsBlendType::Type			blendType;	//ブレンド種別 (mix　乗算　加算　減算）
	SsColorBlendValue			color;		//単色。全体の場合に使用されるカラー値
	SsColorBlendValue			colors[4];	//頂点単位の場合使用されるカラー値

	SsColorBlendValue&			getColors(int index){ return colors[index];}
	int							getTargetToInt(){ return (int)target;}
	int							getBlendTypeToInt(){ return (int)blendType;}
	SsColorAnime()
		: target( SsColorBlendTarget::Invalid )
		, blendType( SsBlendType::Invalid )
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


UENUM()
namespace SsIkRotationArrow
{
	enum Type
	{
		Arrowfree,
		Clockwise,
		Anticlockwise,
		Num,

		Unknown = 254,
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsIkRotationArrow::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsIkRotationArrow::Type>& out);

class SsEffectAttr
{
public:
	int					startTime;				///< 開始フレーム
	float				speed;					///< 再生速度
	bool				independent;			///< 独立動作
	int					loopflag;				///< ループ時の動作フラグをビット対応でまとめたもの
	bool				attrInitialized;
	int					curKeyframe;			///< キーが配置されたフレーム 
	SsEffectAttr() :
		startTime(0),
		speed(1.0f),
		independent(false),
		loopflag(0),
		attrInitialized(false),
		curKeyframe(0)
	{}

	void init()
	{
		startTime = 0;
		speed = 1.0f;
		independent = false;
		loopflag = 0;
		attrInitialized = false;
		curKeyframe = 0;
	}
};

//参照セル値
struct SsRefCell
{
	int		mapid;
	FName	name;
};
class SsUserDataAnime
{
public:
	bool			useInteger;	///<整数が使用されているか
	bool			usePoint;	///<座標データが使用されているか
	bool			useRect;	///<矩形データが使用されているか
	bool			useString;	///<文字列データが使用されているか 

	int				integer;	///< 整数
	FVector2D		point;		///< 座標
	SsIRect			rect;		///< 矩形
	FString			string;		///< 文字列

	SsUserDataAnime() : 
		useInteger(false),
		usePoint(false),
		useRect(false),
		useString(false)
	{}
};

class SsInstanceAttr
{
public:
	bool   		infinity;		///無限ループフラグ
	bool   		reverse;		///逆再生フラグ
	bool   		pingpong;		///往復再生フラグ
	bool		independent;	///独立動作フラグ
	int			loopflag;		///ループフラグをビット対応でまとめたもの
	int			loopNum;		///ループ回数　無限ループフラグ=trueの時には無効
	FName		startLabel;		///再生開始位置 ラベル名称
	int			startOffset;	///再生開始位置 ラベル名称からのオフセット
	FName		endLabel;		///再生終了位置 ラベル名称
	int			endOffset;		///再生終了位置 ラベル名称からのオフセット
	float       speed;			///再生スピード
	int			startFrame;		///ラベル位置とオフセット位置を加えた実際のフレーム数
	int			endFrame;		///ラベル位置とオフセット位置を加えた実際のフレーム数


								//テンポラリ <-エディタ用計算値の可能性もあるので後で整理
	int         curKeyframe; //この値があるキーフレーム値 (計算値）
	float		liveFrame;	//再生時間の累積

	SsInstanceAttr():
		infinity( false ),
		reverse( false ),
		pingpong( false ),
		independent(false),
		loopflag(0),
		loopNum( 1 ),
		startLabel("_start"),
		startOffset(0),
		endLabel("_end"),
		endOffset(0),
		curKeyframe( 0 ),
		speed(1.0f),
		startFrame(0),
		endFrame(0),
		liveFrame(0.0f)
	{}
};

// インスタンスアトリビュートのループフラグ 
enum {
	INSTANCE_LOOP_FLAG_INFINITY    = 1 << 0,
	INSTANCE_LOOP_FLAG_REVERSE     = 1 << 1,
	INSTANCE_LOOP_FLAG_PINGPONG    = 1 << 2,
	INSTANCE_LOOP_FLAG_INDEPENDENT = 1 << 3,
};

// エフェクトアトリビュートのループフラグ 
enum {
	EFFECT_LOOP_FLAG_INFINITY = 1 << 0,
};

//メッシュの分割タイプ
UENUM()
namespace SsMeshDivType
{
	enum Type
	{
		PolylineBase,
		Boxdiv,
		Num,

		Unknown,
	};
}
FString SPRITESTUDIO6_API __EnumToString_(TEnumAsByte<SsMeshDivType::Type> n);
void SPRITESTUDIO6_API __StringToEnum_(FString n , TEnumAsByte<SsMeshDivType::Type>& out);

USTRUCT()
struct FSsTriangle
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, Category=SsTriangle)
	int32	IdxPo1;

	UPROPERTY(VisibleAnywhere, Category=SsTriangle)
	int32	IdxPo2;

	UPROPERTY(VisibleAnywhere, Category=SsTriangle)
	int32	IdxPo3;

};

struct SsBoneBind
{
	int		boneIndex;               //暫定でパーツIDを使用する
	float   blend;
};



//---------------------------------------------------------------
// 描画用の頂点情報
struct FSsRenderVertex
{
	FVector2D Position;	// 0.0f～1.0f (アニメーションのCanvasSizeに対しての座標. 実際の値は範囲外もあり得る) 
	FVector2D TexCoord;
	FColor Color;
	float ColorBlendRate;
};

// 描画用のメッシュ頂点 
struct FSsRenderMeshVertex
{
	FVector2D Position;
	FVector2D TexCoord;
};

// 描画用のメッシュ情報 
struct FSsRenderMesh
{
	TArray<FSsRenderMeshVertex> Vertices;
	TArray<uint32> Indices;
	FColor Color;
	float ColorBlendRate;
};

// 描画用のパーツ情報
struct FSsRenderPart
{
	int32 PartIndex;
	FSsRenderVertex Vertices[4];
	TArray<FSsRenderMesh> Mesh;		// (0 < Mesh.Num()) の場合はメッシュパーツとして扱い、Verticesは参照しない 
	UTexture* Texture;
	SsBlendType::Type AlphaBlendType;
	SsBlendType::Type ColorBlendType;
	bool bMaskInfluence;

	void CopyFrom(const FSsRenderPart* Other)
	{
		PartIndex = Other->PartIndex;
		Vertices[0] = Other->Vertices[0];
		Vertices[1] = Other->Vertices[1];
		Vertices[2] = Other->Vertices[2];
		Vertices[3] = Other->Vertices[3];
		Mesh = Other->Mesh;
		Texture = Other->Texture;
		AlphaBlendType = Other->AlphaBlendType;
		ColorBlendType = Other->ColorBlendType;
	}
};

// SlateBrush付き描画用のパーツ情報
struct FSsRenderPartWithSlateBrush : public FSsRenderPart
{
	TSharedPtr<struct FSlateMaterialBrush> Brush;
};

