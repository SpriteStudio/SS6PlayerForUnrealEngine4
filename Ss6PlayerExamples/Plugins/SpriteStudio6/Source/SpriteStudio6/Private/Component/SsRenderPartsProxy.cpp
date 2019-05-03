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
// IndexBuffer 
//
void FSsPartsIndexBuffer::InitRHI()
{
	if(0 < NumIndices)
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), NumIndices * sizeof(uint16), BUF_Dynamic, CreateInfo);
	}
}


// コンストラクタ
FSsRenderPartsProxy::FSsRenderPartsProxy(USsPlayerComponent* InComponent, uint32 InMaxVertexNum, uint32 InMaxIndexNum)
	: FPrimitiveSceneProxy(InComponent)
	, MaxVertexNum(InMaxVertexNum)
	, MaxIndexNum(InMaxIndexNum)
	, VertexFactory(GetScene().GetFeatureLevel(), "FSsRenderPartsProxy")
{
	// FPrimitiveSceneProxy
	bWillEverBeLit = false;

	Component = InComponent;
	bVerifyUsedMaterials = false;
}

// デストラクタ
FSsRenderPartsProxy::~FSsRenderPartsProxy()
{
	VertexBuffers.PositionVertexBuffer.ReleaseResource();
	VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	VertexBuffers.ColorVertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
}

SIZE_T FSsRenderPartsProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

void FSsRenderPartsProxy::CreateRenderThreadResources()
{
	VertexBuffers.InitWithDummyData(&VertexFactory, MaxVertexNum, 2);
	IndexBuffer.NumIndices = MaxIndexNum;
	IndexBuffer.InitResource();
}

void FSsRenderPartsProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
{
	QUICK_SCOPE_CYCLE_COUNTER( STAT_SsRenderSceneProxy_GetDynamicMeshElements );

	const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;
	FMaterialRenderProxy* MaterialProxy = NULL;
	if(bWireframe)
	{
		FColoredMaterialRenderProxy* WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
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
					MaterialProxy = ItPrim->Material->GetRenderProxy();
				}

				// Draw the mesh. 
				FMeshBatch& Mesh = Collector.AllocateMesh();
				Mesh.VertexFactory              = &VertexFactory;
				Mesh.MaterialRenderProxy        = MaterialProxy;
				Mesh.ReverseCulling             = IsLocalToWorldDeterminantNegative();
				Mesh.CastShadow                 = true;
				Mesh.DepthPriorityGroup         = SDPG_World;
				Mesh.Type                       = PT_TriangleList;
				Mesh.bDisableBackfaceCulling    = true;
				Mesh.bCanApplyViewModeOverrides = false;

				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer    = &IndexBuffer;
				BatchElement.FirstIndex     = ItPrim->FirstIndex;
				BatchElement.MinVertexIndex = ItPrim->MinVertexIndex;
				BatchElement.MaxVertexIndex = ItPrim->MaxVertexIndex;
				BatchElement.NumPrimitives  = ItPrim->NumPrimitives;

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
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance                 = IsShown(View);
	Result.bRenderCustomDepth             = ShouldRenderCustomDepth();
	Result.bRenderInMainPass              = ShouldRenderInMainPass();
	Result.bUsesLightingChannels          = false;
	Result.bOpaqueRelevance               = true;
	Result.bSeparateTranslucencyRelevance = true;
	Result.bNormalTranslucencyRelevance   = false;
	Result.bShadowRelevance               = IsShadowCast(View);
	Result.bDynamicRelevance              = true;
	return Result;
}

uint32 FSsRenderPartsProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

void FSsRenderPartsProxy::SetDynamicData_RenderThread(
	const TArray<FSsPartVertex>& InRenderVertices,
	const TArray<uint16>& InRenderIndices,
	const TArray<FSsPartPrimitive>& InRenderPrimitives
	)
{
	RenderPrimitives = InRenderPrimitives;

	if((InRenderVertices.Num() <= 0) || (InRenderIndices.Num() <= 0))
	{
		return;
	}
	check((uint32)InRenderVertices.Num() <= MaxVertexNum);
	check((uint32)InRenderIndices.Num() <= MaxIndexNum);

	
	FVector TangentY;
	{
		FDynamicMeshVertex DummyVert(InRenderVertices[0].Position, FVector::ForwardVector, FVector::UpVector, InRenderVertices[0].TexCoord, InRenderVertices[0].Color);
		TangentY = DummyVert.GetTangentY();
	}

	for(auto ItVert = InRenderVertices.CreateConstIterator(); ItVert; ++ItVert)
	{
		VertexBuffers.PositionVertexBuffer.VertexPosition(ItVert.GetIndex()) = ItVert->Position;
		VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(ItVert.GetIndex(), FVector::ForwardVector, TangentY, FVector::UpVector);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(ItVert.GetIndex(), 0, ItVert->TexCoord);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(ItVert.GetIndex(), 1, ItVert->ColorBlend);
		VertexBuffers.ColorVertexBuffer.VertexColor(ItVert.GetIndex()) = ItVert->Color;
	}

	{
		auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, InRenderVertices.Num() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), InRenderVertices.Num() * VertexBuffer.GetStride());
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, InRenderVertices.Num() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), InRenderVertices.Num() * VertexBuffer.GetStride());
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
		RHIUnlockVertexBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
		RHIUnlockVertexBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
	}

	{
		void* IndexBufferData = RHILockIndexBuffer(IndexBuffer.IndexBufferRHI, 0, InRenderIndices.Num() * sizeof(uint16), RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, InRenderIndices.GetData(), InRenderIndices.Num() * sizeof(uint16));
		RHIUnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
	}
}
