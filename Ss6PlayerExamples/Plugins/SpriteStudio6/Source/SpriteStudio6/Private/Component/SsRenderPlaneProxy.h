#pragma once

#include "RHIDefinitions.h"
#include "PrimitiveSceneProxy.h"
#include "StaticMeshResources.h"


// IndexBuffer
class FSsPlaneIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;
};

// RenderProxy
class FSsRenderPlaneProxy : public FPrimitiveSceneProxy
{
public:
	FSsRenderPlaneProxy(class USsPlayerComponent* InComponent, UMaterialInterface* InMaterial);
	virtual ~FSsRenderPlaneProxy();

	// FPrimitiveSceneProxy interface
	virtual SIZE_T GetTypeHash() const override;
	virtual void CreateRenderThreadResources() override;
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual uint32 GetMemoryFootprint() const override;

	void SetDynamicData_RenderThread(FRHICommandListImmediate& RHICmdList);
	void SetMaterial(UMaterialInterface* InMaterial) { Material = InMaterial; }
	void SetPivot(const FVector2f& InPivot) { Pivot = InPivot; }

public:
	FVector2f CanvasSizeUU;

private:
	USsPlayerComponent* Component;

	UPROPERTY()
	UMaterialInterface* Material;

	FVector2f Pivot;

	FStaticMeshVertexBuffers VertexBuffers;
	FSsPlaneIndexBuffer IndexBuffer;
	FLocalVertexFactory VertexFactory;
};
