#include "SsRenderPartsProxy.h"

#include "RHIStaticStates.h"
#include "DynamicMeshBuilder.h"

#include "SsPlayerComponent.h"


//
// IndexBuffer 
//
void FSsPartsIndexBuffer::InitRHI()
{
	if(0 < NumIndices)
	{
		FRHIResourceCreateInfo CreateInfo(TEXT("SsComponentPartsIndexBuffer"));
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
	Result.bDrawRelevance        = IsShown(View);
	Result.bRenderCustomDepth    = ShouldRenderCustomDepth();
	Result.bRenderInMainPass     = ShouldRenderInMainPass();
	Result.bUsesLightingChannels = false;
	Result.bOpaque               = true;
	Result.bSeparateTranslucency = true;
	Result.bNormalTranslucency   = false;
	Result.bShadowRelevance      = IsShadowCast(View);
	Result.bDynamicRelevance     = true;
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
		FDynamicMeshVertex DummyVert(FVector3f(InRenderVertices[0].Position), FVector3f::ForwardVector, FVector3f::UpVector, FVector2f(InRenderVertices[0].TexCoord), InRenderVertices[0].Color);
		TangentY = DummyVert.GetTangentY();
	}

	for(auto ItVert = InRenderVertices.CreateConstIterator(); ItVert; ++ItVert)
	{
		VertexBuffers.PositionVertexBuffer.VertexPosition(ItVert.GetIndex()) = ItVert->Position;
		VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(ItVert.GetIndex(), FVector::ForwardVector, TangentY, FVector::UpVector);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(ItVert.GetIndex(), 0, FVector2f(ItVert->TexCoord));
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(ItVert.GetIndex(), 1, FVector2f(ItVert->ColorBlend));
		VertexBuffers.ColorVertexBuffer.VertexColor(ItVert.GetIndex()) = ItVert->Color;
	}

	{
		auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
		void* VertexBufferData = RHILockBuffer(VertexBuffer.VertexBufferRHI, 0, InRenderVertices.Num() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), InRenderVertices.Num() * VertexBuffer.GetStride());
		RHIUnlockBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
		void* VertexBufferData = RHILockBuffer(VertexBuffer.VertexBufferRHI, 0, InRenderVertices.Num() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), InRenderVertices.Num() * VertexBuffer.GetStride());
		RHIUnlockBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHILockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
		RHIUnlockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHILockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
		RHIUnlockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
	}

	{
		void* IndexBufferData = RHILockBuffer(IndexBuffer.IndexBufferRHI, 0, InRenderIndices.Num() * sizeof(uint16), RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, InRenderIndices.GetData(), InRenderIndices.Num() * sizeof(uint16));
		RHIUnlockBuffer(IndexBuffer.IndexBufferRHI);
	}
}
