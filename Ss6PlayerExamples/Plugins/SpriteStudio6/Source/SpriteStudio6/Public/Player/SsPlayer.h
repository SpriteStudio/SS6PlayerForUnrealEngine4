#pragma once

#include "SsTypes.h"
#include "SsPlayerTickResult.h"

class USs6Project;

//
// SpriteStudioデータの再生制御 
//
class SPRITESTUDIO6_API FSsPlayer
{
public:
	FSsPlayer();
	virtual ~FSsPlayer();

	void SetSsProject(TWeakObjectPtr<USs6Project> InSsProject);
	const TWeakObjectPtr<USs6Project> GetSsProject() const { return SsProject; }

	FSsPlayerTickResult Tick(float DeltaSeconds);
	const TArray<FSsRenderPart>& GetRenderParts() const { return RenderParts; }

	// 再生 
	bool Play(int32 InAnimPackIndex, int32 InAnimationIndex, int32 StartFrame=0, float PlayRate=1.f, int32 LoopCount=0, bool bRoundTrip=false);
	// 一時停止 
	void Pause(){ bPlaying = false; }
	// 再開 
	bool Resume();
	// 再生中か取得 
	bool IsPlaying() const { return bPlaying; }
	// 再生中のAnimPackインデックスを取得 
	inline int32 GetPlayingAnimPackIndex() const { return PlayingAnimPackIndex; }
	// 再生中のAnimationインデックスを取得 
	inline int32 GetPlayingAnimationIndex() const { return PlayingAnimationIndex; }
	// アニメーション名からインデックスを取得 
	bool GetAnimationIndex(const FName& InAnimPackName, const FName& InAnimationName, int32& OutAnimPackIndex, int32& OutAnimationIndex);

	// 指定フレーム送り 
	void SetPlayFrame(float Frame);
	// 現在フレーム取得 
	float GetPlayFrame() const;
	// 最終フレーム取得 
	float GetAnimeEndFrame() const;

	// パーツ名からインデックスを取得 
	int32 GetPartIndexFromName(FName PartName) const;

	// パーツのTransformを取得 
	bool GetPartTransform(int32 PartIndex, FVector2D& OutPosition, float& OutRotate, FVector2D& OutScale) const;

	// パーツのColorLabelを取得 
	FName GetPartColorLabel(int32 PartIndex);

	// 非表示パーツの計算を行うかを設定 
	void SetCalcHideParts(bool bInCalcHideParts);

private:
	void TickAnimation(float DeltaSeconds, FSsPlayerTickResult& Result);
	void FindUserDataInInterval(FSsPlayerTickResult& Result, float Start, float End);

public:
	float PlayRate;		// 再生速度 
	int32 LoopCount;	// ループ回数. 0以下の場合は無限ループ. 往復再生の場合は片道で１回とみなす. 
	bool bRoundTrip;	// 往復再生 
	bool bFlipH;		// 左右反転 
	bool bFlipV;		// 上下反転 
	TMap<int32, TWeakObjectPtr<UTexture>> TextureReplacements;	// パーツ毎のテクスチャ差し替え 

	const FVector2D& GetAnimPivot() const { return AnimPivot; }	// アニメーションに設定されたPivotを取得 (0,0 が中央。-0.5, +0.5 が左上) 
	const FVector2D GetAnimCanvasSize() const;

private:
	TWeakObjectPtr<USs6Project> SsProject;
	class SsAnimeDecoder* Decoder;
	TSharedPtr<class SsCellMapList> CellMapList;
	bool bPlaying;
	bool bFirstTick;
	FVector2D AnimPivot;
	int32 PlayingAnimPackIndex;
	int32 PlayingAnimationIndex;
	bool bCalcHideParts;

	TArray<FSsRenderPart> RenderParts;
};
