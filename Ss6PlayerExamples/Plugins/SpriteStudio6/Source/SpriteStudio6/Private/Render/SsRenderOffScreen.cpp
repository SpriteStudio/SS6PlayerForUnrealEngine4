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
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint32), IndexNum * sizeof(uint32), BUF_Dynamic, CreateInfo);
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
void FSsRenderOffScreen::Initialize(uint32 InResolutionX, uint32 InResolutionY, uint32 InVertexNum, uint32 InIndexNum)
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

	VertexBuffer.VertexNum = VertexNum = InVertexNum;
	IndexBuffer.IndexNum = IndexNum = InIndexNum;

	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	
	bInitialized = true;
}

// 指定した引数で再利用可能か 
bool FSsRenderOffScreen::CanReuse(uint32 NewResolutionX, uint32 NewResolutionY, uint32 NewVertexNum, uint32 NewIndexNum) const
{
	if(!bInitialized)
	{
		return false;
	}
	if(NULL == RenderTarget)
	{
		return false;
	}
	if(    (NewVertexNum <= VertexBuffer.VertexNum)
		&& (NewIndexNum  <= IndexBuffer.IndexNum)
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
			case SsBlendType::MixVertex:
				{
					OutColorBlend.X = 6.01f;
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
					FSsOffScreenVertex Vert;
					// 通常パーツ 
					if(0 == ItPart->Mesh.Num())
					{
						for(int32 v = 0; v < 4; ++v)
						{
							FVector4 Position(
								ItPart->Vertices[v].Position.X * SurfaceWidth,
								ItPart->Vertices[v].Position.Y * SurfaceHeight,
								0.f, 1.f
								);
							Position = ProjectionMatrix.TransformFVector4(Position);
							Vert.Position.X = Position.X;
							Vert.Position.Y = Position.Y;
					
							Vert.Color = ItPart->Vertices[v].Color;
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
					RenderParts.IndexBuffer->IndexNum * sizeof(uint32),
					RLM_WriteOnly
					);

				int32 VertexCnt = 0;
				int32 IndexCnt = 0;
				for(auto ItPart = RenderParts.RenderParts.CreateConstIterator(); ItPart; ++ItPart)
				{
					FSsOffScreenVertex Vert;
					// 通常パーツ 
					if(0 == ItPart->Mesh.Num())
					{
						check((IndexCnt + 6) <= (int32)RenderParts.IndexBuffer->IndexNum);
						((uint32*)IndicesPtr)[IndexCnt + 0] = VertexCnt + 0;
						((uint32*)IndicesPtr)[IndexCnt + 1] = VertexCnt + 1;
						((uint32*)IndicesPtr)[IndexCnt + 2] = VertexCnt + 3;
						((uint32*)IndicesPtr)[IndexCnt + 3] = VertexCnt + 0;
						((uint32*)IndicesPtr)[IndexCnt + 4] = VertexCnt + 3;
						((uint32*)IndicesPtr)[IndexCnt + 5] = VertexCnt + 2;
						VertexCnt += 4;
						IndexCnt  += 6;
					}
					// メッシュパーツ 
					else
					{
						for(auto ItMesh = ItPart->Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
						{
							check((IndexCnt + ItMesh->Indices.Num()) <= (int32)RenderParts.IndexBuffer->IndexNum);
							for(auto ItIndex = ItMesh->Indices.CreateConstIterator(); ItIndex; ++ItIndex)
							{
								((uint32*)IndicesPtr)[IndexCnt + ItIndex.GetIndex()] = VertexCnt + *ItIndex;
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
		TShaderMapRef<FSsOffScreenVS> VertexShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
		TShaderMapRef<FSsOffScreenPS> PixelShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

		// マテリアルとブレンドタイプが一致しているパーツ毎に描画 
		uint32 BaseVertexIndex = 0;
		uint32 BaseIndexIndex  = 0;
		uint32 NumRenderVertices = 0;
		uint32 NumRenderIndices  = 0;
		for(int32 i = 0; i < RenderParts.RenderParts.Num(); ++i)
		{
			FSsRenderPart& RenderPart = RenderParts.RenderParts[i];

			// 通常パーツ 
			if(0 == RenderPart.Mesh.Num())
			{
				NumRenderVertices += 4;
				NumRenderIndices  += 6;
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
				)
			{
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
			PixelShader->SetCellTexture(RHICmdList, RenderPart.Texture ? RenderPart.Texture->Resource->TextureRHI : nullptr, SampleState);


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
			case SsBlendType::MulAlpha:
				{
					RHICmdList.GetContext().RHISetBlendState(
						TStaticBlendState<
							CW_RGBA,
							BO_Add, BF_DestColor, BF_InverseSourceAlpha,
							BO_Add, BF_SourceAlpha, BF_One
							>::GetRHI(),
						FLinearColor::White
						);
				} break;
			case SsBlendType::Screen:
				{
					RHICmdList.GetContext().RHISetBlendState(
						TStaticBlendState<
							CW_RGBA,
							BO_Add, BF_InverseDestColor, BF_One,
							BO_Add, BF_SourceAlpha, BF_One
							>::GetRHI(),
						FLinearColor::White
						);
				} break;
			case SsBlendType::Exclusion:
				{
					RHICmdList.GetContext().RHISetBlendState(
						TStaticBlendState<
							CW_RGBA,
							BO_Add, BF_InverseDestColor, BF_InverseSourceColor,
							BO_Add, BF_SourceAlpha, BF_One
							>::GetRHI(),
						FLinearColor::White
						);
				} break;
			case SsBlendType::Invert:
				{
					RHICmdList.GetContext().RHISetBlendState(
						TStaticBlendState<
							CW_RGBA,
							BO_Add, BF_InverseDestColor, BF_Zero,
							BO_Add, BF_SourceAlpha, BF_One
							>::GetRHI(),
						FLinearColor::White
						);
				} break;
			}

			RHICmdList.SetStreamSource(0, RenderParts.VertexBuffer->VertexBufferRHI, 0);
			RHICmdList.DrawIndexedPrimitive(
				RenderParts.IndexBuffer->IndexBufferRHI,
				PT_TriangleList,
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
