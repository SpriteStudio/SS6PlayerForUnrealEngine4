#include "SpriteStudio6PrivatePCH.h"
#include "SsRenderPlaneProxy.h"

#include "DynamicMeshBuilder.h"
#include "SsPlayerComponent.h"



// VertexBuffer
void FSsPlaneVertexBuffer::InitRHI()
{
	FRHIResourceCreateInfo CreateInfo;
	VertexBufferRHI = RHICreateVertexBuffer(NumVerts * sizeof(FDynamicMeshVertex), BUF_Dynamic, CreateInfo);
}

// IndexBuffer
void FSsPlaneIndexBuffer::InitRHI()
{
	FRHIResourceCreateInfo CreateInfo;
	IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint32), NumIndices * sizeof(uint32), BUF_Dynamic, CreateInfo);
}

// VertexFactory
void FSsPlaneVertexFactory::Init(const FSsPlaneVertexBuffer* VertexBuffer)
{
	if(IsInRenderingThread())
	{
		FLocalVertexFactory::FDataType NewData;
		NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Position, VET_Float3);
		NewData.TextureCoordinates.Add(
			FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex,TextureCoordinate), sizeof(FDynamicMeshVertex), VET_Float2)
			);
		NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
		NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
		NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
		SetData(NewData);
	}
	else
	{
		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			InitSsVertexFactory,
			FSsPlaneVertexFactory*, VertexFactory, this,
			const FSsPlaneVertexBuffer*, VertexBuffer, VertexBuffer,
		{
			// Initialize the vertex factory's stream components.
			FDataType NewData;
			NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,Position,VET_Float3);
			NewData.TextureCoordinates.Add(
					FVertexStreamComponent(VertexBuffer, STRUCT_OFFSET(FDynamicMeshVertex,TextureCoordinate), sizeof(FDynamicMeshVertex), VET_Float2)
					);
			NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentX, VET_PackedNormal);
			NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, TangentZ, VET_PackedNormal);
			NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
			VertexFactory->SetData(NewData);
		});
	}
}



// コンストラクタ
FSsRenderPlaneProxy::FSsRenderPlaneProxy(USsPlayerComponent* InComponent, UMaterialInterface* InMaterial)
	: FPrimitiveSceneProxy(InComponent)
	, CanvasSizeUU(100.f, 100.f)
	, Pivot(0.f, 0.f)
{
	// FPrimitiveSceneProxy
	bWillEverBeLit = false;

	Component = InComponent;
	Material = InMaterial;

	VertexBuffer.NumVerts  = 4;
	IndexBuffer.NumIndices = 6;
	VertexFactory.Init(&VertexBuffer);

	BeginInitResource(&VertexBuffer);
	BeginInitResource(&IndexBuffer);
	BeginInitResource(&VertexFactory);

	bVerifyUsedMaterials = false;
}

// デストラクタ
FSsRenderPlaneProxy::~FSsRenderPlaneProxy()
{
	VertexBuffer.ReleaseResource();
	IndexBuffer.ReleaseResource();
	VertexFactory.ReleaseResource();
}

void FSsRenderPlaneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const
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
	else
	{
		if(NULL == Material)
		{
			return;
		}
		MaterialProxy = Material->GetRenderProxy(IsSelected());
	}

	for(int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
	{
		if(VisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = Views[ViewIndex];

			// Draw the mesh.
			FMeshBatch& Mesh = Collector.AllocateMesh();
			FMeshBatchElement& BatchElement = Mesh.Elements[0];
			BatchElement.IndexBuffer = &IndexBuffer;
			Mesh.bDisableBackfaceCulling = true;
			Mesh.bWireframe = bWireframe;
			Mesh.VertexFactory = &VertexFactory;
			Mesh.MaterialRenderProxy = MaterialProxy;
			BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
			BatchElement.FirstIndex = 0;
			BatchElement.NumPrimitives = IndexBuffer.NumIndices / 3;
			BatchElement.MinVertexIndex = 0;
			BatchElement.MaxVertexIndex = VertexBuffer.NumVerts - 1;
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

FPrimitiveViewRelevance FSsRenderPlaneProxy::GetViewRelevance(const FSceneView* View) const
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

uint32 FSsRenderPlaneProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

void FSsRenderPlaneProxy::SetDynamicData_RenderThread()
{
	const int32 NumVerts = 4;
	FDynamicMeshVertex Vertices[NumVerts];
	FVector2D PivotOffSet = -(Pivot * CanvasSizeUU);
	Vertices[0] = FDynamicMeshVertex(FVector(0.f, PivotOffSet.X - CanvasSizeUU.X/2.f, PivotOffSet.Y + CanvasSizeUU.Y/2.f), FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector2D(0.f, 0.f), FColor(255, 255, 255, 255));
	Vertices[1] = FDynamicMeshVertex(FVector(0.f, PivotOffSet.X + CanvasSizeUU.X/2.f, PivotOffSet.Y + CanvasSizeUU.Y/2.f), FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector2D(1.f, 0.f), FColor(255, 255, 255, 255));
	Vertices[2] = FDynamicMeshVertex(FVector(0.f, PivotOffSet.X - CanvasSizeUU.X/2.f, PivotOffSet.Y - CanvasSizeUU.Y/2.f), FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector2D(0.f, 1.f), FColor(255, 255, 255, 255));
	Vertices[3] = FDynamicMeshVertex(FVector(0.f, PivotOffSet.X + CanvasSizeUU.X/2.f, PivotOffSet.Y - CanvasSizeUU.Y/2.f), FVector(1.f, 0.f, 0.f), FVector(0.f, 0.f, 1.f), FVector2D(1.f, 1.f), FColor(255, 255, 255, 255));
	void* VertexBufferData = RHILockVertexBuffer(VertexBuffer.VertexBufferRHI, 0, NumVerts * sizeof(FDynamicMeshVertex), RLM_WriteOnly);
	FMemory::Memcpy(VertexBufferData, &Vertices[0], NumVerts * sizeof(FDynamicMeshVertex));
	RHIUnlockVertexBuffer(VertexBuffer.VertexBufferRHI);
	

	const int32 NumIndices = 6;
	uint32 Indices[NumIndices] = {0, 2, 1, 1, 2, 3};
	void* IndexBufferData = RHILockIndexBuffer(IndexBuffer.IndexBufferRHI, 0, NumIndices * sizeof(int32), RLM_WriteOnly);
	FMemory::Memcpy(IndexBufferData, &Indices[0], NumIndices * sizeof(int32));
	RHIUnlockIndexBuffer(IndexBuffer.IndexBufferRHI);
}
