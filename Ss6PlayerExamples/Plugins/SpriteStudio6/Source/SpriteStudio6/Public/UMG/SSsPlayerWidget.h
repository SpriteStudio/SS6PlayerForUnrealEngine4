#pragma once

class FSsPlayer;
class FSsRenderOffScreen;
struct FSlateMaterialBrush;

DECLARE_DELEGATE_OneParam(FSSsPlayerWidgetOnSlateTick, float);

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

		SLATE_SLOT_ARGUMENT(SSsPlayerWidget::FSlot, Slots)

	SLATE_END_ARGS()

public:
	SSsPlayerWidget();
	virtual ~SSsPlayerWidget();

	void Construct(const FArguments& InArgs);

	void Initialize_Default();
	void Initialize_OffScreen(
		float InResolutionX, float InResolutionY,
		uint32 InMaxVertexNum, uint32 InMaxIndexNum,
		bool bNeedMask
		);

	void SetAnimCanvasSize(const FVector2D& InSize) { AnimCanvasSize = InSize; }

	// SWidget interface 
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

public:
	FSSsPlayerWidgetOnSlateTick OnSlateTick;

private:
	void Terminate_OffScreen();

private:
	FVector2D AnimCanvasSize;


//-----------
// 親子関係 
public:
	static FSlot::FSlotArguments Slot();
	using FScopedWidgetSlotArguments = TPanelChildren<FSlot>::FScopedWidgetSlotArguments;
	FScopedWidgetSlotArguments AddSlot();
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);

	// SWidget interface 
	virtual FChildren* GetChildren() override;

public:
	// SWidget interface 
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const;

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
		const TArray<FSsRenderPart>* InRenderParts,
		const TArray<TSharedPtr<FSlateMaterialBrush>>& InBrush,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyClippingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle
		) const;

public:
	bool bReflectParentAlpha;
	const TArray<FSsRenderPart>* RenderParts;
	TArray<TSharedPtr<FSlateMaterialBrush>> DefaultBrush;
	TSharedPtr<FSlateMaterialBrush> OffScreenBrush;

private:
	bool bRenderOffScreen;
	FSsRenderOffScreen* RenderOffScreen;
};

