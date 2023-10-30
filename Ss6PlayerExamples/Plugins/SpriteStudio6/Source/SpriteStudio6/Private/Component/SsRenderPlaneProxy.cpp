#include "SsRenderPlaneProxy.h"

#include "DynamicMeshBuilder.h"
#include "SsPlayerComponent.h"



// IndexBuffer
void FSsPlaneIndexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	FRHIResourceCreateInfo CreateInfo(TEXT("SsComponentPlaneIndexBuffer"));
	uint32 Size = 6 * sizeof(uint16);
	IndexBufferRHI = RHICmdList.CreateIndexBuffer(sizeof(uint16), Size, BUF_Static, CreateInfo);
	void* Buffer = RHICmdList.LockBuffer(IndexBufferRHI, 0, Size, RLM_WriteOnly);
	((uint16*)Buffer)[0] = 0;
	((uint16*)Buffer)[1] = 2;
	((uint16*)Buffer)[2] = 1;
	((uint16*)Buffer)[3] = 1;
	((uint16*)Buffer)[4] = 2;
	((uint16*)Buffer)[5] = 3;
	RHICmdList.UnlockBuffer(IndexBufferRHI);
}

// コンストラクタ
FSsRenderPlaneProxy::FSsRenderPlaneProxy(USsPlayerComponent* InComponent, UMaterialInterface* InMaterial)
	: FPrimitiveSceneProxy(InComponent)
	, CanvasSizeUU(100.f, 100.f)
	, Pivot(0.f, 0.f)
	, VertexFactory(GetScene().GetFeatureLevel(), "FSsRenderPlaneProxy")
{
	// FPrimitiveSceneProxy
	bWillEverBeLit = false;

	Component = InComponent;
	Material = InMaterial;

	bVerifyUsedMaterials = false;
}

// デストラクタ
FSsRenderPlaneProxy::~FSsRenderPlaneProxy()
{
	VertexBuffers.PositionVertexBuffer.ReleaseResource();
	VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
	VertexBuffers.ColorVertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
}

SIZE_T FSsRenderPlaneProxy::GetTypeHash() const
{
	static size_t UniquePointer;
	return reinterpret_cast<size_t>(&UniquePointer);
}

void FSsRenderPlaneProxy::CreateRenderThreadResources()
{
	VertexBuffers.InitWithDummyData(&VertexFactory, 4);
	IndexBuffer.InitResource(FRHICommandListImmediate::Get());
}

void FSsRenderPlaneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
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
	else
	{
		if(NULL == Material)
		{
			return;
		}
		MaterialProxy = Material->GetRenderProxy();
	}

	for(int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if(VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];

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
			BatchElement.FirstIndex     = 0;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = 3;
			BatchElement.NumPrimitives  = 2;

			Collector.AddMesh(ViewIndex, Mesh);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			// Render bounds
			RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
#endif
		}
	}
}

FPrimitiveViewRelevance FSsRenderPlaneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance Result;
	Result.bDrawRelevance     = IsShown(View);
	Result.bRenderCustomDepth = ShouldRenderCustomDepth();
	Result.bRenderInMainPass  = ShouldRenderInMainPass();
	Result.bRenderInDepthPass = ShouldRenderInDepthPass();
	Result.bShadowRelevance   = IsShadowCast(View);
	Result.bDynamicRelevance  = true;
	Result.bVelocityRelevance = DrawsVelocity() && Result.bOpaque && Result.bRenderInMainPass;

	FMaterialRelevance MaterialRelevance;
	for(auto ItMid = Component->RenderMIDs.CreateConstIterator(); ItMid; ++ItMid)
	{
		if(nullptr != (*ItMid))
		{
			MaterialRelevance |= (*ItMid)->GetRelevance_Concurrent(GetScene().GetFeatureLevel());
		}
	}
	MaterialRelevance.SetPrimitiveViewRelevance(Result);
	
	return Result;
}

uint32 FSsRenderPlaneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

void FSsRenderPlaneProxy::SetDynamicData_RenderThread(FRHICommandListImmediate& RHICmdList)
{
	TArray<FDynamicMeshVertex> Vertices;
	Vertices.Empty(4);
	Vertices.AddUninitialized(4);
	FVector2f PivotOffSet = -(Pivot * CanvasSizeUU);
	Vertices[0] = FDynamicMeshVertex(FVector3f(0.f, PivotOffSet.X - CanvasSizeUU.X/2.f, PivotOffSet.Y + CanvasSizeUU.Y/2.f), FVector3f(1.f, 0.f, 0.f), FVector3f(0.f, 0.f, 1.f), FVector2f(0.f, 0.f), FColor(255, 255, 255, 255));
	Vertices[1] = FDynamicMeshVertex(FVector3f(0.f, PivotOffSet.X + CanvasSizeUU.X/2.f, PivotOffSet.Y + CanvasSizeUU.Y/2.f), FVector3f(1.f, 0.f, 0.f), FVector3f(0.f, 0.f, 1.f), FVector2f(1.f, 0.f), FColor(255, 255, 255, 255));
	Vertices[2] = FDynamicMeshVertex(FVector3f(0.f, PivotOffSet.X - CanvasSizeUU.X/2.f, PivotOffSet.Y - CanvasSizeUU.Y/2.f), FVector3f(1.f, 0.f, 0.f), FVector3f(0.f, 0.f, 1.f), FVector2f(0.f, 1.f), FColor(255, 255, 255, 255));
	Vertices[3] = FDynamicMeshVertex(FVector3f(0.f, PivotOffSet.X + CanvasSizeUU.X/2.f, PivotOffSet.Y - CanvasSizeUU.Y/2.f), FVector3f(1.f, 0.f, 0.f), FVector3f(0.f, 0.f, 1.f), FVector2f(1.f, 1.f), FColor(255, 255, 255, 255));
	
	for(int32 i = 0; i < 4; ++i)
	{
		VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertices[i].Position;
		VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertices[i].TangentX.ToFVector3f(), Vertices[i].GetTangentY(), Vertices[i].TangentZ.ToFVector3f());
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertices[i].TextureCoordinate[0]);
		VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertices[i].Color;
	}

	{
		auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTangentSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTangentData(), VertexBuffer.GetTangentSize());
		RHICmdList.UnlockBuffer(VertexBuffer.TangentsVertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.StaticMeshVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetTexCoordSize(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetTexCoordData(), VertexBuffer.GetTexCoordSize());
		RHICmdList.UnlockBuffer(VertexBuffer.TexCoordVertexBuffer.VertexBufferRHI);
	}
}
