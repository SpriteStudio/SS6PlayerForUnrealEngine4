#pragma once

#include "SsGameSettings.generated.h"

class UMaterialInterface;


USTRUCT()
struct FSsColorBlendModeMaterials
{
	GENERATED_BODY()

public:
	// パーツカラー無し 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color UnUsed"))
	UMaterialInterface* Inv;

	// パーツカラー「ミックス」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color = [Mix]"))
	UMaterialInterface* Mix;

	// パーツカラー「乗算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color = [Mul]"))
	UMaterialInterface* Mul;

	// パーツカラー「加算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color = [Add]"))
	UMaterialInterface* Add;

	// パーツカラー「減算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Parts Color = [Sub]"))
	UMaterialInterface* Sub;

	// エフェクトパーツ 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Effect Parts"))
	UMaterialInterface* Eff;
};

USTRUCT()
struct FSsAlphaBlendModeMaterials
{
	GENERATED_BODY()

public:
	// 描画モード「ミックス」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Mix]"))
	FSsColorBlendModeMaterials Mix;

	// 描画モード「乗算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Mul]"))
	FSsColorBlendModeMaterials Mul;

	// 描画モード「加算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Add]"))
	FSsColorBlendModeMaterials Add;

	// 描画モード「減算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Sub]"))
	FSsColorBlendModeMaterials Sub;

	// 描画モード「α乗算」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [MulAlpha]"))
	FSsColorBlendModeMaterials MulAlpha;

	// 描画モード「スクリーン」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Screen]"))
	FSsColorBlendModeMaterials Screen;

	// 描画モード「除外」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Exclusion]"))
	FSsColorBlendModeMaterials Exclusion;

	// 描画モード「反転」 
	UPROPERTY(EditAnywhere, meta=(DisplayName="Alpha Blend Mode = [Invert]"))
	FSsColorBlendModeMaterials Invert;
};



UCLASS(config=Game, defaultconfig)
class USsGameSettings : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// Componentの「Default」で描画される際のマテリアルを設定します 
	UPROPERTY(EditAnywhere, config, Category="Base Materials per RenderMode")
	FSsAlphaBlendModeMaterials Component_Default;

	// UMGの「UMG_Default」で描画される際のマテリアルを設定します 
	UPROPERTY(EditAnywhere, config, Category="Base Materials per RenderMode")
	FSsAlphaBlendModeMaterials UMG_Default;

	UPROPERTY(Transient)
	UMaterialInterface* Component_OffScreen;

	UPROPERTY(Transient)
	UMaterialInterface* UMG_OffScreen;
};
