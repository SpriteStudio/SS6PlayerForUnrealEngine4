#include "SpriteStudio6PrivatePCH.h"
#include "SsRenderOffScreen.h"

#include "MaterialShader.h"
#include "ClearQuad.h"

#include "SsOffScreenShaders.h"
#include "Ss6Project.h"


//
// 開放予約された FSsRenderOffScreen を監視し、開放可能な状態になったらdeleteする 
//
class FSsOffScreenRenderDestroyer : public FTickableGameObject
{
public:
	virtual ~FSsOffScreenRenderDestroyer()
	{
		Tick(0.f);
	}

	virtual bool IsTickable() const override
	{
		return true;
	}
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FSsOffScreenRenderDestroyer, STATGROUP_Tickables);
	}

	virtual void Tick(float /*DeltaSeconds*/) override
	{
		for(int32 i = 0; i < DestroyRenderArray.Num(); ++i)
		{
			if(DestroyRenderArray[i]->CheckTerminate())
			{
				delete DestroyRenderArray[i];
				DestroyRenderArray.RemoveAt(i);
				--i;
			}
		}
	}

	void Add(FSsRenderOffScreen* InRender)
	{
		DestroyRenderArray.Add(InRender);
	}
private:
	TArray<FSsRenderOffScreen*> DestroyRenderArray;
};
FSsOffScreenRenderDestroyer GSs6OffScreenRenderDestroyer;



// 頂点バッファ
void FSsOffScreenVertexBuffer::InitDynamicRHI()
{
	uint32 BufferSize = (MaxPartsNum * 4) * sizeof(FSsOffScreenVertex);

	if(0 < BufferSize)
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(BufferSize, BUF_Dynamic, CreateInfo);
	}
}
void FSsOffScreenVertexBuffer::ReleaseDynamicRHI()
{
	VertexBufferRHI.SafeRelease();
}

// インデックスバッファ
void FSsOffScreenIndexBuffer::InitDynamicRHI()
{
	uint32 BufferSize = (MaxPartsNum * 6) * sizeof(uint32);

	if(0 < BufferSize)
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint32), BufferSize * sizeof(uint32), BUF_Dynamic, CreateInfo);

		void* IndicesPtr = RHILockIndexBuffer(IndexBufferRHI, 0, BufferSize, RLM_WriteOnly);
		for(uint32 i = 0; i < MaxPartsNum; ++i)
		{
			((uint32*)IndicesPtr)[i*6+0] = i*4+0;
			((uint32*)IndicesPtr)[i*6+1] = i*4+1;
			((uint32*)IndicesPtr)[i*6+2] = i*4+3;
			((uint32*)IndicesPtr)[i*6+3] = i*4+0;
			((uint32*)IndicesPtr)[i*6+4] = i*4+3;
			((uint32*)IndicesPtr)[i*6+5] = i*4+2;
		}
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
}
void FSsOffScreenIndexBuffer::ReleaseDynamicRHI()
{
	IndexBufferRHI.SafeRelease();
}


FSsRenderOffScreen::FSsRenderOffScreen()
	: bSupportAlphaBlendMode(false)
	, ClearColor(0,0,0,0)
	, bInitialized(false)
	, bTerminating(false)
	, MaxPartsNum(0)
{
}
FSsRenderOffScreen::~FSsRenderOffScreen()
{
	// 描画リソースが適切に開放されないままデストラクタが呼び出された
	check(!bInitialized);
	check(!bTerminating);
}

// レンダラの初期化 
void FSsRenderOffScreen::Initialize(uint32 InResolutionX, uint32 InResolutionY, uint32 InMaxPartsNum)
{
	check(!bInitialized);

	RenderTarget = NewObject<UTextureRenderTarget2D>(UTextureRenderTarget2D::StaticClass());
	RenderTarget->AddToRoot();
	RenderTarget->SetFlags(RF_Transient);
	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->bForceLinearGamma = false;
	RenderTarget->AddressX = TA_Clamp;
	RenderTarget->AddressY = TA_Clamp;
	RenderTarget->InitAutoFormat(InResolutionX, InResolutionY);

	VertexBuffer.MaxPartsNum = IndexBuffer.MaxPartsNum = MaxPartsNum = InMaxPartsNum;

	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	
	bInitialized = true;
}

// 指定した引数で再利用可能か 
bool FSsRenderOffScreen::CanReuse(uint32 NewResolutionX, uint32 NewResolutionY, uint32 NewMaxpartsNum) const
{
	if(!bInitialized)
	{
		return false;
	}
	if(NULL == RenderTarget)
	{
		return false;
	}
	if(    (NewMaxpartsNum <= VertexBuffer.MaxPartsNum)
		&& (NewResolutionX == (uint32)RenderTarget->GetSurfaceWidth())
		&& (NewResolutionY == (uint32)RenderTarget->GetSurfaceHeight())
		)
	{
		return true;
	}
	return false;
}

// 破棄予約 
void FSsRenderOffScreen::ReserveTerminate()
{
	BeginTerminate();
	GSs6OffScreenRenderDestroyer.Add(this);
}

// レンダラの後処理の開始 
void FSsRenderOffScreen::BeginTerminate()
{
	if(!bInitialized || bTerminating)
	{
		return;
	}

	if(RenderTarget.IsValid())
	{
		RenderTarget->RemoveFromRoot();
		RenderTarget.Reset();
	}

	BeginReleaseResource(&VertexBuffer);
	BeginReleaseResource(&IndexBuffer);

	ReleaseResourcesFence.BeginFence();

	bTerminating = true;
}

// レンダラの後処理の終了チェック 
// trueを返したら完了 
bool FSsRenderOffScreen::CheckTerminate()
{
	if(!bInitialized)
	{
		return true;
	}
	if(bTerminating && ReleaseResourcesFence.IsFenceComplete())
	{
		bInitialized = false;
		bTerminating = false;
		MaxPartsNum = 0;
		return true;
	}
	return false;
}

namespace
{
	// 描画スレッドへ送る用の描画パーツ全体情報 
	struct FSsRenderPartsForSendingRenderThread
	{
		UTextureRenderTarget2D* RenderTarget;
		FSsOffScreenVertexBuffer* VertexBuffer;
		FSsOffScreenIndexBuffer* IndexBuffer;
		FColor ClearColor;
		TArray<FSsRenderPart> RenderParts;
	};

	// 描画 
	void RenderPartsToRenderTarget(FRHICommandListImmediate& RHICmdList, FSsRenderPartsForSendingRenderThread& RenderParts)
	{
		check(IsInRenderingThread());

		float SurfaceWidth  = RenderParts.RenderTarget->GetSurfaceWidth();
		float SurfaceHeight = RenderParts.RenderTarget->GetSurfaceHeight();

		SetRenderTarget(
			RHICmdList,
			static_cast<FTextureRenderTarget2DResource*>(RenderParts.RenderTarget->GetRenderTargetResource())->GetTextureRHI(),
			FTextureRHIParamRef()
			);

		RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
		RHICmdList.SetViewport(0, 0, 0.f, SurfaceWidth, SurfaceHeight, 1.f);
		RHICmdList.GetContext().RHISetDepthStencilState(TStaticDepthStencilState<false, CF_Always>::GetRHI(), 0);
		RHICmdList.GetContext().RHISetRasterizerState(TStaticRasterizerState<FM_Solid, CM_None>::GetRHI());

		DrawClearQuad(
			RHICmdList,
			FLinearColor(
				RenderParts.ClearColor.R / 255.f,
				RenderParts.ClearColor.G / 255.f,
				RenderParts.ClearColor.B / 255.f,
				RenderParts.ClearColor.A / 255.f
				)
			);

		FMatrix ProjectionMatrix;
		{
			const float Left = 0.0f;
			const float Right = Left+SurfaceWidth;
			const float Top = 0.0f;
			const float Bottom = Top+SurfaceHeight;
			const float ZNear = 0.f;
			const float ZFar = 1.f;
			ProjectionMatrix =
					FMatrix(	FPlane(2.0f/(Right-Left),			0,							0,					0 ),
								FPlane(0,							2.0f/(Top-Bottom),			0,					0 ),
								FPlane(0,							0,							1/(ZNear-ZFar),		0 ),
								FPlane((Left+Right)/(Left-Right),	(Top+Bottom)/(Bottom-Top),	ZNear/(ZNear-ZFar), 1 ) );
		}

		// 頂点バッファへ書き込み 
		if(0 < RenderParts.RenderParts.Num())
		{
			void* VerticesPtr = RHILockVertexBuffer(
					RenderParts.VertexBuffer->VertexBufferRHI,
					0, // Offset
					RenderParts.RenderParts.Num() * sizeof(FSsOffScreenVertex) * 4, // SizeRHI
					RLM_WriteOnly
					);
			for(int32 i = 0; i < RenderParts.RenderParts.Num(); ++i)
			{
				FSsRenderPart& RenderPart = RenderParts.RenderParts[i];
				FSsOffScreenVertex Vert;
				for(int32 v = 0; v < 4; ++v)
				{
					FVector4 Position(
						RenderPart.Vertices[v].Position.X * SurfaceWidth,
						RenderPart.Vertices[v].Position.Y * SurfaceHeight,
						0.f, 1.f
						);
					Position = ProjectionMatrix.TransformFVector4(Position);
					Vert.Position.X = Position.X;
					Vert.Position.Y = Position.Y;
					
					Vert.Color = RenderPart.Vertices[v].Color;
					Vert.TexCoord = RenderPart.Vertices[v].TexCoord;
					
					// カラーブレンドモードの設定 
					switch(RenderPart.ColorBlendType)
					{
						case SsBlendType::Mix:
						case SsBlendType::Mul:
						case SsBlendType::Add:
						case SsBlendType::Sub:
							{
								Vert.ColorBlend.X = (float)(RenderPart.ColorBlendType + 0.01f);
							} break;
						case SsBlendType::MulAlpha:
						case SsBlendType::Screen:
						case SsBlendType::Exclusion:
						case SsBlendType::Invert:
							{
								// カラーブレンドモードは Mix/Mul/Add/Sub のみ. 
								// MulAlpha/Screen/Exclusion/Invert はアルファブレンドモード専用. 
								check(false);
								Vert.ColorBlend.X = 0.01f;
							} break;
						case SsBlendType::Effect:
							{
								// Effect
								Vert.ColorBlend.X = 5.01f;
							} break;
						case SsBlendType::MixVertex:
							{
								Vert.ColorBlend.X = 6.01f;
							} break;
						case SsBlendType::Invalid:
							{
								Vert.ColorBlend.X = 4.01f;
							} break;
						default:
							{
								checkf(false, TEXT("Invalid ColorBlendType %d"), (int32)RenderPart.ColorBlendType);
							} break;
					}
					Vert.ColorBlend.Y = RenderPart.Vertices[v].ColorBlendRate;
				
					((FSsOffScreenVertex*)VerticesPtr)[i*4+v] = Vert;
				}
			}
			RHIUnlockVertexBuffer(RenderParts.VertexBuffer->VertexBufferRHI);
		}

		// シェーダの取得 
		TShaderMapRef<FSsOffScreenVS> VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		TShaderMapRef<FSsOffScreenPS> PixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// マテリアルとブレンドタイプが一致しているパーツ毎に描画 
		uint32 StartPartIndex = 0;
		uint32 NumParts = 1;
		for(int32 i = 0; i < RenderParts.RenderParts.Num(); ++i)
		{
			FSsRenderPart& RenderPart = RenderParts.RenderParts[i];

			// 次パーツが同時に描画出来るか 
			if(    (i != (RenderParts.RenderParts.Num()-1))										// 最後の１つでない
				&& (RenderPart.AlphaBlendType == RenderParts.RenderParts[i+1].AlphaBlendType)	// アルファブレンドタイプが一致
				&& (RenderPart.Texture == RenderParts.RenderParts[i+1].Texture)					// テクスチャが一致
				)
			{
				++NumParts;
				continue;
			}

			RHICmdList.GetContext().RHISetBoundShaderState(
				RHICreateBoundShaderState(
					GSs6OffScreenVertexDeclaration.VertexDeclarationRHI,
					VertexShader->GetVertexShader(),	//VertexShaderRHI
					nullptr,							//HullShaderRHI
					nullptr,							//DomainShaderRHI
					PixelShader->GetPixelShader(),		//PixelShaderRHI
					FGeometryShaderRHIRef()
				));

			// テクスチャをセット
			FSamplerStateRHIRef SampleState = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();
			PixelShader->SetCellTexture(RHICmdList, RenderPart.Texture->Resource->TextureRHI, SampleState);


			switch(RenderPart.AlphaBlendType)
			{
			case SsBlendType::Mix:
				{
					RHICmdList.GetContext().RHISetBlendState(
						TStaticBlendState<
							CW_RGBA,
							BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha,
							BO_Add, BF_SourceAlpha, BF_One
							>::GetRHI(),
						FLinearColor::White
						);
				} break;
			case SsBlendType::Mul:
				{
					RHICmdList.GetContext().RHISetBlendState(
						TStaticBlendState<
							CW_RGBA,
							BO_Add, BF_Zero, BF_SourceColor,
							BO_Add, BF_InverseSourceAlpha, BF_One
							>::GetRHI(),
						FLinearColor::White
						);
				} break;
			case SsBlendType::Add:
				{
					RHICmdList.GetContext().RHISetBlendState(
						TStaticBlendState<
							CW_RGBA,
							BO_Add, BF_SourceAlpha, BF_One,
							BO_Add, BF_SourceAlpha, BF_One
							>::GetRHI(),
						FLinearColor::White
						);
				} break;
			case SsBlendType::Sub:
				{
					RHICmdList.GetContext().RHISetBlendState(
						TStaticBlendState<
							CW_RGBA,
							BO_ReverseSubtract, BF_SourceAlpha, BF_One,
							BO_Add, BF_Zero, BF_DestAlpha
							>::GetRHI(),
						FLinearColor::White
						);
				} break;
			}

			if(GRHISupportsBaseVertexIndex)
			{
				RHICmdList.SetStreamSource(0, RenderParts.VertexBuffer->VertexBufferRHI, 0);
				RHICmdList.DrawIndexedPrimitive(
					RenderParts.IndexBuffer->IndexBufferRHI,
					PT_TriangleList,
					(StartPartIndex * 4),	//BaseVertexIndex
					0,						//MinIndex
					(NumParts * 4),			//NumVertices
					0,						//StartIndex
					(NumParts * 2),			//NumPrimitives
					1						//NumInstances
					);
			}
			else
			{
				RHICmdList.SetStreamSource(
					0,												//StreamIndex
					RenderParts.VertexBuffer->VertexBufferRHI,
					sizeof(FSsOffScreenVertex) * (StartPartIndex * 4)	//Offset
					);
				RHICmdList.DrawIndexedPrimitive(
					RenderParts.IndexBuffer->IndexBufferRHI,
					PT_TriangleList,
					0,				//BaseVertexIndex
					0,				//MinIndex
					(NumParts * 4),	//NumVertices
					0,				//StartIndex
					(NumParts * 2),	//NumPrimitives
					1				//NumInstances
					);
			}

			StartPartIndex = i + 1;
			NumParts = 1;
		}
	}
}

// ゲームスレッドからの描画命令発行 
void FSsRenderOffScreen::Render(const TArray<FSsRenderPart>& InRenderParts)
{
	check(IsInGameThread());
	
	if(!RenderTarget.IsValid())
	{
		return;
	}
	if(!bInitialized || bTerminating)
	{
		return;
	}
	check((uint32)InRenderParts.Num() <= MaxPartsNum);


	FSsRenderPartsForSendingRenderThread RenderParts;
	{
		RenderParts.RenderTarget = RenderTarget.Get();
		RenderParts.VertexBuffer = &VertexBuffer;
		RenderParts.IndexBuffer  = &IndexBuffer;
		RenderParts.ClearColor   = ClearColor;

		for(int32 i = 0; i < InRenderParts.Num(); ++i)
		{
			RenderParts.RenderParts.Add(InRenderParts[i]);
		}
		if(!bSupportAlphaBlendMode)
		{
			for(int32 i = 0; i < InRenderParts.Num(); ++i)
			{
				RenderParts.RenderParts[i].AlphaBlendType = SsBlendType::Mix;
			}
		}
	}

	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		FSsRenderOffScreenRunner,
		FSsRenderPartsForSendingRenderThread, InRenderParts, RenderParts,
		{
			RenderPartsToRenderTarget(RHICmdList, InRenderParts);
		});
}
