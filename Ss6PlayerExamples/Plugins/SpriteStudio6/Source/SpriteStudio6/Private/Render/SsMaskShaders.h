#pragma once

#include "GlobalShader.h"
#include "UniformBuffer.h"
#include "RHICommandList.h"


// 
class FSsMaskVS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSsMaskVS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters){ return true; }

	FSsMaskVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{}
	FSsMaskVS() {}
};

// 
class FSsMaskPS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FSsMaskPS, Global);

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters){ return true; }

	FSsMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
	FSsMaskPS() {}

	void SetCellTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState);

private:
	LAYOUT_FIELD(FShaderResourceParameter, CellTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, CellTextureParameterSampler);
};
