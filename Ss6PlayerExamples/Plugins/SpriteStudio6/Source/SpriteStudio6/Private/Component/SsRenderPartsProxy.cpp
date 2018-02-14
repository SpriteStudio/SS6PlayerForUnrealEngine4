#include "SpriteStudio6PrivatePCH.h"
#include "SsRenderPartsProxy.h"

#include "RHIStaticStates.h"
#include "DynamicMeshBuilder.h"

#include "SsPlayerComponent.h"


namespace
{
	static SsBlendType::Type TypeMix = SsBlendType::Mix;
	static SsBlendType::Type TypeMul = SsBlendType::Mul;
	static SsBlendType::Type TypeAdd = SsBlendType::Add;
	static SsBlendType::Type TypeSub = SsBlendType::Sub;
	static SsBlendType::Type TypeMulAlpha  = SsBlendType::MulAlpha;
	static SsBlendType::Type TypeScreen    = SsBlendType::Screen;
	static SsBlendType::Type TypeExclusion = SsBlendType::Exclusion;
	static SsBlendType::Type TypeInvert    = SsBlendType::Invert;
	void* GetBlendTypeAddr(SsBlendType::Type Type)
	{
		switch(Type)
		{
			case SsBlendType::Mix: { return &TypeMix; }
			case SsBlendType::Mul: { return &TypeMul; }
			case SsBlendType::Add: { return &TypeAdd; }
			case SsBlendType::Sub: { return &TypeSub; }
			case SsBlendType::MulAlpha:  { return &TypeMulAlpha;  }
			case SsBlendType::Screen:    { return &TypeScreen;    }
			case SsBlendType::Exclusion: { return &TypeExclusion; }
			case SsBlendType::Invert:    { return &TypeInvert;    }
		}
		return NULL;
	}
	SsBlendType::Type GetBlendTypeFromAddr(const void* Addr)
	{
		if(Addr == &TypeMix){ return SsBlendType::Mix; }
		if(Addr == &TypeMul){ return SsBlendType::Mul; }
		if(Addr == &TypeAdd){ return SsBlendType::Add; }
		if(Addr == &TypeSub){ return SsBlendType::Sub; }
		if(Addr == &TypeMulAlpha) { return SsBlendType::MulAlpha;  }
		if(Addr == &TypeScreen)   { return SsBlendType::Screen;    }
		if(Addr == &TypeExclusion){ return SsBlendType::Exclusion; }
		if(Addr == &TypeInvert)   { return SsBlendType::Invert;    }
		return SsBlendType::Mix;
	}
};

//
// VertexBuffer 
//
void FSsPartsVertexBuffer::InitRHI()
{
	if(0 < NumVerts)
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(NumVerts * sizeof(FSsRenderPartsProxy::FSsPartVertex), BUF_Dynamic, CreateInfo);
	}
}

//
// IndexBuffer 
//
void FSsPartsIndexBuffer::InitRHI()
{
	if(0 < NumIndices)
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint32), NumIndices * sizeof(uint32), BUF_Dynamic, CreateInfo);

		void* IndicesPtr = RHILockIndexBuffer(IndexBufferRHI, 0, NumIndices * sizeof(uint32), RLM_WriteOnly);
		for(uint32 i = 0; i < NumIndices/6; ++i)
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

//
// VertexFactory 
//
IMPLEMENT_VERTEX_FACTORY_TYPE(FSsPartsVertexFactory, "/Engine/Private/LocalVertexFactory.ush", true, true, true, true, true);

bool FSsPartsVertexFactory::ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
{
	return (   Material->GetFriendlyName().StartsWith("WorldGridMaterial")
			|| Material->GetFriendlyName().StartsWith("WireframeMaterial")
			|| Material->GetFriendlyName().StartsWith("SsPart_")
			)
		&& FLocalVertexFactory::ShouldCache(Platform, Material, ShaderType);
}
void FSsPartsVertexFactory::ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment)
{
	FLocalVertexFactory::ModifyCompilationEnvironment(Platform, Material, OutEnvironment);
}
FVertexFactoryShaderParameters* FSsPartsVertexFactory::ConstructShaderParameters(EShaderFrequency ShaderFrequency)
{
	return (ShaderFrequency == SF_Pixel) ? new FSsPartVertexFactoryShaderParameters() : NULL;
}
void FSsPartsVertexFactory::Init(const FSsPartsVertexBuffer* VertexBuffer)
{
	if(IsInRenderingThread())
	{
		FLocalVertexFactory::FDataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FSsRenderPartsProxy::FSsPartVertex, Position, VET_Float3);
		NewData.TextureCoordinates.Add(FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FSsRenderPartsProxy::FSsPartVertex,TexCoord), sizeof(FSsRenderPartsProxy::FSsPartVertex), VET_Float2));
		NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FSsRenderPartsProxy::FSsPartVertex, Color, VET_Color);
		NewData.TextureCoordinates.Add(FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FSsRenderPartsProxy::FSsPartVertex,ColorBlend), sizeof(FSsRenderPartsProxy::FSsPartVertex), VET_Float2));
		NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FSsRenderPartsProxy::FSsPartVertex, TangentX, VET_PackedNormal);
		NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FSsRenderPartsProxy::FSsPartVertex, TangentZ, VET_PackedNormal);
		SetData(NewData);
	}
	else
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			FInitSsPartsVertexFactory,
			FSsPartsVertexFactory*, VertexFactory, this,
			const FSsPartsVertexBuffer*, VertexBuffer, VertexBuffer,
		{
			FDataType NewData;
			NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FSsRenderPartsProxy::FSsPartVertex, Position, VET_Float3);
			NewData.TextureCoordinates.Add(FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FSsRenderPartsProxy::FSsPartVertex,TexCoord), sizeof(FSsRenderPartsProxy::FSsPartVertex), VET_Float2));
			NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FSsRenderPartsProxy::FSsPartVertex, Color, VET_Color);
			NewData.TextureCoordinates.Add(FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FSsRenderPartsProxy::FSsPartVertex,ColorBlend), sizeof(FSsRenderPartsProxy::FSsPartVertex), VET_Float2));
			NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FSsRenderPartsProxy::FSsPartVertex, TangentX, VET_PackedNormal);
			NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FSsRenderPartsProxy::FSsPartVertex, TangentZ, VET_PackedNormal);
			VertexFactory->SetData(NewData);
		});
	}
}

//
// VertexFactoryShaderParameters
//
void FSsPartVertexFactoryShaderParameters::SetMesh(FRHICommandList& RHICmdList, FShader* Shader,const class FVertexFactory* VertexFactory,const class FSceneView& View,const struct FMeshBatchElement& BatchElement,uint32 DataFlags) const
{
	SsBlendType::Type AlphaBlendType = GetBlendTypeFromAddr(BatchElement.UserData);
	switch(AlphaBlendType)
	{
		case SsBlendType::Mix:
			{
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
}


// コンストラクタ
FSsRenderPartsProxy::FSsRenderPartsProxy(USsPlayerComponent* InComponent, uint32 InMaxVertexNum, uint32 InMaxIndexNum)
	: FPrimitiveSceneProxy(InComponent)
{
	// FPrimitiveSceneProxy
	bWillEverBeLit = false;

	Component = InComponent;

	VertexBuffer.NumVerts  = InMaxVertexNum;
	IndexBuffer.NumIndices = InMaxIndexNum;
	VertexFactory.Init(&VertexBuffer);

	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	bVerifyUsedMaterials = false;
}

// デストラクタ
FSsRenderPartsProxy::~FSsRenderPartsProxy()
{
	VertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
}


void FSsRenderPartsProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_SsRenderSceneProxy_GetDynamicMeshElements );

	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
	FMaterialRenderProxy* MaterialProxy = NULL;
	if(bWireframe)
	{
		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
			FLinearColor(0, 0.5f, 1.f)
			);
		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		MaterialProxy = WireframeMaterialInstance;
	}

	for(int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if(VisibilityMap & (1 << ViewIndex))
		{
			for(auto ItPrim = RenderPrimitives.CreateConstIterator(); ItPrim; ++ItPrim)
			{
				const FSceneView* View = Views[ViewIndex];

				if(!bWireframe)
				{
					if(nullptr == ItPrim->Material)
					{
						continue;
					}
					MaterialProxy = ItPrim->Material->GetRenderProxy(IsSelected());
				}

				// Draw the mesh.
				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.bDisableBackfaceCulling = true;
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;
				BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
				BatchElement.FirstIndex = ItPrim->FirstIndex;
				BatchElement.NumPrimitives = ItPrim->NumPrimitives;
				BatchElement.MinVertexIndex = ItPrim->MinVertexIndex;
				BatchElement.MaxVertexIndex = ItPrim->MaxVertexIndex;
				BatchElement.UserData = GetBlendTypeAddr(ItPrim->AlphaBlendType);
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;
				Collector.AddMesh(ViewIndex, Mesh);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				// Render bounds
				RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
#endif
			}
		}
	}
}

FPrimitiveViewRelevance FSsRenderPartsProxy::GetViewRelevance(const FSceneView* View) const
{
	// どこかでちゃんと精査しないと・・・
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance = IsShown(View);
	Result.bOpaqueRelevance = true;
	Result.bNormalTranslucencyRelevance = false;
	Result.bDynamicRelevance = true;
	Result.bShadowRelevance = IsShadowCast(View);
	Result.bSeparateTranslucencyRelevance = true;
//	Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
	return Result;
}

uint32 FSsRenderPartsProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

void FSsRenderPartsProxy::SetDynamicData_RenderThread(
	const TArray<FSsPartVertex>& InRenderVertices,
	const TArray<uint32>& InRenderIndices,
	const TArray<FSsPartPrimitive>& InRenderPrimitives
	)
{
	RenderPrimitives = InRenderPrimitives;

	if(0 < InRenderVertices.Num())
	{
		check(InRenderVertices.Num() <= (int32)VertexBuffer.NumVerts);
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, InRenderVertices.Num() * sizeof(FSsPartVertex), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, InRenderVertices.GetData(), InRenderVertices.Num() * sizeof(FSsPartVertex));
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
	}
	if(0 < InRenderIndices.Num())
	{
		check(InRenderIndices.Num() <= (int32)IndexBuffer.NumIndices);
		void* IndexBufferData = RHILockIndexBuffer(IndexBuffer.IndexBufferRHI, 0, InRenderIndices.Num() * sizeof(uint32), RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, InRenderIndices.GetData(), InRenderIndices.Num() * sizeof(uint32));
		RHIUnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
	}
}
