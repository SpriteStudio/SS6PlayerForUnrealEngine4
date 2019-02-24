#pragma once

#include "RHIDefinitions.h"
#include "PrimitiveSceneProxy.h"
#include "StaticMeshResources.h"
#include "SsTypes.h"


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
	DECLARE_VERTEX_FACTORY_TYPE(FSsPartsVertexFactory);

public:
	FSsPartsVertexFactory(ERHIFeatureLevel::Type InFeatureLevel, const char* InDebugName)
		: FLocalVertexFactory(InFeatureLevel, InDebugName)
	{}

	static bool ShouldCompilePermutation(EShaderPlatform Platform, const class FMaterial* Material, const class FShaderType* ShaderType)
	{
		return FLocalVertexFactory::ShouldCompilePermutation(Platform, Material, ShaderType);
	}
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment)
	{
		FLocalVertexFactory::ModifyCompilationEnvironment(Platform, Material, OutEnvironment);
	}
	static bool SupportsTessellationShaders()
	{
		return FLocalVertexFactory::SupportsTessellationShaders();
	}
	static FVertexFactoryShaderParameters* ConstructShaderParameters(EShaderFrequency ShaderFrequency);
};

// VertexFactoryShaderParameters
class FSsPartVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{
public:
	virtual void Bind(const class FShaderParameterMap& ParameterMap) override {}
	virtual void Serialize(FArchive& Ar) override {}
	virtual void SetMesh(FRHICommandList& RHICmdList, FShader* Shader,const class FVertexFactory* VertexFactory,const class FSceneView& View,const struct FMeshBatchElement& BatchElement,uint32 DataFlags) const override {}
	virtual uint32 GetSize() const override { return sizeof(*this); }
};

// RenderProxy
class FSsRenderPartsProxy : public FPrimitiveSceneProxy
{
public:
	struct FSsPartVertex
	{
		FVector Position;
		FVector2D TexCoord;
		FColor Color;
		FVector2D ColorBlend;	// [1]:ColorBlendRate 
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
	FSsPartsVertexFactory VertexFactory;
};
