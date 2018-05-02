#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"



// 
struct FSsOffScreenVertex
{
	FVector2D Position;
	FColor Color;
	FVector2D TexCoord;
	FVector2D ColorBlend;	// [0]:ColorBlendType, [1]:ColorBlendRate 
};

// 
class FSsOffScreenVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;
};


// 
class FSsOffScreenVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSsOffScreenVS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters){ return true; }

	FSsOffScreenVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{}
	FSsOffScreenVS() {}
};

// 
class FSsOffScreenPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSsOffScreenPS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters){ return true; }

	FSsOffScreenPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
	FSsOffScreenPS() {}

	virtual bool Serialize(FArchive& Ar) override;

	void SetCellTexture(FRHICommandList& RHICmdList, const FTextureRHIParamRef InTexture, const FSamplerStateRHIRef SamplerState );

private:
	FShaderResourceParameter CellTextureParameter;
	FShaderResourceParameter CellTextureParameterSampler;
};

// 
class FSsOffScreenMaskedPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSsOffScreenMaskedPS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters){ return true; }

	FSsOffScreenMaskedPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
	FSsOffScreenMaskedPS() {}

	virtual bool Serialize(FArchive& Ar) override;

	void SetCellTexture(FRHICommandList& RHICmdList, const FTextureRHIParamRef InTexture, const FSamplerStateRHIRef SamplerState);
	void SetMaskTexture(FRHICommandList& RHICmdList, const FTextureRHIParamRef InTexture, const FSamplerStateRHIRef SamplerState);

private:
	FShaderResourceParameter CellTextureParameter;
	FShaderResourceParameter CellTextureParameterSampler;
	FShaderResourceParameter MaskTextureParameter;
	FShaderResourceParameter MaskTextureParameterSampler;
};

extern TGlobalResource<FSsOffScreenVertexDeclaration> GSs6OffScreenVertexDeclaration;
