#pragma once

#include "Components/MeshComponent.h"

#include "SsTypes.h"
#include "SsPlayerTickResult.h"
#include "SsPlayer.h"
#include "SsPlayPropertySync.h"

class USs6Project;
class FSsRenderPlaneProxy;
class FSsRenderOffScreen;

#include "SsPlayerComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSsEndPlaySignature, FName, AnimPackName, FName, AnimationName, int32, AnimPackIndex, int32, AnimationIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSsUserDataSignature, FName, PartName, int32, PartIndex, int32, KeyFrame, FSsUserDataValue, UserData);


// SsPlayerComponentの描画モード 
UENUM()
namespace ESsPlayerComponentRenderMode
{
	enum Type
	{
		// デフォルトの描画モードです 
		// パーツ毎のポリゴンを3D空間上に描画します 
		// アルファブレンドモードはミックス/加算/減算のみ反映されます 
		// 通常はこのモードを使用して下さい 
		Default,

		// 描画方法はDefaultと同等ですが、Maskedマテリアルを使用するため、半透明は２値化されます 
		// アルファブレンドモードはミックスのみ反映されます 
		Masked,

		// 一旦オフスクリーンレンダリングしたテクスチャを、板ポリに貼り付けます 
		// 板ポリに貼り付ける際のマテリアルを上書き可能です 
		// マテリアルを利用した特殊なエフェクトを実装したい際などに使用して下さい 
		OffScreenPlane,

		// オフスクリーンレンダリングのみを行い、ゲーム画面への描画は行いません 
		// GetRenderTargetでレンダリング結果のテクスチャを取得し、自由に利用出来ます 
		OffScreenOnly,
	};
}


//
// SpriteStudio6 Player Component 
// sspjデータを再生/描画する 
//
UCLASS(ClassGroup=SpriteStudio,meta=(BlueprintSpawnableComponent))
class SPRITESTUDIO6_API USsPlayerComponent : public UMeshComponent, public FSsPlayPropertySync 
{
	GENERATED_UCLASS_BODY()

public:
	// UObject interface
	virtual void Serialize(FArchive& Ar) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// USceneComponent interface
	virtual bool HasAnySockets() const override;
	virtual bool DoesSocketExist(FName InSocketName) const override;
	virtual void QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const override;
	virtual FTransform GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace = RTS_World) const override;

	// UActorComponent interface
	virtual void OnRegister() override;
	virtual void OnUnregister() override;
	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void SendRenderDynamicData_Concurrent() override;

	// UPrimitiveComponent interface
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;


	// アニメーションの更新 
	// AutoUpdate=false の場合はこの関数を直接呼び出して下さい 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	void UpdatePlayer(float DeltaSeconds);

	// 描画対象テクスチャを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintPure)
	UTexture* GetRenderTarget();

	// パーツのアタッチ用Transformを取得 
	UFUNCTION(Category="SpriteStudio", BlueprintCallable)
	bool GetPartAttachTransform(int32 PartIndex, FTransform& OutTransform) const;

#if WITH_EDITOR
	// コード上で直接SsProjectをセットした場合に呼び出す 
	void OnSetSsProject();
#endif

private:
	FSsPlayer Player;
	FSsRenderOffScreen* RenderOffScreen;

	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> PartsMIDRef;

	TMap<UMaterialInterface*, TMap<UTexture*, UMaterialInstanceDynamic*>> PartsMIDMaps;

	UPROPERTY(Transient)
	TMap<int32, UMaterialInterface*> MaterialReplacementMap;

	UPROPERTY(Transient)
	TMap<int32, UMaterialInterface*> MapterialReplacementMapPerBlendMode;


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

	// 自動再生時の水平反転 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	bool bAutoPlayFlipH;

	// 自動再生時の垂直反転 
	UPROPERTY(Category="SpriteStudioPlaySettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	bool bAutoPlayFlipV;


	//
	// SpriteStudioRenderSettings
	//

	// 描画モード 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly)
	TEnumAsByte<ESsPlayerComponentRenderMode::Type> RenderMode;

	// Masked描画モードでのZファイト防止用の、１パーツあたりのPixelDepthOffset値 
	// カメラとの距離などによってZファイトが発生するようであれば、大きな値を設定して下さい 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	float PixelDepthOffsetPerPart;

	// OffScreen描画モードでの、メッシュを描画する際のベースマテリアル 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly, meta=(DisplayName="OffScreen Material", DisplayThumbnail="true"), AdvancedDisplay)
	UMaterialInterface* BaseMaterial;

	// OffScreenPlane用MID 
	UPROPERTY(Transient, BlueprintReadOnly)
	UMaterialInstanceDynamic* OffScreenPlaneMID;

	// オフスクリーンレンダリングの際の解像度 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	FVector2D OffScreenRenderResolution;

	// オフスクリーンレンダリングの際のクリアカラー 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadWrite, AdvancedDisplay)
	FColor OffScreenClearColor;

	// アニメーションのCanvasサイズに対する、メッシュ描画サイズ 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	float UUPerPixel;

	// カリング用半径への係数 
	// パーツがアニメーションのキャンバス範囲外へ動く場合は、このスケールを設定して下さい 
	// 描画モードがOffScreenPlaneの場合は無視されます 
	UPROPERTY(Category="SpriteStudioRenderSettings", EditAnywhere, BlueprintReadOnly, AdvancedDisplay)
	float SsBoundsScale;


	//
	// SpriteStudioCallback 
	//

	// 再生終了イベント 
	UPROPERTY(BlueprintAssignable, Category="SpriteStudioCallback")
	FSsEndPlaySignature OnSsEndPlay;

	// ユーザーデータイベント 
	UPROPERTY(BlueprintAssignable, Category="SpriteStudioCallback")
	FSsUserDataSignature OnSsUserData;


	//
	// Misc 
	//

	// 実際に描画に使用されているMaterialInstanceDynamic 
	UPROPERTY(Transient, BlueprintReadOnly, Category="SpriteStudio")
	TArray<UMaterialInstanceDynamic*> RenderMIDs;


public:
	// Blueprint公開関数 

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
	UFUNCTION(Category = SpriteStudio, BlueprintCallable)
	FName GetPartColorLabel(FName PartName);

	// パーツのカラーラベルを取得(インデックス指定) 
	UFUNCTION(Category = SpriteStudio, BlueprintCallable)
	FName GetPartColorLabelByIndex(int32 PartIndex);


	// 現在のアニメーション再生中、指定パーツを非表示にする 
	UFUNCTION(Category = SpriteStudio, BlueprintCallable)
	void SetPartHidden(FName PartName, bool bHidden=true);

	// 現在のアニメーション再生中、指定パーツを非表示にする(インデックス指定) 
	UFUNCTION(Category = SpriteStudio, BlueprintCallable)
	void SetPartHiddenByIndex(int32 PartIndex, bool bHidden=true);

	// 指定パーツ非表示状態を全解除 
	UFUNCTION(Category = SpriteStudio, BlueprintCallable)
	void ResetPartHidden();
};
