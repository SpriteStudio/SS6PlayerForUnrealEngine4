#pragma once


// 頂点バッファ
class FSsOffScreenVertexBuffer : public FVertexBuffer
{
public:
	virtual void InitDynamicRHI() override;
	virtual void ReleaseDynamicRHI() override;
	uint32 MaxPartsNum;
};
// インデックスバッファ
class FSsOffScreenIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitDynamicRHI() override;
	virtual void ReleaseDynamicRHI() override;
	uint32 MaxPartsNum;
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

	void Initialize(uint32 InResolutionX, uint32 InResolutionY, uint32 InMaxPartsNum);
	bool IsInitialized() const { return bInitialized; }
	bool CanReuse(uint32 NewResolutionX, uint32 NewResolutionY, uint32 NewMaxpartsNum) const;
	void ReserveTerminate();

	void Render(const TArray<FSsRenderPart>& InRenderParts);

	UTextureRenderTarget2D* GetRenderTarget() { return RenderTarget.Get(); }

public:
	//
	// Mix以外のアルファブレンドモードをサポートするかどうか 
	//     オフスクリーンレンダリングの場合、あくまでRenderTarget上のカラーとしかブレンド出来ません 
	//     RenderTarget内に書き込まれたアルファ値がシーンに適用される際のブレンド方法は、その用途に依存することになります 
	//     そのため、本プラグインでのデフォルトの動作としては、オフスクリーンレンダリングでは アルファブレンドモード Mul/Add/Sub をサポートせず、全てMixとして扱います 
	//     但し、ビューアでのみ、背景色を設定し各種アルファブレンドをプレビュー出来るようにします 
	//     もしソースコードを編集してオフスクリーンでのブレンドモードを使用する場合は、FSsRenderOffScreenのコンストラクタでこの値をtrueに初期化して下さい 
	//
	bool bSupportAlphaBlendMode;
	FColor ClearColor;

private:
	void BeginTerminate();
	bool CheckTerminate();

	void Draw_GameThread();

private:
	bool bInitialized;
	bool bTerminating;
	uint32 MaxPartsNum;

	FRenderCommandFence ReleaseResourcesFence;

	TWeakObjectPtr<UTextureRenderTarget2D> RenderTarget;
	FSsOffScreenVertexBuffer VertexBuffer;
	FSsOffScreenIndexBuffer  IndexBuffer;
};

