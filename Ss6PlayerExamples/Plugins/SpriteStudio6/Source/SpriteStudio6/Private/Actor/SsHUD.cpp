#include "SpriteStudio6PrivatePCH.h"
#include "SsHUD.h"

#include "SsPlayerComponent.h"


// コンストラクタ
ASsHUD::ASsHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SsPlayerComponent = ObjectInitializer.CreateDefaultSubobject<USsPlayerComponent>(this, TEXT("SsPlayerComponent"));
	SsPlayerComponent->RenderMode = ESsPlayerComponentRenderMode::Canvas;
	RootComponent = SsPlayerComponent;
}

// SsPlayerをHUDに描画
void ASsHUD::DrawSsPlayer(USsPlayerComponent* PlayerComponent, FVector2D Location, float Rotation, FVector2D Scale)
{
	if(NULL == Canvas)
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("ASsHUD::DrawSsPlayer() Only valid during PostRender() event."));
		return;
	}

	// NULL指定であればRootのSsPlayerComponentを使用する 
	if(NULL == PlayerComponent)
	{
		PlayerComponent = SsPlayerComponent;
	}

	PlayerComponent->RenderToCanvas(Canvas, Location, Rotation, Scale);
}
