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
	void SetPartIndex(int32 InPartIndex);

	// UObject interface 
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// UVisual interface 
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SsPlayerSlot")
	FName PartName;

private:
	SSsPlayerWidget::FSlot* Slot;
};

