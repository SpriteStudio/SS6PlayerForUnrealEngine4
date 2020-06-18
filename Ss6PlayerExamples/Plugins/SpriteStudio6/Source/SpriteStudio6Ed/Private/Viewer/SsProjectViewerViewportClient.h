#pragma once

class FSsPlayer;
class FSsRenderOffScreen;


class FSsProjectViewerViewportClient
	: public FViewportClient
	, public FGCObject
{

public:
	FSsProjectViewerViewportClient();
	virtual ~FSsProjectViewerViewportClient();

	// FViewportClient interface
	virtual void Draw(FViewport* Viewport, FCanvas* Canvas) override;
	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad) override;
	virtual bool InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples, bool bGamepad) override;

	// FGCObject interface
	virtual void AddReferencedObjects( FReferenceCollector& Collector ) {}

public:
	void SetPlayer(FSsPlayer* InPlayer, FSsRenderOffScreen* InRender);
	void SetBackgroundColor(const FLinearColor& InBackgroundColor);
	const FLinearColor& GetBackgroundColor() const { return BackgroundColor; }

private:
	void DrawGrid(FViewport* Viewport, FCanvas* Canvas, const FVector2D& Center);

public:
	bool bDrawGrid;
	int32 GridSize;
	FLinearColor GridColor;
	float RenderScale;
	float RenderScaleStep;
	FVector2D RenderOffset;

private:
	FLinearColor BackgroundColor;
	FSsPlayer* Player;
	FSsRenderOffScreen* Render;
	TWeakObjectPtr<UMaterialInstanceDynamic> ViewerMID;
};
