#pragma once


#include "SsHUD.generated.h"


UCLASS()
class SPRITESTUDIO6_API ASsHUD : public AHUD
{
	GENERATED_UCLASS_BODY()

public:
	// 指定したSsPlayerをCanvasに描画します 
	// Event Receive Draw HUD からのみ使用可能です 
	// アルファ値はテクスチャのアルファのみ反映されます 
	// アルファブレンドモード/カラーブレンドモードは反映されません 
	UFUNCTION(Category=SpriteStudio, BlueprintCallable, meta=(AdvancedDisplay="2"))
	void DrawSsPlayer(class USsPlayerComponent* SsPlayer, FVector2D Location, float Rotation=0.f, FVector2D Scale=FVector2D(1.f,1.f));

private:
	UPROPERTY(Category=SpriteStudio, VisibleAnywhere)
	class USsPlayerComponent* SsPlayerComponent;
};

