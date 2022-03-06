#pragma once

#include "RHIDefinitions.h"
#include "PrimitiveSceneProxy.h"
#include "StaticMeshResources.h"
#include "LocalVertexFactory.h"

#include "SsTypes.h"


// IndexBuffer
class FSsPartsIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI() override;
	uint32 NumIndices;
};
// RenderProxy
class FSsRenderPartsProxy : public FPrimitiveSceneProxy
{
public:
	struct FSsPartVertex
	{
		FVector3f Position;
		FVector2f TexCoord;
		FColor Color;
		FVector2f ColorBlend;	// [0]:PixelDepthOffset, [1]:ColorBlendRate 
	};
	struct FSsPartPrimitive
	{
		UMaterialInterface* Material;
		SsBlendType::Type AlphaBlendType;
		uint32 FirstIndex;
		uint32 NumPrimitives;
		uint32 MinVertexIndex;
		uint32 MaxVertexIndex;
	};

public:
	FSsRenderPartsProxy(class USsPlayerComponent* InComponent, uint32 InMaxVertexNum, uint32 InMaxIndexNum);
	virtual ~FSsRenderPartsProxy();

	// FPrimitiveSceneProxy interface
	virtual SIZE_T GetTypeHash() const override;
	virtual void CreateRenderThreadResources() override;
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual uint32 GetMemoryFootprint() const override;

	void SetDynamicData_RenderThread(const TArray<FSsPartVertex>& InRenderVertices, const TArray<uint16>& InRenderIndices, const TArray<FSsPartPrimitive>& InRenderPrimitives);

private:
	USsPlayerComponent* Component;

	TArray<FSsPartPrimitive> RenderPrimitives;

	uint32 MaxVertexNum;
	uint32 MaxIndexNum;

	FStaticMeshVertexBuffers VertexBuffers;
	FSsPartsIndexBuffer IndexBuffer;
	FLocalVertexFactory VertexFactory;
};
