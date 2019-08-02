#pragma once


// 頂点バッファ
class FSsOffScreenVertexBuffer : public FVertexBuffer
{
public:
	virtual void InitDynamicRHI() override;
	virtual void ReleaseDynamicRHI() override;
	uint32 VertexNum;
};
// インデックスバッファ
class FSsOffScreenIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitDynamicRHI() override;
	virtual void ReleaseDynamicRHI() override;
	uint32 IndexNum;
};

//
// SsPlayerの再生結果をオフスクリーンレンダリング. 
//
class SPRITESTUDIO6_API FSsRenderOffScreen
{
	friend class FSsOffScreenRenderDestroyer;

public:
	FSsRenderOffScreen();
	~FSsRenderOffScreen();

	void Initialize(uint32 InResolutionX, uint32 InResolutionY, uint32 InVertexNum, uint32 InIndexNum, bool bNeedMask);
	bool IsInitialized() const { return bInitialized; }
	void ReserveTerminate();

	void Render(const TArray<FSsRenderPart>& InRenderParts);

	UTextureRenderTarget2D* GetRenderTarget() { return RenderTarget.Get(); }

public:
	FColor ClearColor;

private:
	void BeginTerminate();
	bool CheckTerminate();

private:
	bool bInitialized;
	bool bTerminating;
	uint32 VertexNum;
	uint32 IndexNum;

	FRenderCommandFence ReleaseResourcesFence;

	TWeakObjectPtr<UTextureRenderTarget2D> RenderTarget;
	TWeakObjectPtr<UTextureRenderTarget2D> MaskRenderTarget;
	FSsOffScreenVertexBuffer VertexBuffer;
	FSsOffScreenIndexBuffer  IndexBuffer;

public:
	static void Startup();
	static void Shutdown();
};

