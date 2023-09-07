#include "SsOffScreenShaders.h"

#include "ShaderParameterUtils.h"


IMPLEMENT_SHADER_TYPE(, FSsOffScreenVS,       TEXT("/Plugin/SpriteStudio6/Private/Ss6OffScreenShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FSsOffScreenPS,       TEXT("/Plugin/SpriteStudio6/Private/Ss6OffScreenShader.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FSsOffScreenMaskedVS, TEXT("/Plugin/SpriteStudio6/Private/Ss6OffScreenShader.usf"), TEXT("MainMaskedVS"), SF_Vertex);
IMPLEMENT_SHADER_TYPE(, FSsOffScreenMaskedPS, TEXT("/Plugin/SpriteStudio6/Private/Ss6OffScreenShader.usf"), TEXT("MainMaskedPS"), SF_Pixel);

TGlobalResource<FSsOffScreenVertexDeclaration> GSs6OffScreenVertexDeclaration;

// 
void FSsOffScreenVertexDeclaration::InitRHI(FRHICommandListBase& /*RHICmdList*/)
{
	FVertexDeclarationElementList Elements;
	uint32 Stride = sizeof(FSsOffScreenVertex);
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSsOffScreenVertex, Position), VET_Float2, 0, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSsOffScreenVertex, Color), VET_Color, 1, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSsOffScreenVertex, TexCoord), VET_Float4, 2, Stride));
	Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSsOffScreenVertex, ColorBlend), VET_Float2, 3, Stride));
	
	VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
}
void FSsOffScreenVertexDeclaration::ReleaseRHI()
{
	VertexDeclarationRHI.SafeRelease();
}

//
FSsOffScreenPS::FSsOffScreenPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
	CellTextureParameter.Bind(Initializer.ParameterMap, TEXT("CellTexture"));
	CellTextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("CellTextureSampler"));
}
void FSsOffScreenPS::SetCellTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState )
{
	SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), CellTextureParameter, CellTextureParameterSampler, SamplerState, InTexture);
}

//
FSsOffScreenMaskedPS::FSsOffScreenMaskedPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
	: FGlobalShader(Initializer)
{
	CellTextureParameter.Bind(Initializer.ParameterMap, TEXT("CellTexture"));
	CellTextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("CellTextureSampler"));
	MaskTextureParameter.Bind(Initializer.ParameterMap, TEXT("MaskTexture"));
	MaskTextureParameterSampler.Bind(Initializer.ParameterMap, TEXT("MaskTextureSampler"));
}
void FSsOffScreenMaskedPS::SetCellTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState )
{
	SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), CellTextureParameter, CellTextureParameterSampler, SamplerState, InTexture );
}
void FSsOffScreenMaskedPS::SetMaskTexture(FRHICommandList& RHICmdList, FRHITexture* InTexture, const FSamplerStateRHIRef SamplerState )
{
	SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MaskTextureParameter, MaskTextureParameterSampler, SamplerState, InTexture );
}
