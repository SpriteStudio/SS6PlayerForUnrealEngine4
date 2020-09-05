#pragma once

#include "SsTypes.h"
#include "SsPlayerTickResult.h"

class USs6Project;
class SsAnimeDecoder;
class SsCellMapList;
struct SsPartState;
class SsEffectEmitter;
struct particleDrawData;
struct FSsSequence;

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
	bool Play(int32 InAnimPackIndex, int32 InAnimationIndex, int32 InStartFrame=0, float InPlayRate=1.f, int32 InLoopCount=0, bool bInRoundTrip=false);
	// 一時停止 
	void Pause(){ bPlaying = false; }
	// 再開 
	bool Resume();
	// 再生中か取得 
	inline bool IsPlaying() const { return bPlaying; }
	// 再生中のAnimPackインデックスを取得 
	inline int32 GetPlayingAnimPackIndex() const { return PlayingAnimPackIndex; }
	// 再生中のAnimationインデックスを取得 
	inline int32 GetPlayingAnimationIndex() const { return PlayingAnimationIndex; }
	// アニメーション名からインデックスを取得 
	bool GetAnimationIndex(const FName& InAnimPackName, const FName& InAnimationName, int32& OutAnimPackIndex, int32& OutAnimationIndex);

	// シーケンスの再生 
	bool PlaySequence(int32 InSequencePackIndex, int32 InSequenceIndex, int32 InStartFrame=0, float InPlayRate=1.f);
	// シーケンス再生中か取得 
	inline bool IsPlayingSequence() const { return bPlaying & bPlayingSequence; }
	// 再生中のSequencePackインデックスを取得 
	inline int32 GetPlayingSequencePackIndex() const { return PlayingSequencePackIndex; }
	// 再生中のSequenceインデックスを取得 
	inline int32 GetPlayingSequenceIndex() const { return PlayingSequenceIndex; }
	// シーケンス名からインデックスを取得 
	bool GetSequenceIndex(const FName& InSequencePackName, const FName& InSequenceName, int32& OutSequencePackIndex, int32& OutSequneceIndex);
	// シーケンスIDからインデックスを取得 
	bool GetSequenceIndexById(const FName& InSequencePackName, int32 InSequenceId, int32& OutSequencePackIndex, int32& OutSequneceIndex);

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
	bool PlayInternal(int32 InAnimPackIndex, int32 InAnimationIndex, int32 InStartFrame, float InPlayRate, int32 InLoopCount, bool bInRoundTrip);
	void TickAnimation(float DeltaSeconds, FSsPlayerTickResult& Result);
	void FindUserDataInInterval(FSsPlayerTickResult& Result, float Start, float End);
	void CreateRenderParts(SsAnimeDecoder* RenderDecoder, const FVector2D& CanvasSize, const FVector2D& Pivot, bool bInstance=false);
	bool CreateRenderPart(FSsRenderPart& OutRenderPart, const SsPartState* State, const FVector2D& CanvasSize, const FVector2D& Pivot, bool bInstance);
	void CreateEffectRenderParts(TArray<FSsRenderPart>& OutRenderParts, const SsPartState* State, const FVector2D& CanvasSize, const FVector2D& Pivot);
	void CreateEffectRenderPart(TArray<FSsRenderPart>& OutRenderParts, const SsPartState* State, const FVector2D& CanvasSize, const FVector2D& Pivot, SsEffectEmitter* Emitter, float Time, SsEffectEmitter* Parent=nullptr, const particleDrawData* DrawData=nullptr);

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
	SsAnimeDecoder* Decoder;
	SsCellMapList* CellMapList;
	bool bPlaying;
	bool bFirstTick;
	FVector2D AnimPivot;
	int32 PlayingAnimPackIndex;
	int32 PlayingAnimationIndex;
	bool bCalcHideParts;

	bool bPlayingSequence;
	int32 PlayingSequencePackIndex;
	int32 PlayingSequenceIndex;
	float PlayingSequenceFrame;

	TArray<FSsRenderPart> RenderParts;
};
