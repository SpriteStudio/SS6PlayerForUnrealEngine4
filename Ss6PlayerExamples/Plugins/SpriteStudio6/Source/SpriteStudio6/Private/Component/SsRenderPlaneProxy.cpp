#include "SsRenderPlaneProxy.h"

#include "DynamicMeshBuilder.h"
#include "SsPlayerComponent.h"



// IndexBuffer
void FSsPlaneIndexBuffer::InitRHI()
{
	FRHIResourceCreateInfo CreateInfo(TEXT("SsComponentPlaneIndexBuffer"));
	void* Buffer = nullptr;
	IndexBufferRHI = RHICreateAndLockIndexBuffer(sizeof(uint16), 6 * sizeof(uint16), BUF_Static, CreateInfo, Buffer);
	((uint16*)Buffer)[0] = 0;
	((uint16*)Buffer)[1] = 2;
	((uint16*)Buffer)[2] = 1;
	((uint16*)Buffer)[3] = 1;
	((uint16*)Buffer)[4] = 2;
	((uint16*)Buffer)[5] = 3;
	RHIUnlockIndexBuffer(IndexBufferRHI);
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
	IndexBuffer.InitResource();
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

uint32 FSsRenderPlaneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

void FSsRenderPlaneProxy::SetDynamicData_RenderThread()
{
	TArray<FDynamicMeshVertex> Vertices;
	Vertices.Empty(4);
	Vertices.AddUninitialized(4);
	FVector2D PivotOffSet = -(Pivot * CanvasSizeUU);
	Vertices[0] = FDynamicMeshVertex(FVector(0.f, PivotOffSet.X - CanvasSizeUU.X/2.f, PivotOffSet.Y + CanvasSizeUU.Y/2.f), FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector2D(0.f, 0.f), FColor(255, 255, 255, 255));
	Vertices[1] = FDynamicMeshVertex(FVector(0.f, PivotOffSet.X + CanvasSizeUU.X/2.f, PivotOffSet.Y + CanvasSizeUU.Y/2.f), FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector2D(1.f, 0.f), FColor(255, 255, 255, 255));
	Vertices[2] = FDynamicMeshVertex(FVector(0.f, PivotOffSet.X - CanvasSizeUU.X/2.f, PivotOffSet.Y - CanvasSizeUU.Y/2.f), FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector2D(0.f, 1.f), FColor(255, 255, 255, 255));
	Vertices[3] = FDynamicMeshVertex(FVector(0.f, PivotOffSet.X + CanvasSizeUU.X/2.f, PivotOffSet.Y - CanvasSizeUU.Y/2.f), FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector2D(1.f, 1.f), FColor(255, 255, 255, 255));
	
	for(int32 i = 0; i < 4; ++i)
	{
		VertexBuffers.PositionVertexBuffer.VertexPosition(i) = Vertices[i].Position;
		VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(i, Vertices[i].TangentX.ToFVector(), Vertices[i].GetTangentY(), Vertices[i].TangentZ.ToFVector());
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(i, 0, Vertices[i].TextureCoordinate[0]);
		VertexBuffers.ColorVertexBuffer.VertexColor(i) = Vertices[i].Color;
	}

	{
		auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
		RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
		void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, VertexBuffer.GetNumVertices() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), VertexBuffer.GetNumVertices() * VertexBuffer.GetStride());
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
}
