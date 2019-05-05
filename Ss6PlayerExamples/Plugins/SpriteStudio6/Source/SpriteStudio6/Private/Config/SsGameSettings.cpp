#include "SpriteStudio6PrivatePCH.h"
#include "SsGameSettings.h"


USsGameSettings::USsGameSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 各種マテリアル参照の取得
	// 参照：https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Reference/Classes/index.html#assetreferences
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_MixInv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_MixMix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_MixMul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_MixAdd;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_MixSub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_MixEff;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_MixInv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_MixMix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_MixMul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_MixAdd;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_MixSub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_MixEff;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompOffScreen;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_MixInv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_MixMix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_MixMul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_MixAdd;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_MixSub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_MixEff;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGMasked_MixInv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGMasked_MixMix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGMasked_MixMul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGMasked_MixAdd;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGMasked_MixSub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGMasked_MixEff;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGOffScreen;

		FConstructorStatics()
			: CompDefault_MixInv(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_MixInv"))
			, CompDefault_MixMix(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_MixMix"))
			, CompDefault_MixMul(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_MixMul"))
			, CompDefault_MixAdd(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_MixAdd"))
			, CompDefault_MixSub(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_MixSub"))
			, CompDefault_MixEff(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_MixEff"))
			, CompMasked_MixInv (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_MixInv"))
			, CompMasked_MixMix (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_MixMix"))
			, CompMasked_MixMul (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_MixMul"))
			, CompMasked_MixAdd (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_MixAdd"))
			, CompMasked_MixSub (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_MixSub"))
			, CompMasked_MixEff (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_MixEff"))
			, CompOffScreen     (TEXT("/SpriteStudio6/Component_OffScreen/M_Ss_Component_OffScreen"))
			, UMGDefault_MixInv (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_MixInv"))
			, UMGDefault_MixMix (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_MixMix"))
			, UMGDefault_MixMul (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_MixMul"))
			, UMGDefault_MixAdd (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_MixAdd"))
			, UMGDefault_MixSub (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_MixSub"))
			, UMGDefault_MixEff (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_MixEff"))
			, UMGMasked_MixInv  (TEXT("/SpriteStudio6/UMG_Masked/M_Ss_UMG_Masked_MixInv"))
			, UMGMasked_MixMix  (TEXT("/SpriteStudio6/UMG_Masked/M_Ss_UMG_Masked_MixMix"))
			, UMGMasked_MixMul  (TEXT("/SpriteStudio6/UMG_Masked/M_Ss_UMG_Masked_MixMul"))
			, UMGMasked_MixAdd  (TEXT("/SpriteStudio6/UMG_Masked/M_Ss_UMG_Masked_MixAdd"))
			, UMGMasked_MixSub  (TEXT("/SpriteStudio6/UMG_Masked/M_Ss_UMG_Masked_MixSub"))
			, UMGMasked_MixEff  (TEXT("/SpriteStudio6/UMG_Masked/M_Ss_UMG_Masked_MixEff"))
			, UMGOffScreen      (TEXT("/SpriteStudio6/UMG_OffScreen/M_Ss_UMG_OffScreen"))
		{}
	};
	static FConstructorStatics CS;

	// Component Default 
	Component_Default.Mix.Inv = 
	Component_Default.Mul.Inv = 
	Component_Default.Add.Inv =
	Component_Default.Sub.Inv =
	Component_Default.MulAlpha.Inv =
	Component_Default.Screen.Inv =
	Component_Default.Exclusion.Inv =
	Component_Default.Invert.Inv =
		CS.CompDefault_MixInv.Object;

	Component_Default.Mix.Mix = 
	Component_Default.Mul.Mix = 
	Component_Default.Add.Mix =
	Component_Default.Sub.Mix =
	Component_Default.MulAlpha.Mix =
	Component_Default.Screen.Mix =
	Component_Default.Exclusion.Mix =
	Component_Default.Invert.Mix =
		CS.CompDefault_MixMix.Object;

	Component_Default.Mix.Mul = 
	Component_Default.Mul.Mul = 
	Component_Default.Add.Mul =
	Component_Default.Sub.Mul =
	Component_Default.MulAlpha.Mul =
	Component_Default.Screen.Mul =
	Component_Default.Exclusion.Mul =
	Component_Default.Invert.Mul =
		CS.CompDefault_MixMul.Object;

	Component_Default.Mix.Add = 
	Component_Default.Mul.Add = 
	Component_Default.Add.Add =
	Component_Default.Sub.Add =
	Component_Default.MulAlpha.Add =
	Component_Default.Screen.Add =
	Component_Default.Exclusion.Add =
	Component_Default.Invert.Add =
		CS.CompDefault_MixAdd.Object;

	Component_Default.Mix.Sub = 
	Component_Default.Mul.Sub = 
	Component_Default.Add.Sub =
	Component_Default.Sub.Sub =
	Component_Default.MulAlpha.Sub =
	Component_Default.Screen.Sub =
	Component_Default.Exclusion.Sub =
	Component_Default.Invert.Sub =
		CS.CompDefault_MixSub.Object;

	Component_Default.Mix.Eff = 
	Component_Default.Mul.Eff = 
	Component_Default.Add.Eff =
	Component_Default.Sub.Eff =
	Component_Default.MulAlpha.Eff =
	Component_Default.Screen.Eff =
	Component_Default.Exclusion.Eff =
	Component_Default.Invert.Eff =
		CS.CompDefault_MixEff.Object;

	// Component Masked 
	Component_Masked.Mix.Inv = 
	Component_Masked.Mul.Inv = 
	Component_Masked.Add.Inv =
	Component_Masked.Sub.Inv =
	Component_Masked.MulAlpha.Inv =
	Component_Masked.Screen.Inv =
	Component_Masked.Exclusion.Inv =
	Component_Masked.Invert.Inv =
		CS.CompMasked_MixInv.Object;

	Component_Masked.Mix.Mix = 
	Component_Masked.Mul.Mix = 
	Component_Masked.Add.Mix =
	Component_Masked.Sub.Mix =
	Component_Masked.MulAlpha.Mix =
	Component_Masked.Screen.Mix =
	Component_Masked.Exclusion.Mix =
	Component_Masked.Invert.Mix =
		CS.CompMasked_MixMix.Object;

	Component_Masked.Mix.Mul = 
	Component_Masked.Mul.Mul = 
	Component_Masked.Add.Mul =
	Component_Masked.Sub.Mul =
	Component_Masked.MulAlpha.Mul =
	Component_Masked.Screen.Mul =
	Component_Masked.Exclusion.Mul =
	Component_Masked.Invert.Mul =
		CS.CompMasked_MixMul.Object;

	Component_Masked.Mix.Add = 
	Component_Masked.Mul.Add = 
	Component_Masked.Add.Add =
	Component_Masked.Sub.Add =
	Component_Masked.MulAlpha.Add =
	Component_Masked.Screen.Add =
	Component_Masked.Exclusion.Add =
	Component_Masked.Invert.Add =
		CS.CompMasked_MixAdd.Object;

	Component_Masked.Mix.Sub = 
	Component_Masked.Mul.Sub = 
	Component_Masked.Add.Sub =
	Component_Masked.Sub.Sub =
	Component_Masked.MulAlpha.Sub =
	Component_Masked.Screen.Sub =
	Component_Masked.Exclusion.Sub =
	Component_Masked.Invert.Sub =
		CS.CompMasked_MixSub.Object;

	Component_Masked.Mix.Eff = 
	Component_Masked.Mul.Eff = 
	Component_Masked.Add.Eff =
	Component_Masked.Sub.Eff =
	Component_Masked.MulAlpha.Eff =
	Component_Masked.Screen.Eff =
	Component_Masked.Exclusion.Eff =
	Component_Masked.Invert.Eff =
		CS.CompMasked_MixEff.Object;

	// Component OffScreen 
	Component_OffScreen = CS.CompOffScreen.Object;

	// UMG Default 
	UMG_Default.Mix.Inv = 
	UMG_Default.Mul.Inv = 
	UMG_Default.Add.Inv =
	UMG_Default.Sub.Inv =
	UMG_Default.MulAlpha.Inv =
	UMG_Default.Screen.Inv =
	UMG_Default.Exclusion.Inv =
	UMG_Default.Invert.Inv =
		CS.UMGDefault_MixInv.Object;

	UMG_Default.Mix.Mix = 
	UMG_Default.Mul.Mix = 
	UMG_Default.Add.Mix =
	UMG_Default.Sub.Mix =
	UMG_Default.MulAlpha.Mix =
	UMG_Default.Screen.Mix =
	UMG_Default.Exclusion.Mix =
	UMG_Default.Invert.Mix =
		CS.UMGDefault_MixMix.Object;

	UMG_Default.Mix.Mul = 
	UMG_Default.Mul.Mul = 
	UMG_Default.Add.Mul =
	UMG_Default.Sub.Mul =
	UMG_Default.MulAlpha.Mul =
	UMG_Default.Screen.Mul =
	UMG_Default.Exclusion.Mul =
	UMG_Default.Invert.Mul =
		CS.UMGDefault_MixMul.Object;

	UMG_Default.Mix.Add = 
	UMG_Default.Mul.Add = 
	UMG_Default.Add.Add =
	UMG_Default.Sub.Add =
	UMG_Default.MulAlpha.Add =
	UMG_Default.Screen.Add =
	UMG_Default.Exclusion.Add =
	UMG_Default.Invert.Add =
		CS.UMGDefault_MixAdd.Object;

	UMG_Default.Mix.Sub = 
	UMG_Default.Mul.Sub = 
	UMG_Default.Add.Sub =
	UMG_Default.Sub.Sub =
	UMG_Default.MulAlpha.Sub =
	UMG_Default.Screen.Sub =
	UMG_Default.Exclusion.Sub =
	UMG_Default.Invert.Sub =
		CS.UMGDefault_MixSub.Object;

	UMG_Default.Mix.Eff = 
	UMG_Default.Mul.Eff = 
	UMG_Default.Add.Eff =
	UMG_Default.Sub.Eff =
	UMG_Default.MulAlpha.Eff =
	UMG_Default.Screen.Eff =
	UMG_Default.Exclusion.Eff =
	UMG_Default.Invert.Eff =
		CS.UMGDefault_MixEff.Object;

	// UMG Masked 
	UMG_Masked.Mix.Inv = 
	UMG_Masked.Mul.Inv = 
	UMG_Masked.Add.Inv =
	UMG_Masked.Sub.Inv =
	UMG_Masked.MulAlpha.Inv =
	UMG_Masked.Screen.Inv =
	UMG_Masked.Exclusion.Inv =
	UMG_Masked.Invert.Inv =
		CS.UMGMasked_MixInv.Object;

	UMG_Masked.Mix.Mix = 
	UMG_Masked.Mul.Mix = 
	UMG_Masked.Add.Mix =
	UMG_Masked.Sub.Mix =
	UMG_Masked.MulAlpha.Mix =
	UMG_Masked.Screen.Mix =
	UMG_Masked.Exclusion.Mix =
	UMG_Masked.Invert.Mix =
		CS.UMGMasked_MixMix.Object;

	UMG_Masked.Mix.Mul = 
	UMG_Masked.Mul.Mul = 
	UMG_Masked.Add.Mul =
	UMG_Masked.Sub.Mul =
	UMG_Masked.MulAlpha.Mul =
	UMG_Masked.Screen.Mul =
	UMG_Masked.Exclusion.Mul =
	UMG_Masked.Invert.Mul =
		CS.UMGMasked_MixMul.Object;

	UMG_Masked.Mix.Add = 
	UMG_Masked.Mul.Add = 
	UMG_Masked.Add.Add =
	UMG_Masked.Sub.Add =
	UMG_Masked.MulAlpha.Add =
	UMG_Masked.Screen.Add =
	UMG_Masked.Exclusion.Add =
	UMG_Masked.Invert.Add =
		CS.UMGMasked_MixAdd.Object;

	UMG_Masked.Mix.Sub = 
	UMG_Masked.Mul.Sub = 
	UMG_Masked.Add.Sub =
	UMG_Masked.Sub.Sub =
	UMG_Masked.MulAlpha.Sub =
	UMG_Masked.Screen.Sub =
	UMG_Masked.Exclusion.Sub =
	UMG_Masked.Invert.Sub =
		CS.UMGMasked_MixSub.Object;

	UMG_Masked.Mix.Eff = 
	UMG_Masked.Mul.Eff = 
	UMG_Masked.Add.Eff =
	UMG_Masked.Sub.Eff =
	UMG_Masked.MulAlpha.Eff =
	UMG_Masked.Screen.Eff =
	UMG_Masked.Exclusion.Eff =
	UMG_Masked.Invert.Eff =
		CS.UMGMasked_MixEff.Object;

	// UMG OffScreen 
	UMG_OffScreen = CS.UMGOffScreen.Object;
}

