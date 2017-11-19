#ifndef __SSATRIBUTE__
#define __SSATRIBUTE__

#include "sstypes.h"
#include "ssvalue.h"
#include "ssInterpolation.h"
#include <list>
#include	<map>





//アニメーション中のキーフレームの内容を表現するクラス
class SsKeyframe
{
public:
	int							time;	///< 時間
	SsInterpolationType::_enum	ipType;	///< 補間タイプ
	SsCurve						curve;	///< 曲線補間計算用パラメータ
	SsValue						value;	///< 値
public:
	SsKeyframe() : 
	  ipType(SsInterpolationType::invalid) ,
	  time(0)
	  {}
	virtual ~SsKeyframe(){}

	SSSERIALIZE_BLOCK
	{
		SSAR_DECLARE_ATTRIBUTE( time );
		SSAR_DECLARE_ATTRIBUTE_ENUM( ipType );

		if ( SsNeedsCurveParams(ipType) )
		{
			SSAR_DECLARE( curve );
		}
		
		SsValueSeriarizer( ar , value );

	}
};

typedef std::vector<SsKeyframe*> AttributeKeyList;
typedef std::map<int,SsKeyframe*> AttributeKeyDic;


//タグ毎に存在するキーフレームを格納するクラス
class	SsAttribute	//Tag毎に存在する
{

public:
	//キーフレームデータ : Value（タグによって異なるの組み)
	SsAttributeKind::_enum  tag;
	AttributeKeyList		key;

	AttributeKeyDic		key_dic;

public:
	SsAttribute(){}
	virtual ~SsAttribute(){
		for ( AttributeKeyList::iterator itr = key.begin() ; 
			itr != key.end() ; itr ++ ) delete (*itr);
	}


	SSSERIALIZE_BLOCK
	{
		SSAR_DECLARE_ATTRIBUTE_ENUM( tag );
		SSAR_DECLARE_LISTEX( key ,  "" );

		key_dic.clear();
		for ( AttributeKeyList::iterator itr = key.begin() ; itr != key.end() ; itr++)
		{
			int time = (*itr)->time;
			SsKeyframe* key = (*itr);
			key_dic[time] = key;
		}
	}

	bool	isEmpty()
	{
		return key.empty();
	}

	const SsKeyframe*	firstKey();

	///時間から左側のキーを取得
	const SsKeyframe*	findLeftKey( int time );

	//時間から右側のキーを取得する
	const SsKeyframe*	findRightKey( int time );


};


void	GetSsPartsColorValue( const SsKeyframe* key , SsPartsColorAnime& v );
void	GetSsColorValue(const SsKeyframe* key, SsColorAnime& v);
void	GetSsVertexAnime( const SsKeyframe* key , SsVertexAnime& v );
void	GetSsRefCell( const SsKeyframe* key , SsRefCell& v );
void	GetSsUserDataAnime( const SsKeyframe* key , SsUserDataAnime& v );
void	GetSsInstparamAnime( const SsKeyframe* key , SsInstanceAttr& v );
void	GetSsEffectParamAnime( const SsKeyframe* key , SsEffectAttr& v );


#endif
