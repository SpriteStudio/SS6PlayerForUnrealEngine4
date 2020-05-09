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

	void SetCellTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState );

private:
	LAYOUT_FIELD(FShaderResourceParameter, CellTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, CellTextureParameterSampler);
};
 

// 
class FSsOffScreenMaskedVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSsOffScreenMaskedVS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters){ return true; }

	FSsOffScreenMaskedVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{}
	FSsOffScreenMaskedVS() {}
};

// 
class FSsOffScreenMaskedPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSsOffScreenMaskedPS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters){ return true; }

	FSsOffScreenMaskedPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
	FSsOffScreenMaskedPS() {}

	void SetCellTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState);
	void SetMaskTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState);

private:
	LAYOUT_FIELD(FShaderResourceParameter, CellTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, CellTextureParameterSampler);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureParameterSampler);
};

extern TGlobalResource<FSsOffScreenVertexDeclaration> GSs6OffScreenVertexDeclaration;
