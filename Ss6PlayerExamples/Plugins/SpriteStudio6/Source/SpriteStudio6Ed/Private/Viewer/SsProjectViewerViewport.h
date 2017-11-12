#pragma once

#include "SceneViewport.h"

class FSsPlayer;
class FSsRenderOffScreen;


class SSsProjectViewerViewport : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSsProjectViewerViewport) {}
	SLATE_END_ARGS()

public:
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void Construct(const FArguments& InArgs);

	void SetPlayer(FSsPlayer* InPlayer, FSsRenderOffScreen* InRender);

private:
	TSharedPtr<FSceneViewport> Viewport;
	TSharedPtr<SViewport> ViewportWidget;
	
	FSsPlayer* Player;
	FSsRenderOffScreen* Render;


public:
	TSharedPtr<class FSsProjectViewerViewportClient> ViewportClient;

};
