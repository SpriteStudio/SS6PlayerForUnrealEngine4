#include "SpriteStudio6PrivatePCH.h"
#include "SsGameSettings.h"


USsGameSettings::USsGameSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 各種マテリアル参照の取得
	// 参照：https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Reference/Classes/index.html#assetreferences
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_Inv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_Mix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_Mul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_Add;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_Sub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompDefault_Eff;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_Inv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_Mix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_Mul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_Add;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_Sub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompMasked_Eff;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> CompOffScreen;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_Inv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_Mix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_Mul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_Add;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_Sub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGDefault_Eff;

		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGOffScreen;

		FConstructorStatics()
			: CompDefault_Inv(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_Inv"))
			, CompDefault_Mix(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_Mix"))
			, CompDefault_Mul(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_Mul"))
			, CompDefault_Add(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_Add"))
			, CompDefault_Sub(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_Sub"))
			, CompDefault_Eff(TEXT("/SpriteStudio6/Component_Default/M_Ss_Component_Default_Eff"))
			, CompMasked_Inv (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_Inv"))
			, CompMasked_Mix (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_Mix"))
			, CompMasked_Mul (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_Mul"))
			, CompMasked_Add (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_Add"))
			, CompMasked_Sub (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_Sub"))
			, CompMasked_Eff (TEXT("/SpriteStudio6/Component_Masked/M_Ss_Component_Masked_Eff"))
			, CompOffScreen  (TEXT("/SpriteStudio6/Component_OffScreen/M_Ss_Component_OffScreen"))
			, UMGDefault_Inv (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_Inv"))
			, UMGDefault_Mix (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_Mix"))
			, UMGDefault_Mul (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_Mul"))
			, UMGDefault_Add (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_Add"))
			, UMGDefault_Sub (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_Sub"))
			, UMGDefault_Eff (TEXT("/SpriteStudio6/UMG_Default/M_Ss_UMG_Default_Eff"))
			, UMGOffScreen   (TEXT("/SpriteStudio6/UMG_OffScreen/M_Ss_UMG_OffScreen"))
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
		CS.CompDefault_Inv.Object;

	Component_Default.Mix.Mix = 
	Component_Default.Mul.Mix = 
	Component_Default.Add.Mix =
	Component_Default.Sub.Mix =
	Component_Default.MulAlpha.Mix =
	Component_Default.Screen.Mix =
	Component_Default.Exclusion.Mix =
	Component_Default.Invert.Mix =
		CS.CompDefault_Mix.Object;

	Component_Default.Mix.Mul = 
	Component_Default.Mul.Mul = 
	Component_Default.Add.Mul =
	Component_Default.Sub.Mul =
	Component_Default.MulAlpha.Mul =
	Component_Default.Screen.Mul =
	Component_Default.Exclusion.Mul =
	Component_Default.Invert.Mul =
		CS.CompDefault_Mul.Object;

	Component_Default.Mix.Add = 
	Component_Default.Mul.Add = 
	Component_Default.Add.Add =
	Component_Default.Sub.Add =
	Component_Default.MulAlpha.Add =
	Component_Default.Screen.Add =
	Component_Default.Exclusion.Add =
	Component_Default.Invert.Add =
		CS.CompDefault_Add.Object;

	Component_Default.Mix.Sub = 
	Component_Default.Mul.Sub = 
	Component_Default.Add.Sub =
	Component_Default.Sub.Sub =
	Component_Default.MulAlpha.Sub =
	Component_Default.Screen.Sub =
	Component_Default.Exclusion.Sub =
	Component_Default.Invert.Sub =
		CS.CompDefault_Sub.Object;

	Component_Default.Mix.Eff = 
	Component_Default.Mul.Eff = 
	Component_Default.Add.Eff =
	Component_Default.Sub.Eff =
	Component_Default.MulAlpha.Eff =
	Component_Default.Screen.Eff =
	Component_Default.Exclusion.Eff =
	Component_Default.Invert.Eff =
		CS.CompDefault_Eff.Object;

	// Component Masked 
	Component_Masked.Mix.Inv = 
	Component_Masked.Mul.Inv = 
	Component_Masked.Add.Inv =
	Component_Masked.Sub.Inv =
	Component_Masked.MulAlpha.Inv =
	Component_Masked.Screen.Inv =
	Component_Masked.Exclusion.Inv =
	Component_Masked.Invert.Inv =
		CS.CompMasked_Inv.Object;

	Component_Masked.Mix.Mix = 
	Component_Masked.Mul.Mix = 
	Component_Masked.Add.Mix =
	Component_Masked.Sub.Mix =
	Component_Masked.MulAlpha.Mix =
	Component_Masked.Screen.Mix =
	Component_Masked.Exclusion.Mix =
	Component_Masked.Invert.Mix =
		CS.CompMasked_Mix.Object;

	Component_Masked.Mix.Mul = 
	Component_Masked.Mul.Mul = 
	Component_Masked.Add.Mul =
	Component_Masked.Sub.Mul =
	Component_Masked.MulAlpha.Mul =
	Component_Masked.Screen.Mul =
	Component_Masked.Exclusion.Mul =
	Component_Masked.Invert.Mul =
		CS.CompMasked_Mul.Object;

	Component_Masked.Mix.Add = 
	Component_Masked.Mul.Add = 
	Component_Masked.Add.Add =
	Component_Masked.Sub.Add =
	Component_Masked.MulAlpha.Add =
	Component_Masked.Screen.Add =
	Component_Masked.Exclusion.Add =
	Component_Masked.Invert.Add =
		CS.CompMasked_Add.Object;

	Component_Masked.Mix.Sub = 
	Component_Masked.Mul.Sub = 
	Component_Masked.Add.Sub =
	Component_Masked.Sub.Sub =
	Component_Masked.MulAlpha.Sub =
	Component_Masked.Screen.Sub =
	Component_Masked.Exclusion.Sub =
	Component_Masked.Invert.Sub =
		CS.CompMasked_Sub.Object;

	Component_Masked.Mix.Eff = 
	Component_Masked.Mul.Eff = 
	Component_Masked.Add.Eff =
	Component_Masked.Sub.Eff =
	Component_Masked.MulAlpha.Eff =
	Component_Masked.Screen.Eff =
	Component_Masked.Exclusion.Eff =
	Component_Masked.Invert.Eff =
		CS.CompMasked_Eff.Object;

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
		CS.UMGDefault_Inv.Object;

	UMG_Default.Mix.Mix = 
	UMG_Default.Mul.Mix = 
	UMG_Default.Add.Mix =
	UMG_Default.Sub.Mix =
	UMG_Default.MulAlpha.Mix =
	UMG_Default.Screen.Mix =
	UMG_Default.Exclusion.Mix =
	UMG_Default.Invert.Mix =
		CS.UMGDefault_Mix.Object;

	UMG_Default.Mix.Mul = 
	UMG_Default.Mul.Mul = 
	UMG_Default.Add.Mul =
	UMG_Default.Sub.Mul =
	UMG_Default.MulAlpha.Mul =
	UMG_Default.Screen.Mul =
	UMG_Default.Exclusion.Mul =
	UMG_Default.Invert.Mul =
		CS.UMGDefault_Mul.Object;

	UMG_Default.Mix.Add = 
	UMG_Default.Mul.Add = 
	UMG_Default.Add.Add =
	UMG_Default.Sub.Add =
	UMG_Default.MulAlpha.Add =
	UMG_Default.Screen.Add =
	UMG_Default.Exclusion.Add =
	UMG_Default.Invert.Add =
		CS.UMGDefault_Add.Object;

	UMG_Default.Mix.Sub = 
	UMG_Default.Mul.Sub = 
	UMG_Default.Add.Sub =
	UMG_Default.Sub.Sub =
	UMG_Default.MulAlpha.Sub =
	UMG_Default.Screen.Sub =
	UMG_Default.Exclusion.Sub =
	UMG_Default.Invert.Sub =
		CS.UMGDefault_Sub.Object;

	UMG_Default.Mix.Eff = 
	UMG_Default.Mul.Eff = 
	UMG_Default.Add.Eff =
	UMG_Default.Sub.Eff =
	UMG_Default.MulAlpha.Eff =
	UMG_Default.Screen.Eff =
	UMG_Default.Exclusion.Eff =
	UMG_Default.Invert.Eff =
		CS.UMGDefault_Eff.Object;

	// UMG OffScreen 
	UMG_OffScreen = CS.UMGOffScreen.Object;
}

