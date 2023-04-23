#pragma once

#include "UMG.h"

#include "SsTypes.h"
#include "SsPlayerTickResult.h"
#include "SsPlayer.h"
#include "SsPlayPropertySync.h"

#include "SsPlayerWidget.generated.h"

class UMaterialInterface;
class USs6Project;
class SSsPlayerWidget;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSsWidgetEndPlaySignature2, FName, AnimPackName, FName, AnimationName, int32, AnimPackIndex, int32, AnimationIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSsWidgetUserDataSignature2, FName, PartName, int32, PartIndex, int32, KeyFrame, FSsUserDataValue, UserData);


// SsPlayerWidgetの描画モード 
UENUM()
namespace ESsPlayerWidgetRenderMode
{
	enum Type
	{
		// UMGデフォルトの描画モードです 
		// アルファブレンドモードはミックス/加算のみ反映されます 
		UMG_Default,

		// 描画方法はUMG_Defaultと同等ですが、Maskedマテリアルを使用するため、半透明は２値化されます 
		// アルファブレンドモードはミックスのみ反映されます 
		UMG_Masked,

		// 一旦オフスクリーンレンダリングしたテクスチャを描画します。 
		// UMG_Defaultに比べて処理が重くなりますが、BaseMaterilを変更することで、特殊な効果を実装することが可能です。 
		// 子Widgetのアタッチがサポートされません 
		UMG_OffScreen,
	};
}

//
// SpriteStudio6 Player Widget 
// sspjデータを再生/UMG上で描画する 
//
UCLASS(ClassGroup=SpriteStudio, meta=(DisplayName="Ss Player Widget"))
class SPRITESTUDIO6_API USsPlayerWidget : public UPanelWidget, public FSsPlayPropertySync 
{
	GENERATED_UCLASS_BODY()

public:
	// UObject interface
	virtual void BeginDestroy() override;
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	// UWidget interface
	virtual void SynchronizeProperties() override;
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override { return FText::FromString(TEXT("Sprite Studio")); }
#endif

public:
	void OnSlateTick(float DeltaTime);


protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;

	// UPanelWidget interface
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* InSlot) override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;

private:
	FSsPlayer Player;

	TSharedPtr<SSsPlayerWidget> PlayerWidget;
	TMap<UMaterialInterface*, TSharedPtr<struct FSlateMaterialBrush>> BrushMap;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic* OffScreenMID;

	UPROPERTY(Transient)
	UTexture* OffScreenRenderTarget;


	TMap<UMaterialInterface*, TMap<UTexture*, UMaterialInstanceDynamic*>> PartsMIDMaps;

	UPROPERTY(Transient)
	TMap<int32, UMaterialInterface*> MaterialReplacementMap;

	UPROPERTY(Transient)
	TMap<int32, UMaterialInterface*> MaterialReplacementMapPerBlendMode;


#if WITH_EDITOR
private:
	// １フレームに複数回Tick呼び出しされてしまう問題の対処用 
	float BackWorldTime;
#endif



public:
	// 公開PROPERTY 

	//
	// SpriteStudioAsset
	//

	// 再生するSsProject 
	UPROPERTY(Category="SpriteStudioAsset", EditAnywhere, BlueprintReadOnly, meta=(DisplayThumbnail="true"))
	USs6Project* SsProject;


	//
	// SpriteStudioPlayerSettings
	//

	// 自動更新. Offの場合はUpdatePlayer()を呼び出してアニメーションを更新します. 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	bool bAutoUpdate;

	// 自動再生 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	bool bAutoPlay;

	// 自動再生時のAnimPack名 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly)
	FName AutoPlayAnimPackName;

	// 自動再生時のAnimation名 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly)
	FName AutoPlayAnimationName;

	// 自動再生時のAnimPackIndex 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly)
	int32 AutoPlayAnimPackIndex;

	// 自動再生時のAnimationIndex 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly)
	int32 AutoPlayAnimationIndex;

	// 自動再生時の開始フレーム 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	int32 AutoPlayStartFrame;

	// 自動再生時の再生速度(負の値で逆再生) 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	float AutoPlayRate;

	// 自動再生時のループ回数(0で無限ループ) 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	int32 AutoPlayLoopCount;

	// 自動再生時の往復再生 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	bool bAutoPlayRoundTrip;

	// ウィジェットが非表示の時はアニメーションを更新しない 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	bool bDontUpdateIfHidden;

	// Pause中でもTickする 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	bool bTickableWhenPaused;


	//
	// SpriteStudioRenderSettings 
	//

	// 描画モード 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<ESsPlayerWidgetRenderMode::Type> RenderMode;

	// 描画モードがOffScreenの場合のベースマテリアル 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly, meta=(DisplayName="OffScreen Material", DisplayThumbnail="true"), AdvancedDisplay)
	UMaterialInterface* BaseMaterial;

	// オフスクリーンレンダリングの際の解像度 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FVector2D OffScreenRenderResolution;

	// オフスクリーンレンダリングの際のクリアカラー 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	FColor OffScreenClearColor;

	// 親Widgetのアルファ値を反映するか 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly)
	bool bReflectParentAlpha;


	//
	// SpriteStudioCallback 
	//

	// 再生終了イベント
	UPROPERTY(BlueprintAssignable, Category="SpriteStudioCallback")
	FSsWidgetEndPlaySignature2 OnSsEndPlay;

	// ユーザーデータイベント
	UPROPERTY(BlueprintAssignable, Category="SpriteStudioCallback")
	FSsWidgetUserDataSignature2 OnSsUserData;


	//
	// Misc 
	//

	// 実際に描画に使用されているMaterialInstanceDynamic 
	UPROPERTY(Transient, BlueprintReadOnly, Category="SpriteStudio")
	TArray<UMaterialInstanceDynamic*> RenderMIDs;


public:
	// BP公開関数 

	// アニメーションの更新 
	// AutoUpdate=false の場合はこの関数を直接呼び出して下さい 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void UpdatePlayer(float DeltaSeconds);

	// 描画対象テクスチャを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintPure)
	UTexture* GetRenderTarget();

	// アニメーションの再生開始 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable, meta=(AdvancedDisplay="2"))
	bool Play(FName AnimPackName, FName AnimationName, int32 StartFrame=0, float PlayRate=1.f, int32 LoopCount=0, bool bRoundTrip=false);

	// アニメーションの再生開始(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable, meta=(AdvancedDisplay="2"))
	bool PlayByIndex(int32 AnimPackIndex, int32 AnimationIndex, int32 StartFrame=0, float PlayRate=1.f, int32 LoopCount=0, bool bRoundTrip=false);

	// シーケンスの再生開始 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable, meta=(AdvancedDisplay="2"))
	bool PlaySequence(FName SequencePackName, FName SequenceName, int32 StartFrame=0, float PlayRate=1.f);

	// シーケンスの再生開始(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable, meta=(AdvancedDisplay="2"))
	bool PlaySequenceByIndex(int32 SequencePackIndex, int32 SequenceIndex, int32 StartFrame=0, float PlayRate=1.f);

	// 再生中のアニメーション名を取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void GetPlayingAnimationName(FName& OutAnimPackName, FName& OutAnimationName) const;

	// 再生中のアニメーションインデックスを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void GetPlayingAnimationIndex(int32& OutAnimPackIndex, int32& OutAnimationIndex) const;

	// 再生中のシーケンス名を取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void GetPlayingSequenceName(FName& OutSequencePackName, FName& OutSequenceName) const;

	// 再生中のシーケンスインデックスを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void GetPlayingSequenceIndex(int32& OutSequencePackIndex, int32& OutSequenceIndex) const;

	// アニメーション再生の一時停止 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void Pause();

	// 一時停止したアニメーション再生の再開 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool Resume();

	// アニメーションが再生中かを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool IsPlaying() const;

	// シーケンスを再生中かを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool IsPlayingSequence() const;

	// セットされたSsProjectのAnimPack数を取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	int32 GetNumAnimPacks() const;

	// 指定されたAnimPackのAnimation数を取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	int32 GetNumAnimations(FName AnimPackName) const;

	// 指定されたAnimPackのAnimation数を取得(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	int32 GetNumAnimationsByIndex(int32 AnimPackIndex) const;

	// セットされたSsProjectのSequencePack数を取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	int32 GetNumSequencePacks() const;

	// 指定されたSequencePackのSequence数を取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	int32 GetNumSequences(FName SequencePackName) const;

	// 指定されたSequencePackのSequence数を取得(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	int32 GetNumSequencesByIndex(int32 SequencePackIndex) const;

	// シーケンスIDからインデックスを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool GetSequenceIndexById(FName SequencePackName, int32 SequenceId, int32& OutSequencePackIndex, int32& OutSequneceIndex) const;

	// 指定フレームへジャンプ 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void SetPlayFrame(float Frame);

	// 現在のフレームを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	float GetPlayFrame() const;

	// ループ数を設定(0で無限ループ) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void SetLoopCount(int32 InLoopCount);

	// 残りループ数を取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	int32 GetLoopCount() const;

	// 往復再生するかを設定 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void SetRoundTrip(bool bInRoundTrip);

	// 往復再生中かを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool IsRoundTrip() const;

	// 再生速度を設定(負の値で逆再生) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void SetPlayRate(float InRate=1.f);

	// 再生速度を取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	float GetPlayRate() const;

	// 水平反転の設定 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void SetFlipH(bool InFlipH);

	// 水平反転の取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool GetFlipH() const;

	// 垂直反転の設定 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void SetFlipV(bool InFlipV);

	// 垂直反転の取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool GetFlipV() const;

	// 置き換えテクスチャの登録 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void AddTextureReplacement(FName PartName, UTexture* Texture);

	// 置き換えテクスチャの登録(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void AddTextureReplacementByIndex(int32 PartIndex, UTexture* Texture);

	// 置き換えテクスチャの登録解除 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void RemoveTextureReplacement(FName PartName);

	// 置き換えテクスチャの登録解除(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void RemoveTextureReplacementByIndex(int32 PartIndex);

	// 全ての置き換えテクスチャの登録解除 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void RemoveTextureReplacementAll();

	// 置き換えマテリアルの登録(パーツ単位) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void AddMaterialReplacement(FName PartName, UMaterialInterface* InBaseMaterial);

	// 置き換えマテリアルの登録(パーツ単位)(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void AddMaterialReplacementByIndex(int32 PartIndex, UMaterialInterface* InBaseMaterial);

	// 置き換えマテリアルの登録解除(パーツ単位) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void RemoveMaterialReplacement(FName PartName);

	// 置き換えマテリアルの登録解除(パーツ単位)(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void RemoveMaterialReplacementByIndex(int32 PartIndex);

	// 全ての置き換えマテリアルの登録解除(パーツ単位) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void RemoveMaterialReplacementAll();

	// 置き換えマテリアルの登録(ブレンドモード単位) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void AddMaterialReplacementPerBlendMode(EAlphaBlendType AlphaBlendMode, EColorBlendType ColorBlendMode, UMaterialInterface* InBaseMaterial);

	// 置き換えマテリアルの登録解除(ブレンドモード単位) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void RemoveMaterialReplacementPerBlendMode(EAlphaBlendType AlphaBlendMode, EColorBlendType ColorBlendMode);

	// 全ての置き換えマテリアルの登録解除(ブレンドモード単位) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void RemoveMaterialReplacementAllPerBlendMode();

	// パーツのカラーラベルを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	FName GetPartColorLabel(FName PartName);

	// パーツのカラーラベルを取得(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	FName GetPartColorLabelByIndex(int32 PartIndex);

	// ウィジェット内のパーツのTransformを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool GetPartTransform(FName PartName, FVector2D& OutPosition, float& OutAngle, FVector2D& OutScale) const;

	// ウィジェット内のパーツのTransformを取得(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool GetPartTransformByIndex(int32 PartIndex, FVector2D& OutPosition, float& OutAngle, FVector2D& OutScale) const;

	// 名前からパーツインデックスを取得 
	UFUNCTION()
	int32 GetPartIndexFromName(FName InPartName);

	// 現在のアニメーション再生中、指定パーツを非表示にする 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void SetPartHidden(FName PartName, bool bHidden=true);

	// 現在のアニメーション再生中、指定パーツを非表示にする(インデックス指定) 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void SetPartHiddenByIndex(int32 PartIndex, bool bHidden=true);

	// 指定パーツ非表示状態を全解除 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void ResetPartHidden();


	// SlateのTickを有効/無効化 
	// 無効化するとアニメーションは一切更新されませんが、TickによるCPU負荷を削減出来ます 
	// 非表示状態や全く動きの無いアニメーション再生中のCPU負荷を削減したい場合に使用して下さい 
	UFUNCTION(Category="SpriteStudio|Optimize", BlueprintCallable)
	void SetCanSlateTick(bool bInCanTick=true);

	// SlateのTickが有効かを取得 
	UFUNCTION(Category="SpriteStudio|Optimize", BlueprintCallable)
	bool GetCanSlateTick() const;
};

