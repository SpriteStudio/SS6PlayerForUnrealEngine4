#include "SpriteStudio6PrivatePCH.h"
#include "SsRenderOffScreen.h"

#include "PipelineStateCache.h"
#include "MaterialShader.h"
#include "ClearQuad.h"

#include "SsOffScreenShaders.h"
#include "SsMaskShaders.h"
#include "Ss6Project.h"


DECLARE_GPU_STAT_NAMED(StatName_Ss6RenderOffScreen, TEXT("SpriteStudio6 RenderOffScreen") );

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
	if(0 < VertexNum)
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(VertexNum * sizeof(FSsOffScreenVertex), BUF_Dynamic, CreateInfo);
	}
}
void FSsOffScreenVertexBuffer::ReleaseDynamicRHI()
{
	VertexBufferRHI.SafeRelease();
}

// インデックスバッファ
void FSsOffScreenIndexBuffer::InitDynamicRHI()
{
	if(0 < IndexNum)
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexNum * sizeof(uint16), BUF_Dynamic, CreateInfo);
	}
}
void FSsOffScreenIndexBuffer::ReleaseDynamicRHI()
{
	IndexBufferRHI.SafeRelease();
}


FSsRenderOffScreen::FSsRenderOffScreen()
	: ClearColor(0,0,0,0)
	, bInitialized(false)
	, bTerminating(false)
	, VertexNum(0)
	, IndexNum(0)
{
}
FSsRenderOffScreen::~FSsRenderOffScreen()
{
	// 描画リソースが適切に開放されないままデストラクタが呼び出された
	check(!bInitialized);
	check(!bTerminating);
}

// レンダラの初期化 
void FSsRenderOffScreen::Initialize(uint32 InResolutionX, uint32 InResolutionY, uint32 InVertexNum, uint32 InIndexNum, bool bNeedMask)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsRenderOffScreen_Initialize);
	check(!bInitialized);

	RenderTarget = NewObject<UTextureRenderTarget2D>(UTextureRenderTarget2D::StaticClass());
	RenderTarget->AddToRoot();
	RenderTarget->SetFlags(RF_Transient);
	RenderTarget->RenderTargetFormat = RTF_RGBA8;
	RenderTarget->bForceLinearGamma = false;
	RenderTarget->AddressX = TA_Clamp;
	RenderTarget->AddressY = TA_Clamp;
	RenderTarget->InitAutoFormat(InResolutionX, InResolutionY);

	if(bNeedMask)
	{
		MaskRenderTarget = NewObject<UTextureRenderTarget2D>(UTextureRenderTarget2D::StaticClass());
		MaskRenderTarget->AddToRoot();
		MaskRenderTarget->SetFlags(RF_Transient);
		MaskRenderTarget->RenderTargetFormat = RTF_R8;
		MaskRenderTarget->bForceLinearGamma = false;
		MaskRenderTarget->AddressX = TA_Clamp;
		MaskRenderTarget->AddressY = TA_Clamp;
		MaskRenderTarget->InitAutoFormat(InResolutionX, InResolutionY);
	}

	VertexBuffer.VertexNum = VertexNum = InVertexNum;
	IndexBuffer.IndexNum = IndexNum = InIndexNum;

	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	
	bInitialized = true;
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

	if(MaskRenderTarget.IsValid())
	{
		MaskRenderTarget->RemoveFromRoot();
		MaskRenderTarget.Reset();
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
		VertexNum = IndexNum = 0;
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
		UTextureRenderTarget2D* MaskRenderTarget;
		FSsOffScreenVertexBuffer* VertexBuffer;
		FSsOffScreenIndexBuffer* IndexBuffer;
		FColor ClearColor;
		TArray<FSsRenderPart> RenderParts;
	};

	// カラーブレンドモードの設定 
	void SetVertexColorBlendType(SsBlendType::Type ColorBlendType, FVector2D& OutColorBlend)
	{
		switch(ColorBlendType)
		{
			case SsBlendType::Mix:
			case SsBlendType::Mul:
			case SsBlendType::Add:
			case SsBlendType::Sub:
				{
					OutColorBlend.X = (float)(ColorBlendType + 0.01f);
				} break;
			case SsBlendType::MulAlpha:
			case SsBlendType::Screen:
			case SsBlendType::Exclusion:
			case SsBlendType::Invert:
				{
					// カラーブレンドモードは Mix/Mul/Add/Sub のみ. 
					// MulAlpha/Screen/Exclusion/Invert はアルファブレンドモード専用. 
					check(false);
					OutColorBlend.X = 0.01f;
				} break;
			case SsBlendType::Effect:
				{
					// Effect
					OutColorBlend.X = 5.01f;
				} break;
			case SsBlendType::Mask:
				{
					OutColorBlend.X = 0.01f;
				} break;
			case SsBlendType::Invalid:
				{
					OutColorBlend.X = 4.01f;
				} break;
			default:
				{
					checkf(false, TEXT("Invalid ColorBlendType %d"), (int32)ColorBlendType);
				} break;
		}
	}

	// マスクバッファの描画 
	void RenderMaskBuffer(
		FRHICommandListImmediate& RHICmdList,
		const FSsRenderPartsForSendingRenderThread& RenderParts,
		int32 CurrentPartIndex
		)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_SsRenderOffScreen_RenderMaskBuffer);

		if(nullptr == RenderParts.MaskRenderTarget)
		{
			return;
		}

		// RenderTarget切り替え
		FRHIRenderPassInfo RPInfo(
			static_cast<FTextureRenderTarget2DResource*>(RenderParts.MaskRenderTarget->GetRenderTargetResource())->GetRenderTargetTexture(),
			ERenderTargetActions::DontLoad_Store,
			static_cast<FTextureRenderTarget2DResource*>(RenderParts.MaskRenderTarget->GetRenderTargetResource())->GetTextureRHI()
			);
		RHICmdList.BeginRenderPass(RPInfo, TEXT("Ss6RenderMaskBuffer"));

		DrawClearQuad(RHICmdList, FLinearColor::Black);


		// シェーダの取得 
		TShaderMapRef<FSsMaskVS> VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		TShaderMapRef<FSsMaskPS> PixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
		GraphicsPSOInit.RasterizerState   = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSs6OffScreenVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader->GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI  = PixelShader->GetPixelShader();

		FSamplerStateRHIRef SampleState = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();


		uint32 BaseVertexIndex = 0;
		uint32 BaseIndexIndex  = 0;
		for(int32 i = 0; i < RenderParts.RenderParts.Num(); ++i)
		{
			const FSsRenderPart& RenderPart = RenderParts.RenderParts[i];
			if(nullptr == RenderPart.Texture)
			{
				continue;
			}

			uint32 NumRenderVertices = 0;
			uint32 NumRenderIndices  = 0;
			if(0 == RenderPart.Mesh.Num())
			{
				check((4 == RenderPart.Vertices.Num()) || (5 == RenderPart.Vertices.Num()));
				if(4 == RenderPart.Vertices.Num())
				{
					NumRenderVertices = 4;
					NumRenderIndices  = 6;
				}
				else
				{
					NumRenderVertices = 5;
					NumRenderIndices  = 12;
				}
			}
			else
			{
				for(auto ItMesh = RenderPart.Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
				{
					NumRenderVertices += ItMesh->Vertices.Num();
					NumRenderIndices  += ItMesh->Indices.Num();
				}
			}

			// マスクバッファに書き込み 
			if((CurrentPartIndex < i) && (SsBlendType::Mask == RenderPart.ColorBlendType))
			{
				if(RenderPart.bMaskInfluence)
				{
					// Rチャンネルのみの加算描画を指定 
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RED,
						BO_Add, BF_One, BF_One
						>::GetRHI();
				}
				else
				{
					// マスク対象でない場合は上書き 
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RED,
						BO_Add, BF_One, BF_Zero
						>::GetRHI();
				}
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

				// テクスチャをセット 
				PixelShader->SetCellTexture(RHICmdList, RenderPart.Texture ? RenderPart.Texture->Resource->TextureRHI : nullptr, SampleState);

				RHICmdList.SetStreamSource(0, RenderParts.VertexBuffer->VertexBufferRHI, 0);
				RHICmdList.DrawIndexedPrimitive(
					RenderParts.IndexBuffer->IndexBufferRHI,
					0,						//BaseVertexIndex
					BaseVertexIndex,		//MinIndex
					NumRenderVertices,		//NumVertices
					BaseIndexIndex,			//StartIndex
					NumRenderIndices / 3,	//NumPrimitives
					1						//NumInstances
				);
			}

			BaseVertexIndex += NumRenderVertices;
			BaseIndexIndex  += NumRenderIndices;
		}

		RHICmdList.EndRenderPass();
	}

	// 描画 
	void RenderPartsToRenderTarget(FRHICommandListImmediate& RHICmdList, const FSsRenderPartsForSendingRenderThread& RenderParts)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_SsRenderOffScreen_RenderPartsToRenderTarget);
		SCOPED_DRAW_EVENT(RHICmdList, StatName_Ss6RenderOffScreen);
		SCOPED_GPU_STAT(RHICmdList, StatName_Ss6RenderOffScreen);

		check(IsInRenderingThread());

		float SurfaceWidth  = RenderParts.RenderTarget->GetSurfaceWidth();
		float SurfaceHeight = RenderParts.RenderTarget->GetSurfaceHeight();

		FRHIRenderPassInfo RPInfo(
			static_cast<FTextureRenderTarget2DResource*>(RenderParts.RenderTarget->GetRenderTargetResource())->GetRenderTargetTexture(),
			ERenderTargetActions::DontLoad_Store,
			static_cast<FTextureRenderTarget2DResource*>(RenderParts.RenderTarget->GetRenderTargetResource())->GetTextureRHI()
		);
		RHICmdList.BeginRenderPass(RPInfo, TEXT("Ss6RenderOffScreen"));
		RHICmdList.SetScissorRect(false, 0, 0, 0, 0);
		RHICmdList.SetViewport(0, 0, 0.f, SurfaceWidth, SurfaceHeight, 1.f);

		DrawClearQuad(
			RHICmdList,
			FLinearColor(
				RenderParts.ClearColor.R / 255.f,
				RenderParts.ClearColor.G / 255.f,
				RenderParts.ClearColor.B / 255.f,
				RenderParts.ClearColor.A / 255.f
				)
			);

		if(0 < RenderParts.RenderParts.Num())
		{
			// 頂点バッファへ書き込み 
			{
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

				void* VerticesPtr = RHILockVertexBuffer(
						RenderParts.VertexBuffer->VertexBufferRHI,
						0, // Offset
						RenderParts.VertexBuffer->VertexNum * sizeof(FSsOffScreenVertex),
						RLM_WriteOnly
						);
				int32 VertexCnt = 0;
				for(auto ItPart = RenderParts.RenderParts.CreateConstIterator(); ItPart; ++ItPart)
				{
					if(nullptr == ItPart->Texture)
					{
						continue;
					}

					FSsOffScreenVertex Vert;
					// 通常パーツ/マスクパーツ 
					if(0 == ItPart->Mesh.Num())
					{
						for(int32 v = 0; v < ItPart->Vertices.Num(); ++v)
						{
							FVector4 Position(
								ItPart->Vertices[v].Position.X * SurfaceWidth,
								ItPart->Vertices[v].Position.Y * SurfaceHeight,
								0.f, 1.f
								);
							Position = ProjectionMatrix.TransformFVector4(Position);
							Vert.Position.X = Position.X;
							Vert.Position.Y = Position.Y;
					
#if PLATFORM_SWITCH
							Vert.Color = FColor(ItPart->Vertices[v].Color.B, ItPart->Vertices[v].Color.G, ItPart->Vertices[v].Color.R, ItPart->Vertices[v].Color.A);
#else
							Vert.Color = ItPart->Vertices[v].Color;
#endif
							Vert.TexCoord = ItPart->Vertices[v].TexCoord;
					
							// カラーブレンドモードの設定 
							SetVertexColorBlendType(ItPart->ColorBlendType, Vert.ColorBlend);
							Vert.ColorBlend.Y = ItPart->Vertices[v].ColorBlendRate;
				
							check(VertexCnt < (int32)RenderParts.VertexBuffer->VertexNum);
							((FSsOffScreenVertex*)VerticesPtr)[VertexCnt] = Vert;
							++VertexCnt;
						}
					}
					// メッシュパーツ 
					else
					{
						for(auto ItMesh = ItPart->Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
						{
							for(auto ItVert = ItMesh->Vertices.CreateConstIterator(); ItVert; ++ItVert)
							{
								FVector4 Position(
									ItVert->Position.X * SurfaceWidth,
									ItVert->Position.Y * SurfaceHeight,
									0.f, 1.f
									);
								Position = ProjectionMatrix.TransformFVector4(Position);
								Vert.Position.X = Position.X;
								Vert.Position.Y = Position.Y;

								Vert.Color = ItMesh->Color;
								Vert.TexCoord = ItVert->TexCoord;

								SetVertexColorBlendType(ItPart->ColorBlendType, Vert.ColorBlend);
								Vert.ColorBlend.Y = ItMesh->ColorBlendRate;

								check(VertexCnt < (int32)RenderParts.VertexBuffer->VertexNum);
								((FSsOffScreenVertex*)VerticesPtr)[VertexCnt] = Vert;
								++VertexCnt;
							}
						}
					}
				}
				RHIUnlockVertexBuffer(RenderParts.VertexBuffer->VertexBufferRHI);
			}

			// インデックスバッファへ書き込み 
			{
				void* IndicesPtr = RHILockIndexBuffer(
					RenderParts.IndexBuffer->IndexBufferRHI,
					0,
					RenderParts.IndexBuffer->IndexNum * sizeof(uint16),
					RLM_WriteOnly
					);

				int32 VertexCnt = 0;
				int32 IndexCnt = 0;
				for(auto ItPart = RenderParts.RenderParts.CreateConstIterator(); ItPart; ++ItPart)
				{
					if(nullptr == ItPart->Texture)
					{
						continue;
					}

					FSsOffScreenVertex Vert;
					// 通常パーツ/マスクパーツ 
					if(0 == ItPart->Mesh.Num())
					{
						check((4 == ItPart->Vertices.Num()) || (5 == ItPart->Vertices.Num()));
						if(4 == ItPart->Vertices.Num())
						{
							check((uint32)(IndexCnt + 6) <= RenderParts.IndexBuffer->IndexNum);
							((uint16*)IndicesPtr)[IndexCnt + 0] = VertexCnt + 0;
							((uint16*)IndicesPtr)[IndexCnt + 1] = VertexCnt + 1;
							((uint16*)IndicesPtr)[IndexCnt + 2] = VertexCnt + 3;

							((uint16*)IndicesPtr)[IndexCnt + 3] = VertexCnt + 0;
							((uint16*)IndicesPtr)[IndexCnt + 4] = VertexCnt + 3;
							((uint16*)IndicesPtr)[IndexCnt + 5] = VertexCnt + 2;

							VertexCnt += 4;
							IndexCnt  += 6;
						}
						else
						{
							((uint16*)IndicesPtr)[IndexCnt +  0] = VertexCnt + 0;
							((uint16*)IndicesPtr)[IndexCnt +  1] = VertexCnt + 1;
							((uint16*)IndicesPtr)[IndexCnt +  2] = VertexCnt + 4;

							((uint16*)IndicesPtr)[IndexCnt +  3] = VertexCnt + 1;
							((uint16*)IndicesPtr)[IndexCnt +  4] = VertexCnt + 3;
							((uint16*)IndicesPtr)[IndexCnt +  5] = VertexCnt + 4;

							((uint16*)IndicesPtr)[IndexCnt +  6] = VertexCnt + 3;
							((uint16*)IndicesPtr)[IndexCnt +  7] = VertexCnt + 2;
							((uint16*)IndicesPtr)[IndexCnt +  8] = VertexCnt + 4;

							((uint16*)IndicesPtr)[IndexCnt +  9] = VertexCnt + 2;
							((uint16*)IndicesPtr)[IndexCnt + 10] = VertexCnt + 0;
							((uint16*)IndicesPtr)[IndexCnt + 11] = VertexCnt + 4;

							VertexCnt += 5;
							IndexCnt  += 12;
						}
					}
					// メッシュパーツ 
					else
					{
						for(auto ItMesh = ItPart->Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
						{
							check((uint32)(IndexCnt + ItMesh->Indices.Num()) <= RenderParts.IndexBuffer->IndexNum);
							for(auto ItIndex = ItMesh->Indices.CreateConstIterator(); ItIndex; ++ItIndex)
							{
								((uint16*)IndicesPtr)[IndexCnt + ItIndex.GetIndex()] = VertexCnt + *ItIndex;
							}
							VertexCnt += ItMesh->Vertices.Num();
							IndexCnt  += ItMesh->Indices.Num();
						}
					}
				}

				RHIUnlockIndexBuffer(RenderParts.IndexBuffer->IndexBufferRHI);
			}
		}

		// シェーダの取得 
		TShaderMapRef<FSsOffScreenVS>       VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		TShaderMapRef<FSsOffScreenPS>       PixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		TShaderMapRef<FSsOffScreenMaskedVS> MaskedVertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		TShaderMapRef<FSsOffScreenMaskedPS> MaskedPixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
		GraphicsPSOInit.RasterizerState   = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSs6OffScreenVertexDeclaration.VertexDeclarationRHI;

		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader->GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI  = PixelShader->GetPixelShader();

		FSamplerStateRHIRef SampleState = TStaticSamplerState<SF_Point,AM_Clamp,AM_Clamp,AM_Clamp>::GetRHI();

		// マテリアルとブレンドタイプが一致しているパーツ毎に描画 
		bool bNeedUpdateMask = true;
		uint32 BaseVertexIndex = 0;
		uint32 BaseIndexIndex  = 0;
		uint32 NumRenderVertices = 0;
		uint32 NumRenderIndices  = 0;
		for(int32 i = 0; i < RenderParts.RenderParts.Num(); ++i)
		{
			const FSsRenderPart& RenderPart = RenderParts.RenderParts[i];
			if(nullptr == RenderPart.Texture)
			{
				continue;
			}

			// マスクパーツ 
			if(SsBlendType::Mask == RenderPart.ColorBlendType)
			{
				BaseVertexIndex += 4;
				BaseIndexIndex  += 6;
				bNeedUpdateMask = true;
				continue;
			}

			// 通常パーツ 
			if(0 == RenderPart.Mesh.Num())
			{
				check((4 == RenderPart.Vertices.Num()) || (5 == RenderPart.Vertices.Num()));
				if(4 == RenderPart.Vertices.Num())
				{
					NumRenderVertices += 4;
					NumRenderIndices  += 6;
				}
				else
				{
					NumRenderVertices += 5;
					NumRenderIndices  += 12;
				}
			}
			// メッシュパーツ 
			else
			{
				for(auto ItMesh = RenderPart.Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
				{
					NumRenderVertices += ItMesh->Vertices.Num();
					NumRenderIndices  += ItMesh->Indices.Num();
				}
			}

			// 次パーツが同時に描画出来るか 
			if(    (i != (RenderParts.RenderParts.Num()-1))										// 最後の１つでない
				&& (RenderPart.AlphaBlendType == RenderParts.RenderParts[i+1].AlphaBlendType)	// アルファブレンドタイプが一致
				&& (RenderPart.Texture == RenderParts.RenderParts[i+1].Texture)					// テクスチャが一致
				&& (RenderParts.RenderParts[i].bMaskInfluence == RenderParts.RenderParts[i+1].bMaskInfluence)	// マスク対象かどうかが一致
				&& (RenderParts.RenderParts[i+1].ColorBlendType != SsBlendType::Mask)			// 次がマスクパーツでない
				)
			{
				continue;
			}

			// マスクバッファの描画
			if(bNeedUpdateMask && RenderPart.bMaskInfluence)
			{
				RHICmdList.EndRenderPass();
				RenderMaskBuffer(RHICmdList, RenderParts, i);
				RHICmdList.BeginRenderPass(RPInfo, TEXT("Ss6RenderOffScreen"));

				bNeedUpdateMask = false;
			}

			// マスク影響の有無でシェーダを切り替える 
			if(RenderPart.bMaskInfluence && (nullptr != RenderParts.MaskRenderTarget))
			{
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = MaskedVertexShader->GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI  = MaskedPixelShader->GetPixelShader();
			}
			else
			{
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader->GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI  = PixelShader->GetPixelShader();
			}


			switch(RenderPart.AlphaBlendType)
			{
			case SsBlendType::Mix:
				{
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha,
						BO_Add, BF_SourceAlpha, BF_One
						>::GetRHI();
				} break;
			case SsBlendType::Mul:
				{
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_Add, BF_Zero, BF_SourceColor,
						BO_Add, BF_InverseSourceAlpha, BF_One
						>::GetRHI();
				} break;
			case SsBlendType::Add:
				{
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_Add, BF_SourceAlpha, BF_One,
						BO_Add, BF_SourceAlpha, BF_One
						>::GetRHI();
				} break;
			case SsBlendType::Sub:
				{
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_ReverseSubtract, BF_SourceAlpha, BF_One,
						BO_Add, BF_Zero, BF_DestAlpha
						>::GetRHI();
				} break;
			case SsBlendType::MulAlpha:
				{
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_Add, BF_DestColor, BF_InverseSourceAlpha,
						BO_Add, BF_SourceAlpha, BF_One
						>::GetRHI();
				} break;
			case SsBlendType::Screen:
				{
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_Add, BF_InverseDestColor, BF_One,
						BO_Add, BF_SourceAlpha, BF_One
						>::GetRHI();
				} break;
			case SsBlendType::Exclusion:
				{
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_Add, BF_InverseDestColor, BF_InverseSourceColor,
						BO_Add, BF_SourceAlpha, BF_One
						>::GetRHI();
				} break;
			case SsBlendType::Invert:
				{
					GraphicsPSOInit.BlendState = TStaticBlendState<
						CW_RGBA,
						BO_Add, BF_InverseDestColor, BF_Zero,
						BO_Add, BF_SourceAlpha, BF_One
						>::GetRHI();
				} break;
			default:
				{
					check(false);
				}
			}
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

			// テクスチャをセット 
			if(RenderPart.bMaskInfluence && (nullptr != RenderParts.MaskRenderTarget))
			{
				MaskedPixelShader->SetCellTexture(RHICmdList, RenderPart.Texture ? RenderPart.Texture->Resource->TextureRHI : nullptr, SampleState);
				MaskedPixelShader->SetMaskTexture(RHICmdList, RenderParts.MaskRenderTarget ? RenderParts.MaskRenderTarget->Resource->TextureRHI : nullptr, SampleState);
			}
			else
			{
				PixelShader->SetCellTexture(RHICmdList, RenderPart.Texture ? RenderPart.Texture->Resource->TextureRHI : nullptr, SampleState);
			}

			RHICmdList.SetStreamSource(0, RenderParts.VertexBuffer->VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(
				RenderParts.IndexBuffer->IndexBufferRHI,
				0,						//BaseVertexIndex
				BaseVertexIndex,		//MinIndex
				NumRenderVertices,		//NumVertices
				BaseIndexIndex,			//StartIndex
				NumRenderIndices / 3,	//NumPrimitives
				1						//NumInstances
				);

			BaseVertexIndex += NumRenderVertices;
			BaseIndexIndex  += NumRenderIndices;
			NumRenderVertices = 0;
			NumRenderIndices  = 0;
		}

		RHICmdList.EndRenderPass();
	}
}

// ゲームスレッドからの描画命令発行 
void FSsRenderOffScreen::Render(const TArray<FSsRenderPart>& InRenderParts)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsRenderOffScreen_Render);
	check(IsInGameThread());
	
	if(!RenderTarget.IsValid())
	{
		return;
	}
	if(!bInitialized || bTerminating)
	{
		return;
	}

	FSsRenderPartsForSendingRenderThread RenderParts;
	{
		RenderParts.RenderTarget     = RenderTarget.Get();
		RenderParts.MaskRenderTarget = MaskRenderTarget.Get();
		RenderParts.VertexBuffer     = &VertexBuffer;
		RenderParts.IndexBuffer      = &IndexBuffer;
		RenderParts.ClearColor       = ClearColor;

		for(int32 i = 0; i < InRenderParts.Num(); ++i)
		{
			RenderParts.RenderParts.Add(InRenderParts[i]);
		}
	}

	ENQUEUE_RENDER_COMMAND(FSsRenderOffScreenRunner)(
		[RenderParts](FRHICommandListImmediate& RHICmdList)
		{
			RenderPartsToRenderTarget(RHICmdList, RenderParts);
		}
		);
}
