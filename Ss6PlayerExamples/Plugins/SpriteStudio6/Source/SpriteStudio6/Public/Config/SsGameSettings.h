#pragma once

#include "SsGameSettings.generated.h"

class UMaterialInterface;


USTRUCT()
struct SPRITESTUDIO6_API FSsColorBlendModeMaterials
{
	GENERATED_BODY()

public:
	// パーツカラー無し 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color UnUsed"), Category="Parts Color Setting")
	UMaterialInterface* Inv = nullptr;

	// パーツカラー「ミックス」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color = [Mix]"), Category="Parts Color Setting")
	UMaterialInterface* Mix = nullptr;

	// パーツカラー「乗算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color = [Mul]"), Category="Parts Color Setting")
	UMaterialInterface* Mul = nullptr;

	// パーツカラー「加算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color = [Add]"), Category="Parts Color Setting")
	UMaterialInterface* Add = nullptr;

	// パーツカラー「減算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color = [Sub]"), Category="Parts Color Setting")
	UMaterialInterface* Sub = nullptr;

	// エフェクトパーツ 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Effect Parts"), Category="Parts Effect Setting")
	UMaterialInterface* Eff = nullptr;
};

USTRUCT()
struct SPRITESTUDIO6_API FSsAlphaBlendModeMaterials
{
	GENERATED_BODY()

public:
	// 描画モード「ミックス」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Mix]"), Category="Alpha Blend Mode Setting")
	FSsColorBlendModeMaterials Mix;

	// 描画モード「乗算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Mul]"), Category="Alpha Blend Mode Setting")
	FSsColorBlendModeMaterials Mul;

	// 描画モード「加算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Add]"), Category="Alpha Blend Mode Setting")
	FSsColorBlendModeMaterials Add;

	// 描画モード「減算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Sub]"), Category="Alpha Blend Mode Setting")
	FSsColorBlendModeMaterials Sub;

	// 描画モード「α乗算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [MulAlpha]"), Category="Alpha Blend Mode Setting")
	FSsColorBlendModeMaterials MulAlpha;

	// 描画モード「スクリーン」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Screen]"), Category="Alpha Blend Mode Setting")
	FSsColorBlendModeMaterials Screen;

	// 描画モード「除外」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Exclusion]"), Category="Alpha Blend Mode Setting")
	FSsColorBlendModeMaterials Exclusion;

	// 描画モード「反転」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Invert]"), Category="Alpha Blend Mode Setting")
	FSsColorBlendModeMaterials Invert;
};



UCLASS(config=Game, defaultconfig)
class SPRITESTUDIO6_API USsGameSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// Componentの「Default」で描画される際のマテリアルを設定します 
	UPROPERTY(EditAnywhere, config, Category="Base Materials per RenderMode")
	FSsAlphaBlendModeMaterials Component_Default;

	// Componentの「Masked」で描画される際のマテリアルを設定します 
	UPROPERTY(EditAnywhere, config, Category="Base Materials per RenderMode")
	FSsAlphaBlendModeMaterials Component_Masked;

	// UMGの「UMG_Default」で描画される際のマテリアルを設定します 
	UPROPERTY(EditAnywhere, config, Category="Base Materials per RenderMode")
	FSsAlphaBlendModeMaterials UMG_Default;

	// UMGの「UMG_Masked」で描画される際のマテリアルを設定します 
	UPROPERTY(EditAnywhere, config, Category="Base Materials per RenderMode")
	FSsAlphaBlendModeMaterials UMG_Masked;

	UPROPERTY(Transient)
	UMaterialInterface* Component_OffScreen;

	UPROPERTY(Transient)
	UMaterialInterface* UMG_OffScreen;
};
