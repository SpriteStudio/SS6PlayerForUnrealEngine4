#pragma once

class FSsPlayer;
class FSsRenderOffScreen;
struct FSlateMaterialBrush;

//
class SPRITESTUDIO6_API SSsPlayerWidget : public SPanel
{
public:
	class FSlot : public TSlotBase<FSlot>
	{
	public:
		FSlot& PartIndex(const TAttribute<int32>& InPartIndex)
		{
			PartIndexAttr = InPartIndex;
			return *this;
		}
		FSlot& ReflectPartAlpha(const TAttribute<bool>& InReflectPartAlpha)
		{
			ReflectPartAlphaAttr = InReflectPartAlpha;
			return *this;
		}

		TAttribute<int32> PartIndexAttr;
		TAttribute<bool> ReflectPartAlphaAttr;

		FSlot()
			: TSlotBase<FSlot>()
			, PartIndexAttr(-1)
			, ReflectPartAlphaAttr(false)
			, WidgetSlot(nullptr)
		{}

	public:
		class USsPlayerSlot* WidgetSlot;
	};

	SLATE_BEGIN_ARGS(SSsPlayerWidget) {}

		SLATE_SUPPORTS_SLOT(SSsPlayerWidget::FSlot)

	SLATE_END_ARGS()

public:
	SSsPlayerWidget();
	virtual ~SSsPlayerWidget();

	void Construct(const FArguments& InArgs);

	void Initialize_Default();
	void Initialize_OffScreen(
		float InResolutionX, float InResolutionY,
		uint32 InMaxPartsNum
		);

	void SetAnimCanvasSize(const FVector2D& InSize) { AnimCanvasSize = InSize; }
	void SetRenderParts_Default(const TArray<FSsRenderPartWithSlateBrush>& InRenderParts);
	void SetRenderParts_OffScreen(const TArray<FSsRenderPart>& InRenderParts, TSharedPtr<FSlateMaterialBrush>& InOffscreenBrush);

	// SWidget interface 
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	void Terminate_OffScreen();

private:
	FVector2D AnimCanvasSize;


//-----------
// 親子関係 
public:
	FSlot& AddSlot();
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);

	// SWidget interface 
	virtual FChildren* GetChildren() override;

public:
	// SWidget interface 
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const;

private:
	template<class T>
	void ArrangeChildrenInternal(
		TArray<T> InRenderParts,
		const FGeometry& AllottedGeometry,
		FArrangedChildren& ArrangedChildren
		) const;

private:
	TPanelChildren<FSlot> Children;


//-----------
// 描画 
public:
	// SWidget interface 
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	UTexture* GetRenderTarget();
	FSsRenderOffScreen* GetRenderOffScreen();

private:
	void PaintInternal(
		const TArray<FSsRenderPartWithSlateBrush>& InRenderParts,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyClippingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle
		) const;

public:
	bool bIgnoreClipRect;
	bool bReflectParentAlpha;

private:
	bool bRenderOffScreen;
	TSharedPtr<FSlateMaterialBrush> OffScreenBrush;

	TArray<FSsRenderPartWithSlateBrush> RenderParts_Default;

	TArray<FSsRenderPart> RenderParts_OffScreen;
	FSsRenderOffScreen* RenderOffScreen;
};

