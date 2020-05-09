#include "SsMaskShaders.h"

#include "ShaderParameterUtils.h"


IMPLEMENT_SHADER_TYPE(, FSsMaskVS, TEXT("/Plugin/SpriteStudio6/Private/Ss6MaskShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FSsMaskPS, TEXT("/Plugin/SpriteStudio6/Private/Ss6MaskShader.usf"), TEXT("MainPS"), SF_Pixel);


//
FSsMaskPS::FSsMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
	CellTextureParameter.Bind(Initializer.ParameterMap, TEXT("CellTexture"));
	CellTextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("CellTextureSampler"));
}
void FSsMaskPS::SetCellTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState )
{
	SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), CellTextureParameter, CellTextureParameterSampler, SamplerState, InTexture );
}
