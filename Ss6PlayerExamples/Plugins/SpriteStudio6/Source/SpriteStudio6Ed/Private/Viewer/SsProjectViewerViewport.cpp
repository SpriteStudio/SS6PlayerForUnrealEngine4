#include "SsProjectViewerViewport.h"

#include "SsPlayer.h"
#include "SsRenderOffScreen.h"
#include "SsProjectViewerViewportClient.h"


void SSsProjectViewerViewport::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if(Player)
	{
		Player->Tick(InDeltaTime);

		if(Render)
		{
			Render->Render(Player->GetRenderParts());
		}
	}

	Viewport->InvalidateDisplay();
}

void SSsProjectViewerViewport::Construct(const FArguments& InArgs)
{
	this->ChildSlot
	[
		SAssignNew(ViewportWidget, SViewport)
			.EnableGammaCorrection(false)
			.ShowEffectWhenDisabled(false)
			.EnableBlending(true)
	];

	ViewportClient = MakeShareable(new FSsProjectViewerViewportClient());

	Viewport = MakeShareable(new FSceneViewport(ViewportClient.Get(), ViewportWidget));

	ViewportWidget->SetViewportInterface( Viewport.ToSharedRef() );
}

void SSsProjectViewerViewport::SetPlayer(FSsPlayer* InPlayer, FSsRenderOffScreen* InRender)
{
	Player = InPlayer;
	Render = InRender;
	ViewportClient->SetPlayer(Player, Render);
}
