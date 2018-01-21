#pragma once

#include "UMG.h"

#include "SSsPlayerWidget.h"

#include "SsPlayerSlot.generated.h"


//
UCLASS()
class SPRITESTUDIO6_API USsPlayerSlot : public UPanelSlot 
{
	GENERATED_UCLASS_BODY()

public:
	virtual ~USsPlayerSlot();
	void BuildSlot(TSharedRef<SSsPlayerWidget> SsPlayerWidget);
	void SetupSlateWidget(int32 InPartIndex);

	// UObject interface 
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// UVisual interface 
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

public:
	// 子ウィジェットをアタッチするSpriteStudioパーツ名 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SsPlayerSlot")
	FName PartName;

	// 子ウィジェットに対して、SpriteStudioパーツのアルファ値を反映させるか 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SsPlayerSlot")
	bool bReflectPartAlpha;

private:
	SSsPlayerWidget::FSlot* Slot;
};

