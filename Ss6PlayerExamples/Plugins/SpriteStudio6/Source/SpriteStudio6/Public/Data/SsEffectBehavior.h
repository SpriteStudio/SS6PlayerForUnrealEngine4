#pragma once

#include "SsEffectElement.h"
#include "SsTypes.h"
#include "SsValue.h"

#include "SsEffectBehavior.generated.h"


USTRUCT()
struct SPRITESTUDIO6_API FSsEffectBehavior
{
	GENERATED_USTRUCT_BODY()
	void Serialize(FArchive& Ar);

public:
	struct FSsCell* RefCell;
	TArray<TSharedPtr<FSsEffectElementBase>> PList;

	UPROPERTY(VisibleAnywhere, Category=SsEffectBehavior)
	FName CellName;

	UPROPERTY(VisibleAnywhere, Category=SsEffectBehavior)
	FName CellMapName;

	UPROPERTY(VisibleAnywhere, Category=SsEffectBehavior)
	TEnumAsByte<SsRenderBlendType::Type> BlendType;

public:
	FSsEffectBehavior()
		: RefCell(nullptr)
		, BlendType(SsRenderBlendType::Invalid)
	{
	}
};

