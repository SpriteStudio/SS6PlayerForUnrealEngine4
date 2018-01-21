#pragma once

#include "PrimitiveSceneProxy.h"


// VertexBuffer
class FSsPartsVertexBuffer : public FVertexBuffer
{
public:
	virtual void InitRHI() override;
	uint32 NumVerts;
};

// IndexBuffer
class FSsPartsIndexBuffer : public FIndexBuffer
{
public:
	virtual void InitRHI() override;
	uint32 NumIndices;
};

// VertexFactory
class FSsPartsVertexFactory : public FLocalVertexFactory
{
public:
	DECLARE_VERTEX_FACTORY_TYPE(FSsPartsVertexFactory);

	static bool ShouldCache(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType);
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment);
	static FVertexFactoryShaderParameters* ConstructShaderParameters(EShaderFrequency ShaderFrequency);

	void Init(const FSsPartsVertexBuffer* VertexBuffer);
};

// VertexFactoryShaderParameters
class FSsPartVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	virtual void Bind(const class FShaderParameterMap& ParameterMap) override {}
	virtual void Serialize(FArchive& Ar) override {}
	virtual void SetMesh(FRHICommandList& RHICmdList, FShader* Shader,const class FVertexFactory* VertexFactory,const class FSceneView& View,const struct FMeshBatchElement& BatchElement,uint32 DataFlags) const override;
	virtual uint32 GetSize() const override { return sizeof(*this); }
};


// RenderProxy
class FSsRenderPartsProxy : public FPrimitiveSceneProxy
{
public:
	FSsRenderPartsProxy(class USsPlayerComponent* InComponent, uint32 InMaxVertexNum, uint32 InMaxIndexNum);
	virtual ~FSsRenderPartsProxy();

	// FPrimitiveSceneProxy interface
	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override;
	virtual uint32 GetMemoryFootprint() const override;


	void SetDynamicData_RenderThread(const TArray<FSsRenderPartWithMaterial>& InRenderParts);
	void SetPivot(const FVector2D& InPivot) { Pivot = InPivot; }

public:
	FVector2D CanvasSizeUU;

private:
	USsPlayerComponent* Component;

	TArray<FSsRenderPartWithMaterial> RenderParts;
	FVector2D Pivot;

	FSsPartsVertexBuffer  VertexBuffer;
	FSsPartsIndexBuffer   IndexBuffer;
	FSsPartsVertexFactory VertexFactory;
};
