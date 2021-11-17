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
	UFUNCTION(BlueprintCallable, Category="SsPlayerSlot")
	void SetAttachPart(FName InNewPartName);

public:
	// 子ウィジェットをアタッチするSpriteStudioパーツ名 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SsPlayerSlot")
	FName PartName;

	// 子ウィジェットに対して、SpriteStudioパーツのアルファ値を反映させるか 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SsPlayerSlot")
	bool bReflectPartAlpha;

	// パーツサイズを無視して指定値で上書き 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SsPlayerSlot")
	bool bOverridePartSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SsPlayerSlot", meta=(EditCondition="bOverridePartSize"))
	FVector2D PartSize = FVector2D(100.f, 100.f);


private:
	SSsPlayerWidget::FSlot* Slot;
};

