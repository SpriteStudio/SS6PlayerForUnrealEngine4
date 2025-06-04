#include "SsRenderPartsProxy.h"

#include "RHIStaticStates.h"
#include "DynamicMeshBuilder.h"
#include "Materials/MaterialRenderProxy.h"

#include "SsPlayerComponent.h"


DEFINE_RENDER_COMMAND_PIPE(SsParts, ERenderCommandPipeFlags::None);


//
// IndexBuffer 
//
void FSsPartsIndexBuffer::InitRHI(FRHICommandListBase& RHICmdList)
{
	if(0 < NumIndices)
	{
		FRHIBufferCreateDesc CreateDesc =
			FRHIBufferCreateDesc::CreateIndex<uint16>(TEXT("SsComponentPartsIndexBuffer"), NumIndices)
			.AddUsage(EBufferUsageFlags::Dynamic | EBufferUsageFlags::ShaderResource)
			.DetermineInitialState();
		IndexBufferRHI = RHICmdList.CreateBuffer(CreateDesc);
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

	VertexBuffers.InitWithDummyData(&UE::RenderCommandPipe::SsParts, &VertexFactory, MaxVertexNum, 2U);
	IndexBuffer.NumIndices = MaxIndexNum;

	ENQUEUE_RENDER_COMMAND(InitSsPartsResources)(UE::RenderCommandPipe::SsParts,
		[this] (FRHICommandList& RHICmdList)
		{
			IndexBuffer.InitResource(RHICmdList);
		});
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

uint32 FSsRenderPartsProxy::GetMemoryFootprint() const
{
	return sizeof(*this) + GetAllocatedSize();
}

void FSsRenderPartsProxy::SetDynamicData_RenderThread(
	FRHICommandListImmediate& RHICmdList,
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

	for(auto ItVert = InRenderVertices.CreateConstIterator(); ItVert; ++ItVert)
	{
		VertexBuffers.PositionVertexBuffer.VertexPosition(ItVert.GetIndex()) = ItVert->Position;
		VertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(ItVert.GetIndex(), FVector3f::RightVector, FVector3f::DownVector, FVector3f::BackwardVector);
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(ItVert.GetIndex(), 0, FVector2f(ItVert->TexCoord));
		VertexBuffers.StaticMeshVertexBuffer.SetVertexUV(ItVert.GetIndex(), 1, FVector2f(ItVert->ColorBlend));
		VertexBuffers.ColorVertexBuffer.VertexColor(ItVert.GetIndex()) = ItVert->Color;
	}

	{
		auto& VertexBuffer = VertexBuffers.PositionVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, InRenderVertices.Num() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), InRenderVertices.Num() * VertexBuffer.GetStride());
		RHICmdList.UnlockBuffer(VertexBuffer.VertexBufferRHI);
	}

	{
		auto& VertexBuffer = VertexBuffers.ColorVertexBuffer;
		void* VertexBufferData = RHICmdList.LockBuffer(VertexBuffer.VertexBufferRHI, 0, InRenderVertices.Num() * VertexBuffer.GetStride(), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, VertexBuffer.GetVertexData(), InRenderVertices.Num() * VertexBuffer.GetStride());
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

	{
		void* IndexBufferData = RHICmdList.LockBuffer(IndexBuffer.IndexBufferRHI, 0, InRenderIndices.Num() * sizeof(uint16), RLM_WriteOnly);
		FMemory::Memcpy(IndexBufferData, InRenderIndices.GetData(), InRenderIndices.Num() * sizeof(uint16));
		RHICmdList.UnlockBuffer(IndexBuffer.IndexBufferRHI);
	}

#if WITH_EDITOR
	{
		TArray<UMaterialInterface*> TempUsedMaterialsForVerification;
		Component->GetUsedMaterials(TempUsedMaterialsForVerification);
		SetUsedMaterialForVerification(TempUsedMaterialsForVerification);
	}
#endif
}
