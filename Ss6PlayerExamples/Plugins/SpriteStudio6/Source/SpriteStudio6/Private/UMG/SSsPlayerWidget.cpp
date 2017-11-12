#include "SpriteStudio6PrivatePCH.h"
#include "SSsPlayerWidget.h"

#include "SlateMaterialBrush.h"

#include "SsPlayer.h"
#include "SsPlayerSlot.h"
#include "SsRenderOffScreen.h"


namespace
{
	static const float NearCheck = 0.01f;

	float Vector2DAngle(const FVector2D& Vec)
	{
		float Len = FVector2D::Distance(FVector2D::ZeroVector, Vec);
		float Angle = FMath::Acos(Vec.X / Len);
		if(Vec.Y < 0.f)
		{
			Angle *= -1.f;
		}
		return Angle;
	}
	float SubAngle(float Angle1, float Angle2)
	{
		if(Angle2 <= Angle1)
		{
			return Angle1 - Angle2;
		}
		else
		{
			return Angle1 + (2.f*PI) - Angle2;
		}
	}

	// 直線の交差判定 
	bool CalcLineIntersectionPoint(
		const FVector2D& V00, const FVector2D& V01,
		const FVector2D& V10, const FVector2D& V11,
		FVector2D& OutIntersectionPoint
		)
	{
		float D = (V01.X - V00.X) * (V11.Y - V10.Y) - (V01.Y - V00.Y) * (V11.X - V10.X);
		if (0.f == D)
		{
			return false;
		}

		FVector2D V00to10 = V10 - V00;
		float DR = ((V11.Y - V10.Y) * V00to10.X - (V11.X - V10.X) * V00to10.Y) / D;
		float DS = ((V01.Y - V00.Y) * V00to10.X - (V01.X - V00.X) * V00to10.Y) / D;

		OutIntersectionPoint = V00 + DR * (V01 - V00);
		return true;
	}

	// 線分の交差判定 
	bool CalcLineSegmentIntersectionPoint(
		const FVector2D& V00, const FVector2D& V01,
		const FVector2D& V10, const FVector2D& V11,
		FVector2D& OutIntersectionPoint
		)
	{
		FVector2D IntersectionPoint;
		if(!CalcLineIntersectionPoint(V00, V01, V10, V11, IntersectionPoint))
		{
			return false;
		}

		{
			float MinX = FMath::Min(V00.X, V01.X) - NearCheck;
			float MaxX = FMath::Max(V00.X, V01.X) + NearCheck;
			if((IntersectionPoint.X < MinX) || (MaxX < IntersectionPoint.X))
			{
				return false;
			}
		}
		{
			float MinY = FMath::Min(V00.Y, V01.Y) - NearCheck;
			float MaxY = FMath::Max(V00.Y, V01.Y) + NearCheck;
			if((IntersectionPoint.Y < MinY) || (MaxY < IntersectionPoint.Y))
			{
				return false;
			}
		}
		{
			float MinX = FMath::Min(V10.X, V11.X) - NearCheck;
			float MaxX = FMath::Max(V10.X, V11.X) + NearCheck;
			if((IntersectionPoint.X < MinX) || (MaxX < IntersectionPoint.X))
			{
				return false;
			}
		}
		{
			float MinY = FMath::Min(V10.Y, V11.Y) - NearCheck;
			float MaxY = FMath::Max(V10.Y, V11.Y) + NearCheck;
			if((IntersectionPoint.Y < MinY) || (MaxY < IntersectionPoint.Y))
			{
				return false;
			}
		}

		OutIntersectionPoint = IntersectionPoint;
		return true;
	}
	// 線分とRectの交差判定 
	//	@retval 交差点の数
	//	@arg OutIntersectionPoint0/1 交差点. V0に近い側が0, 遠い側が1 
	int32 CalcRectVsLineSegmentIntersectionPoint(
		const FVector2D& V0, const FVector2D& V1,
		const FSlateRect& Rect,
		FVector2D& OutIntersectionPoint0, FVector2D& OutIntersectionPoint1
		)
	{
		int32 ResultCount = 0;
		FVector2D TmpResultPoint;
		FVector2D ResultPoint[2];

		if(CalcLineSegmentIntersectionPoint(
			V0, V1, Rect.GetTopLeft(), Rect.GetTopRight(),
			TmpResultPoint
			))
		{
			bool bFound = false;
			for(int32 i = 0; i < ResultCount; ++i)
			{
				if((TmpResultPoint - ResultPoint[i]).IsNearlyZero())
				{
					bFound = true;
				}
			}
			if(!bFound)
			{
				ResultPoint[ResultCount] = TmpResultPoint;
				ResultCount++;
				check(ResultCount <= 2);
			}
		}
		if(CalcLineSegmentIntersectionPoint(
			V0, V1, Rect.GetTopRight(), Rect.GetBottomRight(),
			TmpResultPoint
			))
		{
			bool bFound = false;
			for(int32 i = 0; i < ResultCount; ++i)
			{
				if((TmpResultPoint - ResultPoint[i]).IsNearlyZero(NearCheck))
				{
					bFound = true;
				}
			}
			if(!bFound)
			{
				ResultPoint[ResultCount] = TmpResultPoint;
				ResultCount++;
				check(ResultCount <= 2);
			}
		}
		if(CalcLineSegmentIntersectionPoint(
			V0, V1, Rect.GetBottomRight(), Rect.GetBottomLeft(),
			TmpResultPoint
			))
		{
			bool bFound = false;
			for(int32 i = 0; i < ResultCount; ++i)
			{
				if((TmpResultPoint - ResultPoint[i]).IsNearlyZero(NearCheck))
				{
					bFound = true;
				}
			}
			if(!bFound)
			{
				ResultPoint[ResultCount] = TmpResultPoint;
				ResultCount++;
				check(ResultCount <= 2);
			}
		}
		if(CalcLineSegmentIntersectionPoint(
			V0, V1, Rect.GetBottomLeft(), Rect.GetTopLeft(),
			TmpResultPoint
			))
		{
			bool bFound = false;
			for(int32 i = 0; i < ResultCount; ++i)
			{
				if((TmpResultPoint - ResultPoint[i]).IsNearlyZero(NearCheck))
				{
					bFound = true;
				}
			}
			if(!bFound)
			{
				ResultPoint[ResultCount] = TmpResultPoint;
				ResultCount++;
				check(ResultCount <= 2);
			}
		}

		if(2 == ResultCount)
		{
			float DistSq0 = FVector2D::DistSquared(V0, ResultPoint[0]);
			float DistSq1 = FVector2D::DistSquared(V0, ResultPoint[1]);
			if(DistSq0 <= DistSq1)
			{
				OutIntersectionPoint0 = ResultPoint[0];
				OutIntersectionPoint1 = ResultPoint[1];
			}
			else
			{
				OutIntersectionPoint0 = ResultPoint[1];
				OutIntersectionPoint1 = ResultPoint[0];
			}
		}
		else if(1 == ResultCount)
		{
			OutIntersectionPoint0 = ResultPoint[0];
		}
		return ResultCount;
	}

	// 指定点が三角形の内側にあるか 
	bool TriangleContainsPoint(
		const FVector2D& V0,
		const FVector2D& V1,
		const FVector2D& V2,
		const FVector2D& Point
		)
	{
		float CP0 = FVector2D::CrossProduct(Point - V0, V1 - V0);
		float CP1 = FVector2D::CrossProduct(Point - V1, V2 - V1);
		float CP2 = FVector2D::CrossProduct(Point - V2, V0 - V2);

		return ((0 <= CP0) && (0 <= CP1) && (0 <= CP2))
			|| ((CP0 <= 0) && (CP1 <= 0) && (CP2 <= 0))
			;
	}

	// Rectを構成する頂点のうち、指定三角形の内側にある点のなかで、指定点に最も近い点を求める 
	bool NearestRectVertexInTriangle(
		const FSlateRect& Rect,
		const FVector2D& V0, const FVector2D& V1, const FVector2D& V2,
		const FVector2D& Point,
		FVector2D& OutPoint
		)
	{
		FVector2D TL = Rect.GetTopLeft();
		FVector2D TR = Rect.GetTopRight();
		FVector2D BL = Rect.GetBottomLeft();
		FVector2D BR = Rect.GetBottomRight();

		bool bIsInTL = TriangleContainsPoint(V0, V1, V2, TL);
		bool bIsInTR = TriangleContainsPoint(V0, V1, V2, TR);
		bool bIsInBL = TriangleContainsPoint(V0, V1, V2, BL);
		bool bIsInBR = TriangleContainsPoint(V0, V1, V2, BR);

		float DistTL = !bIsInTL ? 1000000.f : FVector2D::Distance(Point, TL);
		float DistTR = !bIsInTR ? 1000000.f : FVector2D::Distance(Point, TR);
		float DistBL = !bIsInBL ? 1000000.f : FVector2D::Distance(Point, BL);
		float DistBR = !bIsInBR ? 1000000.f : FVector2D::Distance(Point, BR);


		if(bIsInTL && (DistTL <= DistTR) && (DistTL <= DistBL) && (DistTL <= DistBR))
		{
			OutPoint = TL;
			return true;
		}
		if(bIsInTR && (DistTR <= DistBL) && (DistTR <= DistBR))
		{
			OutPoint = TR;
			return true;
		}
		if(bIsInBL && (DistBL <= DistBR))
		{
			OutPoint = BL;
			return true;
		}
		if(bIsInBR)
		{
			OutPoint = BR;
			return true;
		}
		return false;
	}

	// ３角形上の特定の座標におけるUV,Color,ColorBlendRateを求める 
	FSsRenderVertex InterpolationSsVertex(
		const FSsRenderVertex& V0,
		const FSsRenderVertex& V1,
		const FSsRenderVertex& V2,
		const FVector2D& P
		)
	{
		FVector2D Intersection_V0P_V1V2;
		if(!CalcLineIntersectionPoint(P, V0.Position, V2.Position, V1.Position, Intersection_V0P_V1V2))
		{
			// ゼロ面積３角形の場合はココに来るかも 
			return V0;
		}

		float LenV0P  = FVector2D::Distance(V0.Position, P);
		float LenV0It = FVector2D::Distance(V0.Position, Intersection_V0P_V1V2);
		float WeightV0 = (LenV0It - LenV0P) / LenV0It;

		float LenV1It = FVector2D::Distance(V1.Position, Intersection_V0P_V1V2);
		float LenV1V2 = FVector2D::Distance(V1.Position, V2.Position);
		float WeightV1 = (1.f - WeightV0) * ((LenV1V2 - LenV1It) / LenV1V2);
		float WeightV2 = (1.f - WeightV0) * (LenV1It / LenV1V2);

		check(FMath::IsNearlyEqual((WeightV0 + WeightV1 + WeightV2), 1.f, NearCheck));

		FSsRenderVertex Result;
		Result.Position = P;
		Result.Color = FColor(
			(uint8)((V0.Color.R * WeightV0) + (V1.Color.R * WeightV1) + (V2.Color.R * WeightV2)),
			(uint8)((V0.Color.G * WeightV0) + (V1.Color.G * WeightV1) + (V2.Color.G * WeightV2)),
			(uint8)((V0.Color.B * WeightV0) + (V1.Color.B * WeightV1) + (V2.Color.B * WeightV2)),
			(uint8)((V0.Color.A * WeightV0) + (V1.Color.A * WeightV1) + (V2.Color.A * WeightV2))
			);
		Result.ColorBlendRate = (V0.ColorBlendRate * WeightV0) + (V1.ColorBlendRate * WeightV1) + (V2.ColorBlendRate * WeightV2);
		Result.TexCoord = (V0.TexCoord * WeightV0) + (V1.TexCoord * WeightV1) + (V2.TexCoord * WeightV2);

		return Result;
	}

	// 最後の要素と異なる座標の頂点であれば配列に追加 
	void AddArrayIfDifferent(TArray<FSsRenderVertex>& Array, const FSsRenderVertex& NewElem)
	{
		if(0 == Array.Num())
		{
			Array.Add(NewElem);
		}
		else if(Array[Array.Num()-1].Position != NewElem.Position)
		{
			Array.Add(NewElem);
		}
	}
}

SSsPlayerWidget::SSsPlayerWidget()
	: SPanel()
	, AnimCanvasSize(0.f, 0.f)
	, bIgnoreClipRect(true)
	, bRenderOffScreen(false)
	, RenderOffScreen(nullptr)
{
}
SSsPlayerWidget::~SSsPlayerWidget()
{
	Terminate_OffScreen();
}

void SSsPlayerWidget::Construct(const FArguments& InArgs)
{
	for(int32 i = 0; i < InArgs.Slots.Num(); ++i)
	{
		Children.Add(InArgs.Slots[i]);
	}
}

void SSsPlayerWidget::Initialize_Default()
{
	Terminate_OffScreen();
	bRenderOffScreen = false;
}
void SSsPlayerWidget::Initialize_OffScreen(
	float InResolutionX, float InResolutionY,
	uint32 InMaxPartsNum
	)
{
	Terminate_OffScreen();
	bRenderOffScreen = true;

	RenderOffScreen = new FSsRenderOffScreen();
	RenderOffScreen->Initialize(InResolutionX, InResolutionY, InMaxPartsNum);
}
void SSsPlayerWidget::Terminate_OffScreen()
{
	bRenderOffScreen = false;
	OffScreenBrush.Reset();
	if(RenderOffScreen)
	{
		RenderOffScreen->ReserveTerminate();
		RenderOffScreen = nullptr;
	}
}


//
// 描画用パーツ情報の登録 (Default) 
//
void SSsPlayerWidget::SetRenderParts_Default(const TArray<FSsRenderPartWithSlateBrush>& InRenderParts)
{
	RenderParts_Default = InRenderParts;
}

//
// 描画用パーツ情報の登録 (OffScreen) 
//
void SSsPlayerWidget::SetRenderParts_OffScreen(
	const TArray<FSsRenderPart>& InRenderParts,
	TSharedPtr<FSlateMaterialBrush>& InOffscreenBrush
	)
{
	RenderParts_OffScreen = InRenderParts;
	if(RenderOffScreen)
	{
		RenderOffScreen->Render(RenderParts_OffScreen);
	}
	OffScreenBrush = InOffscreenBrush;
}

FVector2D SSsPlayerWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	return (AnimCanvasSize != FVector2D::ZeroVector) ? AnimCanvasSize : FVector2D(256.f, 256.f);
}


SSsPlayerWidget::FSlot& SSsPlayerWidget::AddSlot()
{
	Invalidate(EInvalidateWidget::Layout);

	FSlot* NewSlot = new FSlot();
	Children.Add(NewSlot);
	return *NewSlot;
}
int32 SSsPlayerWidget::RemoveSlot(const TSharedRef<SWidget>& SlotWidget)
{
	Invalidate(EInvalidateWidget::Layout);

	for(int32 SlotIdx = 0; SlotIdx < Children.Num(); ++SlotIdx)
	{
		if(SlotWidget == Children[SlotIdx].GetWidget())
		{
			Children.RemoveAt(SlotIdx);
			return SlotIdx;
		}
	}

	return -1;
}
FChildren* SSsPlayerWidget::GetChildren()
{
	return &Children;
}

void SSsPlayerWidget::OnArrangeChildren(
	const FGeometry& AllottedGeometry,
	FArrangedChildren& ArrangedChildren
	) const
{	
	if(!bRenderOffScreen)
	{
		ArrangeChildrenInternal<FSsRenderPartWithSlateBrush>(
			RenderParts_Default,
			AllottedGeometry,
			ArrangedChildren
			);
	}
	else
	{
		ArrangeChildrenInternal<FSsRenderPart>(
			RenderParts_OffScreen,
			AllottedGeometry,
			ArrangedChildren
			);
	}
}
template<class T>
void SSsPlayerWidget::ArrangeChildrenInternal(
	TArray<T> InRenderParts,
	const FGeometry& AllottedGeometry,
	FArrangedChildren& ArrangedChildren
	) const
{
	FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	int32 InvalidPartCnt = 0;
	for(int32 i = 0; i < Children.Num(); ++i)
	{
		bool bValidPart = false;
		if(0 <= Children[i].PartIndexAttr.Get())
		{
			for(auto It = InRenderParts.CreateConstIterator(); It; ++It)
			{
				if(It->PartIndex == Children[i].PartIndexAttr.Get())
				{
					FVector2D VertPosition[3];
					for(int32 ii = 0; ii < 3; ++ii)
					{
						VertPosition[ii] = FVector2D(
							It->Vertices[ii].Position.X * LocalSize.X,
							It->Vertices[ii].Position.Y * LocalSize.Y
							);
					}

					FVector2D d01(VertPosition[1].X - VertPosition[0].X, VertPosition[1].Y - VertPosition[0].Y);
					FVector2D d02(VertPosition[2].X - VertPosition[0].X, VertPosition[2].Y - VertPosition[0].Y);
					float len01 = FVector2D::Distance(VertPosition[0], VertPosition[1]);
					float len02 = FVector2D::Distance(VertPosition[0], VertPosition[2]);
					float a01 = Vector2DAngle(VertPosition[1] - VertPosition[0]);
					float a02 = Vector2DAngle(VertPosition[2] - VertPosition[0]);
					float sub_a02_01 = SubAngle(a02, a01);

					float Width  = len01;
					float Height = FMath::Sin(sub_a02_01) * len02;

					//
					// 前状態がキャッシュされてるようで、直前が面積ゼロだった時に１回だけTransformMatrixが壊れる 
					// 回避のため、計算前にリセットしておく 
					//
					Children[i].GetWidget()->SetRenderTransform(FSlateRenderTransform());

					if(Children[i].WidgetSlot && Children[i].WidgetSlot->Content)
					{
						Children[i].WidgetSlot->Content->SetRenderTransformPivot(FVector2D::ZeroVector);
						Children[i].WidgetSlot->Content->SetRenderAngle(FMath::RadiansToDegrees(a01));
						Children[i].WidgetSlot->Content->SetRenderShear(
							FVector2D(FMath::RadiansToDegrees((PI/2.f)- sub_a02_01), 0.f)
							);
					}

					ArrangedChildren.AddWidget(
						EVisibility::Visible,
						AllottedGeometry.MakeChild(
							Children[i].GetWidget(),
							FVector2D(
								It->Vertices[0].Position.X * LocalSize.X,
								It->Vertices[0].Position.Y * LocalSize.Y
								),
							FVector2D(Width, Height)
							)
						);

					bValidPart = true;
					break;
				}
			}
		}

		if(!bValidPart)
		{
			ArrangedChildren.AddWidget(
				EVisibility::Visible,
				AllottedGeometry.MakeChild(
					Children[i].GetWidget(),
					FVector2D(InvalidPartCnt * 50.f, 0.f),
					FVector2D(50.f, 50.f)
					)
				);
			++InvalidPartCnt;
		}
	}
}

//
// 描画 
//
int32 SSsPlayerWidget::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled
	) const
{
	FSlateRect MyClippingRect = FSlateRect(
		FVector2D(AllottedGeometry.AbsolutePosition.X, AllottedGeometry.AbsolutePosition.Y),
		FVector2D(
			AllottedGeometry.AbsolutePosition.X + AllottedGeometry.GetLocalSize().X * AllottedGeometry.Scale,
			AllottedGeometry.AbsolutePosition.Y + AllottedGeometry.GetLocalSize().Y * AllottedGeometry.Scale
			)
		);

	if(!bRenderOffScreen)
	{
		PaintInternal(
			RenderParts_Default,
			AllottedGeometry,
			MyClippingRect,
			OutDrawElements,
			LayerId
			);
	}
	else
	{
		if((0 < RenderParts_OffScreen.Num()) && (nullptr != OffScreenBrush.Get()))
		{
			FSsRenderPartWithSlateBrush Part;
			Part.PartIndex = -1;
			Part.Vertices[0].Position.X = 0.f;
			Part.Vertices[0].Position.Y = 0.f;
			Part.Vertices[0].TexCoord.X = 0.f;
			Part.Vertices[0].TexCoord.Y = 0.f;
			Part.Vertices[0].Color = FColor::White;
			Part.Vertices[0].ColorBlendRate = 0.f;
			Part.Vertices[1].Position.X = 1.f;
			Part.Vertices[1].Position.Y = 0.f;
			Part.Vertices[1].TexCoord.X = 1.f;
			Part.Vertices[1].TexCoord.Y = 0.f;
			Part.Vertices[1].Color = FColor::White;
			Part.Vertices[1].ColorBlendRate = 0.f;
			Part.Vertices[2].Position.X = 0.f;
			Part.Vertices[2].Position.Y = 1.f;
			Part.Vertices[2].TexCoord.X = 0.f;
			Part.Vertices[2].TexCoord.Y = 1.f;
			Part.Vertices[2].Color = FColor::White;
			Part.Vertices[2].ColorBlendRate = 0.f;
			Part.Vertices[3].Position.X = 1.f;
			Part.Vertices[3].Position.Y = 1.f;
			Part.Vertices[3].TexCoord.X = 1.f;
			Part.Vertices[3].TexCoord.Y = 1.f;
			Part.Vertices[3].Color = FColor::White;
			Part.Vertices[3].ColorBlendRate = 0.f;
			Part.ColorBlendType = SsBlendType::Mix;
			Part.AlphaBlendType = SsBlendType::Mix;
			Part.Texture  = nullptr;
			Part.Brush = OffScreenBrush;

			TArray<FSsRenderPartWithSlateBrush> Parts;
			Parts.Add(Part);
			PaintInternal(
				Parts,
				AllottedGeometry,
				MyClippingRect,
				OutDrawElements,
				LayerId
				);
		}
	}


	// 子階層の描画 
	FArrangedChildren ArrangedChildren(EVisibility::Visible);
	ArrangeChildren(AllottedGeometry, ArrangedChildren);

	bool bForwardedEnabled = ShouldBeEnabled(bParentEnabled);
	int32 MaxLayerId = LayerId;

	const FPaintArgs NewArgs = Args.WithNewParent(this);

	for(int32 ChildIndex = 0; ChildIndex < ArrangedChildren.Num(); ++ChildIndex)
	{
		FArrangedWidget& CurWidget = ArrangedChildren[ChildIndex];

		bool bWereOverlapping;
		FSlateRect ChildClipRect = MyClippingRect.IntersectionWith(CurWidget.Geometry.GetLayoutBoundingRect(), bWereOverlapping);

		if(bWereOverlapping)
		{
			FSlateClippingManager& ClippingManager = OutDrawElements.GetClippingManager();
			if(EWidgetClipping::Inherit != this->Clipping)
			{
				ClippingManager.PushClip(FSlateClippingZone(CurWidget.Geometry));
			}

			const int32 CurWidgetsMaxLayerId = CurWidget.Widget->Paint(
				NewArgs,
				CurWidget.Geometry,
				MyCullingRect,
				OutDrawElements,
				MaxLayerId + 1,
				InWidgetStyle,
				bForwardedEnabled
				);
			MaxLayerId = FMath::Max(MaxLayerId, CurWidgetsMaxLayerId);

			if(EWidgetClipping::Inherit != this->Clipping)
			{
				ClippingManager.PopClip();
			}
		}
	}

	return MaxLayerId;
}
void SSsPlayerWidget::PaintInternal(
	const TArray<FSsRenderPartWithSlateBrush>& InRenderParts,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyClippingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId
	) const
{
	if(0 == InRenderParts.Num())
	{
		return;
	}

	FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	struct FRenderData
	{
		TArray<FSlateVertex> Vertices;
		TArray<SlateIndex> Indices;
		FSlateResourceHandle RenderResourceHandle;
	};

	TArray<FRenderData> RenderDataArray;
	FRenderData RenderData;
	FSlateMaterialBrush* BkBrush = nullptr;
	SsBlendType::Type BkAlphaBlendType = SsBlendType::Invalid;
	for(auto It = InRenderParts.CreateConstIterator(); It; ++It)
	{
		if((0 != It.GetIndex())
		   && (   (It->Brush.Get() != BkBrush)
			   || (It->AlphaBlendType != BkAlphaBlendType)
			   )
			)
		{
			RenderDataArray.Add(RenderData);
			RenderData = FRenderData();
		}

		bool bAllInClipRect = false;

		// クリッピング 
		if(!bIgnoreClipRect)
		{
			FSsRenderVertex Verts[4];
			bool bIsInClipRect[4];
			for(int32 i = 0; i < 4; ++i)
			{
				Verts[i] = It->Vertices[i];
				Verts[i].Position = FVector2D(
					AllottedGeometry.AbsolutePosition.X + It->Vertices[i].Position.X * LocalSize.X * AllottedGeometry.Scale,
					AllottedGeometry.AbsolutePosition.Y + It->Vertices[i].Position.Y * LocalSize.Y * AllottedGeometry.Scale
					);
				bIsInClipRect[i] = MyClippingRect.ContainsPoint(Verts[i].Position);
			}
			// 全頂点がClipRectの内側であれば、Clip無しと同様の描画 
			bAllInClipRect = bIsInClipRect[0] & bIsInClipRect[1] & bIsInClipRect[2] & bIsInClipRect[3];

			if(!bAllInClipRect)
			{
				const int32 EdgeList[2][3][3] =
				{
					// tri[0-1-3] 
					{
						{ 0, 1, 3 },
						{ 1, 3, 0 },
						{ 3, 0, 1 },
					},
					// tri[0-3-2] 
					{
						{ 0, 3, 2 },
						{ 3, 2, 0 },
						{ 2, 0, 3 },
					}
				};

				for(int32 t = 0; t < 2; ++t)
				{
					TArray<FSsRenderVertex> TmpVerts;
					for(int32 i = 0; i < 3; ++i)
					{
						int32 Idx0 = EdgeList[t][i][0];
						int32 Idx1 = EdgeList[t][i][1];
						int32 Idx2 = EdgeList[t][i][2];

						// 2頂点ともClipRectの内側 
						if(bIsInClipRect[Idx0] && bIsInClipRect[Idx1])
						{
							AddArrayIfDifferent(TmpVerts, Verts[Idx0]);
							AddArrayIfDifferent(TmpVerts, Verts[Idx1]);
						}
						// 2頂点ともClipRectの外側 
						else if(!bIsInClipRect[Idx0] && !bIsInClipRect[Idx1])
						{
							FVector2D IntersectionPoint[2];
							int32 IntersectionCount = CalcRectVsLineSegmentIntersectionPoint(
								Verts[Idx0].Position, Verts[Idx1].Position,
								MyClippingRect,
								IntersectionPoint[0], IntersectionPoint[1]
								);
							// 外から外へ突き抜ける 
							if(2 == IntersectionCount)
							{
								AddArrayIfDifferent(TmpVerts, InterpolationSsVertex(Verts[Idx0], Verts[Idx1], Verts[Idx2], IntersectionPoint[0]));
								AddArrayIfDifferent(TmpVerts, InterpolationSsVertex(Verts[Idx0], Verts[Idx1], Verts[Idx2], IntersectionPoint[1]));
							}
							// 完全に外側(１点交差の場合もこちらとみなす) 
							else
							{
								FVector2D P;
								if(NearestRectVertexInTriangle(
									MyClippingRect,
									Verts[Idx0].Position, Verts[Idx1].Position, Verts[Idx2].Position,
									Verts[Idx0].Position,
									P
									))
								{
									AddArrayIfDifferent(TmpVerts, InterpolationSsVertex(Verts[Idx0], Verts[Idx1], Verts[Idx2], P));
								}
								if(NearestRectVertexInTriangle(
									MyClippingRect,
									Verts[Idx0].Position, Verts[Idx1].Position, Verts[Idx2].Position,
									Verts[Idx1].Position,
									P
									))
								{
									AddArrayIfDifferent(TmpVerts, InterpolationSsVertex(Verts[Idx0], Verts[Idx1], Verts[Idx2], P));
								}
							}
						}
						// 内側から外側へ 
						else if(bIsInClipRect[Idx0])
						{
							AddArrayIfDifferent(TmpVerts, Verts[Idx0]);

							FVector2D IntersectionPoint[2];
							int32 IntersectionCount = CalcRectVsLineSegmentIntersectionPoint(
								Verts[Idx0].Position, Verts[Idx1].Position,
								MyClippingRect,
								IntersectionPoint[0], IntersectionPoint[1]
								);
							check(1 <= IntersectionCount);

							for(int32 j = 0; j < IntersectionCount; ++j)
							{
								AddArrayIfDifferent(TmpVerts, InterpolationSsVertex(Verts[Idx0], Verts[Idx1], Verts[Idx2], IntersectionPoint[j]));
							}
						}
						// 外側から内側へ 
						else
						{
							FVector2D IntersectionPoint[2];
							int32 IntersectionCount = CalcRectVsLineSegmentIntersectionPoint(
								Verts[Idx0].Position, Verts[Idx1].Position,
								MyClippingRect,
								IntersectionPoint[0], IntersectionPoint[1]
								);
							check(1 <= IntersectionCount);
							for(int32 j = 0; j < IntersectionCount; ++j)
							{
								AddArrayIfDifferent(TmpVerts, InterpolationSsVertex(Verts[Idx0], Verts[Idx1], Verts[Idx2], IntersectionPoint[j]));
							}

							AddArrayIfDifferent(TmpVerts, Verts[Idx1]);
						}
					}

					// TriangleFan 
					for(int32 i = 1; i < TmpVerts.Num() - 1; ++i)
					{
						RenderData.Indices.Add(RenderData.Vertices.Num() + 0);
						RenderData.Indices.Add(RenderData.Vertices.Num() + i);
						RenderData.Indices.Add(RenderData.Vertices.Num() + i + 1);
					}
					for(int32 i = 0; i < TmpVerts.Num(); ++i)
					{
						FVector2D TransPosition =
							AllottedGeometry.GetAccumulatedRenderTransform().TransformPoint(
								FVector2D(
									(TmpVerts[i].Position.X - AllottedGeometry.AbsolutePosition.X) / AllottedGeometry.Scale,
									(TmpVerts[i].Position.Y - AllottedGeometry.AbsolutePosition.Y) / AllottedGeometry.Scale
									));

						FSlateVertex Vert;
						Vert.Position[0] = TransPosition.X;
						Vert.Position[1] = TransPosition.Y;

						Vert.TexCoords[0] = TmpVerts[i].TexCoord.X;
						Vert.TexCoords[1] = TmpVerts[i].TexCoord.Y;
						Vert.TexCoords[2] = 0.f;
						Vert.TexCoords[3] = TmpVerts[i].ColorBlendRate;
						Vert.MaterialTexCoords[0] = Vert.MaterialTexCoords[1] = 0.f;
						Vert.Color = TmpVerts[i].Color;
						RenderData.Vertices.Add(Vert);
					}
				}
			}
		}

		// クリッピング無し 
		if(bIgnoreClipRect || bAllInClipRect)
		{
			RenderData.Indices.Add(RenderData.Vertices.Num() + 0);
			RenderData.Indices.Add(RenderData.Vertices.Num() + 1);
			RenderData.Indices.Add(RenderData.Vertices.Num() + 3);
			RenderData.Indices.Add(RenderData.Vertices.Num() + 0);
			RenderData.Indices.Add(RenderData.Vertices.Num() + 3);
			RenderData.Indices.Add(RenderData.Vertices.Num() + 2);

			for (int32 i = 0; i < 4; ++i)
			{
				FVector2D TransPosition = AllottedGeometry.GetAccumulatedRenderTransform().TransformPoint(
					FVector2D(
						It->Vertices[i].Position.X * LocalSize.X,
						It->Vertices[i].Position.Y * LocalSize.Y
						));
				FSlateVertex Vert;
				Vert.Position[0] = TransPosition.X;
				Vert.Position[1] = TransPosition.Y;
				Vert.TexCoords[0] = It->Vertices[i].TexCoord.X;
				Vert.TexCoords[1] = It->Vertices[i].TexCoord.Y;
				Vert.TexCoords[2] = 0.f;
				Vert.TexCoords[3] = It->Vertices[i].ColorBlendRate;
				Vert.MaterialTexCoords[0] = Vert.MaterialTexCoords[1] = 0.f;
				Vert.Color = It->Vertices[i].Color;
				//Vert.ClipRect = FSlateRotatedClipRectType(MyClippingRect);	// MakeCustomVertsでは無視される 

				RenderData.Vertices.Add(Vert);
			}
		}

		RenderData.RenderResourceHandle =
			FSlateApplication::Get().GetRenderer()->GetResourceHandle(
				*It->Brush.Get()
				);

		BkBrush = It->Brush.Get();
		BkAlphaBlendType = It->AlphaBlendType;	// そもそもアルファブレンドモードはサポート出来なさげ 
	}
	RenderDataArray.Add(RenderData);

	for(auto It = RenderDataArray.CreateConstIterator(); It; ++It)
	{
		if(0 == (*It).Indices.Num())
		{
			continue;
		}
		FSlateDrawElement::MakeCustomVerts(
			OutDrawElements,
			LayerId,
			(*It).RenderResourceHandle,
			(*It).Vertices,
			(*It).Indices,
			nullptr, 0, 0
			);
	}
}

UTexture* SSsPlayerWidget::GetRenderTarget()
{
	if(RenderOffScreen)
	{
		return RenderOffScreen->GetRenderTarget();
	}
	return nullptr;
}


FSsRenderOffScreen* SSsPlayerWidget::GetRenderOffScreen()
{
	return RenderOffScreen;
}
