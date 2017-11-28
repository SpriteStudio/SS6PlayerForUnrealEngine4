#ifndef __SSPLAYER_ANIMEDECODE__
#define __SSPLAYER_ANIMEDECODE__

//#include "../Loader/ssloader.h"
//#include "../Helper/ssHelper.h"


//#include "ssplayer_types.h"
#include "ssplayer_cellmap.h"
#include "ssplayer_PartState.h"
#include "ssplayer_macro.h"

#include "Ss6Project.h"


class SsAnimeDecoder;
class SsCelMapLinker;
class SsMeshAnimator;


//パーツとアニメを関連付ける
typedef TPair<FSsPart*, FSsPartAnime*> SsPartAndAnime;


//パーツのソート順
class SsPartStateLess
{
public:
	bool operator()(const SsPartState* lhs, const SsPartState* rhs) const
	{
		if (lhs->prio == rhs->prio)
			return lhs->index < rhs->index;
		return lhs->prio < rhs->prio;
	}
};



class SsAnimeDecoder
{
public:

//private:

	///パーツ情報とパーツアニメーションを結びつけアレイにしたもの
	TArray<SsPartAndAnime>		partAnime;
	TArray<SsPartAndAnime>		setupPartAnime;		///セットアップデータ

	///パーツ名からアニメ情報をとるために使うもし、そういった用途が無い場合はローカル変数でも機能する
	TMap<FName,FSsPartAnime*> partAnimeDic;
	TMap<FName, FSsPartAnime*>setupPartAnimeDic;	///セットアップデータ

	SsCellMapList*					curCellMapManager;///アニメに関連付けられているセルマップ

	SsPartState*					partState;			///パーツの現在の状態が格納されています。
	TArray<SsPartState*>			sortList;			///ソート状態
	TArray<SsPartState*>			partStatesMask_;	///マスクテンポラリ
	TArray<SsPartState*>		maskIndexList;


	int				seedOffset;							//エフェクトのシードへ影響
	float			nowPlatTime;
	float			nowPlatTimeOld;						//前のフレームで再生した時間
	float			frameDelta;
	int				curAnimeStartFrame;
	int				curAnimeEndFrame;
	int				curAnimeTotalFrame;
	int				curAnimeFPS;
	FSsAnimation*	curAnimation;
	bool			instancePartsHide;
	bool			maskFuncFlag;						//マスク機能（初期化、描画）を有効にするか？インスタンスパーツ内のマスク対応
	bool			maskParentSetting;					//親のマスク対象

	size_t			stateNum;

	SsMeshAnimator*	meshAnimator;
	FSsModel*		myModel;

private:
	void	updateState( int nowTime , FSsPart* part , FSsPartAnime* part_anime , SsPartState* state );
	void	updateInstance( int nowTime , FSsPart* part , FSsPartAnime* part_anime , SsPartState* state );
	void	updateEffect( float frameDelta , int nowTime , FSsPart* part , FSsPartAnime* part_anime , SsPartState* state );


	void	updateMatrix(FSsPart* part , FSsPartAnime* anime , SsPartState* state);

	void	updateVertices(FSsPart* part , FSsPartAnime* anime , SsPartState* state);

    int		CalcAnimeLabel2Frame(const FName& str, int offset, FSsAnimation* Animation  );
	int		findAnimetionLabel(const FName& str, FSsAnimation* Animation);
	bool	getFirstCell(FSsPart* part, SsCellValue& out);


public:
	SsAnimeDecoder();
	virtual ~SsAnimeDecoder()
	{
//		if ( curCellMapManager )
//			delete curCellMapManager;

		if ( partState )
			delete [] partState;
	}

	virtual void	update( float frameDelta = 1.0f );
//	virtual void	draw();

	void	setAnimation( FSsModel*	model , FSsAnimation* anime , SsCellMapList* cellmap , USs6Project* sspj=0 );
//	void	setAnimation(SsModel*	model, SsAnimation* anime, SsAnimePack *animepack , SsCellMapList* cellmap, SsProject* sspj = 0);

	void	setPlayFrame( float time ) { nowPlatTime = time; }
	int		getAnimeStartFrame() { return curAnimeStartFrame; }
	int		getAnimeEndFrame() { return curAnimeEndFrame; }
	int		getAnimeTotalFrame() { return curAnimeTotalFrame; }
	int		getAnimeFPS() {
		return curAnimeFPS; }		

	size_t	getStateNum() { return stateNum; }
	SsPartState*  getPartState() { return partState; }
	FSsModel*	getMyModel(){return myModel;}

	TArray<SsPartState*>&		getPartSortList(){return sortList;}
	TArray<SsPartAndAnime>&	getPartAnime(){ return	partAnime; }
	
	template<typename mytype> int	SsGetKeyValue( int time , FSsAttribute* attr , mytype&  value );
	template<typename mytype> void	SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , mytype& v );
	void	SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , bool& v );
	void	SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsCellValue& v );
	void	SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsPartsColorAnime& v );
	void	SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsColorAnime& v );
	void	SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , FSsVertexAnime& v );
	void	SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsInstanceAttr& v );
	void	SsInterpolationValue( int time , const FSsKeyframe* leftkey , const FSsKeyframe* rightkey , SsEffectAttr& v );


	void	setInstancePartsHide(bool hide){
		instancePartsHide = hide;
	}
	void	restart();
	void	reset();

	void	setSeedOffset(int a ){ seedOffset = a; }
	int		getSeedOffset(){ return seedOffset; }

	void setMaskFuncFlag(bool flg);									//マスク用ステンシルバッファの初期化を行うか
	void setMaskParentSetting(bool flg);							//親のマスク対象を設定する
	bool getMaskParentSetting(void) { return maskParentSetting; };	//設定された親のマスク対象を取得する

};



#endif
