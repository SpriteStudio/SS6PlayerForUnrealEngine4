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
}

SSsPlayerWidget::SSsPlayerWidget()
	: SPanel()
	, AnimCanvasSize(0.f, 0.f)
	, Children(this)
	, bReflectParentAlpha(false)
	, RenderParts(nullptr)
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
	uint32 InMaxVertexNum, uint32 InMaxIndexNum,
	bool bNeedMask
	)
{
	Terminate_OffScreen();
	bRenderOffScreen = true;

	RenderOffScreen = new FSsRenderOffScreen();
	RenderOffScreen->Initialize(InResolutionX, InResolutionY, InMaxVertexNum, InMaxIndexNum, bNeedMask);
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
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SSsPlayerWidget_OnArrangeChilderen);
	if(nullptr == RenderParts)
	{
		return;
	}

	FVector2D LocalSize = AllottedGeometry.GetLocalSize();

	int32 InvalidPartCnt = 0;
	for(int32 i = 0; i < Children.Num(); ++i)
	{
		bool bValidPart = false;
		if(0 <= Children[i].PartIndexAttr.Get())
		{
			for(auto It = RenderParts->CreateConstIterator(); It; ++It)
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
						Children[i].WidgetSlot->Content->SetRenderTransformAngle(FMath::RadiansToDegrees(a01));
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
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SSsPlayerWidget_OnPaint);

	if((AllottedGeometry.Size.X <= 0.01f) || (AllottedGeometry.Size.Y <= 0.01f))
	{
		return LayerId;
	}

	FSlateRect MyClippingRect = FSlateRect(
		FVector2D(AllottedGeometry.AbsolutePosition.X, AllottedGeometry.AbsolutePosition.Y),
		FVector2D(
			AllottedGeometry.AbsolutePosition.X + AllottedGeometry.GetLocalSize().X * AllottedGeometry.Scale,
			AllottedGeometry.AbsolutePosition.Y + AllottedGeometry.GetLocalSize().Y * AllottedGeometry.Scale
			)
		);

	if(nullptr != RenderParts)
	{
		if(!bRenderOffScreen)
		{
			PaintInternal(
				RenderParts,
				DefaultBrush,
				AllottedGeometry,
				MyClippingRect,
				OutDrawElements,
				LayerId,
				InWidgetStyle
				);
		}
		else
		{
			if((0 < RenderParts->Num()) && (nullptr != OffScreenBrush.Get()))
			{
				TArray<FSsRenderPart> Parts;
				FSsRenderPart& Part = Parts.AddZeroed_GetRef();
				Part.Vertices.AddUninitialized(4);
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

				TArray<TSharedPtr<FSlateMaterialBrush>> Brush;
				Brush.Add(OffScreenBrush);

				PaintInternal(
						&Parts,
						Brush,
					AllottedGeometry,
					MyClippingRect,
					OutDrawElements,
					LayerId,
					InWidgetStyle
					);
			}
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

			FWidgetStyle WidgetStyle(InWidgetStyle);
			if(nullptr != RenderParts)
			{
				float Alpha = 1.f;
				for(auto It = RenderParts->CreateConstIterator(); It; ++It)
				{
					if(It->PartIndex == Children[ChildIndex].PartIndexAttr.Get())
					{
						if(Children[ChildIndex].ReflectPartAlphaAttr.Get())
						{
							for(int32 v = 0; v < It->Vertices.Num(); ++v)
							{
								Alpha = FMath::Min<float>(Alpha, (float)It->Vertices[v].Color.A / 255.f);
							}
						}
						break;
					}
				}
				WidgetStyle.BlendColorAndOpacityTint(FLinearColor(1.f, 1.f, 1.f, Alpha));
			}

			const int32 CurWidgetsMaxLayerId = CurWidget.Widget->Paint(
				NewArgs,
				CurWidget.Geometry,
				MyCullingRect,
				OutDrawElements,
				MaxLayerId + 1,
				WidgetStyle,
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
	const TArray<FSsRenderPart>* InRenderParts,
	const TArray<TSharedPtr<FSlateMaterialBrush>>& InBrush,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyClippingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle
	) const
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SSsPlayerWidget_PaintInternal);
	if((nullptr == InRenderParts) || (0 == InRenderParts->Num()))
	{
		return;
	}
	if(InRenderParts->Num() != InBrush.Num())
	{
		return;
	}

	FVector2D LocalSize = AllottedGeometry.GetLocalSize();


	FSlateMaterialBrush* BkBrush = nullptr;

	int32 VertexCount = 0;
	int32 IndexCount = 0;
	{
		int32 VertexCountTmp = 0;
		int32 IndexCountTmp = 0;
		for(auto It = InRenderParts->CreateConstIterator(); It; ++It)
		{
			if(	   (nullptr == InBrush[It.GetIndex()].Get())
				|| (SsBlendType::Mask == It->ColorBlendType)	// マスクパーツ（未対応） 
				)
			{
				continue;
			}
			if((0 != It.GetIndex()) && (InBrush[It.GetIndex()].Get() != BkBrush))
			{
				VertexCount = FMath::Max(VertexCount, VertexCountTmp);
				IndexCount = FMath::Max(IndexCount, IndexCountTmp);
				VertexCountTmp = IndexCountTmp = 0;
			}

			// 通常パーツ 
			if(0 == It->Mesh.Num())
			{
				check((4 == It->Vertices.Num()) || (5 == It->Vertices.Num()));
				if(4 == It->Vertices.Num())
				{
					IndexCountTmp += 6;
				}
				else
				{
					IndexCountTmp += 12;
				}

				VertexCountTmp += It->Vertices.Num();
			}
			// メッシュパーツ 
			else
			{
				for(auto ItMesh = It->Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
				{
					IndexCountTmp += ItMesh->Indices.Num();
					VertexCountTmp += ItMesh->Vertices.Num();
				}
			}

			BkBrush = InBrush[It.GetIndex()].Get();
		}
		VertexCount = FMath::Max(VertexCount, VertexCountTmp);
		IndexCount = FMath::Max(IndexCount, IndexCountTmp);
	}


	TArray<FSlateVertex> Vertices;
	TArray<SlateIndex> Indices;
	Vertices.Reserve(VertexCount);
	Indices.Reserve(IndexCount);
	for(auto It = InRenderParts->CreateConstIterator(); It; ++It)
	{
		if(	   (nullptr == InBrush[It.GetIndex()].Get())
			|| (SsBlendType::Mask == It->ColorBlendType)	// マスクパーツ（未対応） 
			)
		{
			continue;
		}
		if((0 != It.GetIndex()) && (InBrush[It.GetIndex()].Get() != BkBrush) && (0 < Indices.Num()))
		{
			FSlateResourceHandle RenderResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*(BkBrush));
			if(RenderResourceHandle.IsValid())
			{
				FSlateDrawElement::MakeCustomVerts(
					OutDrawElements,
					LayerId,
					RenderResourceHandle,
					Vertices,
					Indices,
					nullptr, 0, 0
				);
			}
			Vertices.Empty(VertexCount);
			Indices.Empty(IndexCount);
		}

		// 通常パーツ 
		if(0 == It->Mesh.Num())
		{
			check((4 == It->Vertices.Num()) || (5 == It->Vertices.Num()));
			if(4 == It->Vertices.Num())
			{
				int32 Base = Indices.Num();
				Indices.AddUninitialized(6);

				Indices[Base + 0] = Vertices.Num() + 0;
				Indices[Base + 1] = Vertices.Num() + 1;
				Indices[Base + 2] = Vertices.Num() + 3;

				Indices[Base + 3] = Vertices.Num() + 0;
				Indices[Base + 4] = Vertices.Num() + 3;
				Indices[Base + 5] = Vertices.Num() + 2;
			}
			else
			{
				int32 Base = Indices.Num();
				Indices.AddUninitialized(12);

				Indices[Base +  0] = Vertices.Num() + 0;
				Indices[Base +  1] = Vertices.Num() + 1;
				Indices[Base +  2] = Vertices.Num() + 4;

				Indices[Base +  3] = Vertices.Num() + 1;
				Indices[Base +  4] = Vertices.Num() + 3;
				Indices[Base +  5] = Vertices.Num() + 4;

				Indices[Base +  6] = Vertices.Num() + 3;
				Indices[Base +  7] = Vertices.Num() + 2;
				Indices[Base +  8] = Vertices.Num() + 4;

				Indices[Base +  9] = Vertices.Num() + 2;
				Indices[Base + 10] = Vertices.Num() + 0;
				Indices[Base + 11] = Vertices.Num() + 4;
			}

			{
				int32 Base = Vertices.Num();
				Vertices.AddUninitialized(It->Vertices.Num());
				for (int32 i = 0; i < It->Vertices.Num(); ++i)
				{
					FVector2D TransPosition = AllottedGeometry.GetAccumulatedRenderTransform().TransformPoint(
						FVector2D(
							It->Vertices[i].Position.X * LocalSize.X,
							It->Vertices[i].Position.Y * LocalSize.Y
							));
						int32 Idx = Base + i;
						Vertices[Idx].Position.X = TransPosition.X;
						Vertices[Idx].Position.Y = TransPosition.Y;
						Vertices[Idx].TexCoords[0] = It->Vertices[i].TexCoord.X;
						Vertices[Idx].TexCoords[1] = It->Vertices[i].TexCoord.Y;
						Vertices[Idx].TexCoords[2] = 0.f;
						Vertices[Idx].TexCoords[3] = It->Vertices[i].ColorBlendRate;
						Vertices[Idx].MaterialTexCoords[0] = Vertices[Idx].MaterialTexCoords[1] = 0.f;
						Vertices[Idx].Color = It->Vertices[i].Color;
					if(bReflectParentAlpha)
					{
							Vertices[Idx].Color.A *= InWidgetStyle.GetColorAndOpacityTint().A;
					}
				}
			}
		}
		// メッシュパーツ 
		else
		{
			for(auto ItMesh = It->Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
			{
				{
					int32 Base = Indices.Num();
					Indices.AddUninitialized(ItMesh->Indices.Num());
					for(auto ItIndex = ItMesh->Indices.CreateConstIterator(); ItIndex; ++ItIndex)
					{
						Indices[Base + ItIndex.GetIndex()] = Vertices.Num() + *ItIndex;
					}
				}
				{
					int32 Base = Vertices.Num();
					Vertices.AddUninitialized(ItMesh->Vertices.Num());
					for(auto ItVertex = ItMesh->Vertices.CreateConstIterator(); ItVertex; ++ItVertex)
					{
						FVector2D TransPosition = AllottedGeometry.GetAccumulatedRenderTransform().TransformPoint(
							FVector2D(
								ItVertex->Position.X * LocalSize.X,
								ItVertex->Position.Y * LocalSize.Y
							));
						int32 Idx = Base + ItVertex.GetIndex();
						Vertices[Idx].Position.X = TransPosition.X;
						Vertices[Idx].Position.Y = TransPosition.Y;
						Vertices[Idx].TexCoords[0] = ItVertex->TexCoord.X;
						Vertices[Idx].TexCoords[1] = ItVertex->TexCoord.Y;
						Vertices[Idx].TexCoords[2] = 0.f;
						Vertices[Idx].TexCoords[3] = ItMesh->ColorBlendRate;
						Vertices[Idx].MaterialTexCoords[0] = Vertices[Idx].MaterialTexCoords[1] = 0.f;
						Vertices[Idx].Color = ItMesh->Color;
						if(bReflectParentAlpha)
						{
							Vertices[Idx].Color.A *= InWidgetStyle.GetColorAndOpacityTint().A;
						}
					}
				}
			}
		}

		BkBrush = InBrush[It.GetIndex()].Get();
	}

	if(nullptr != BkBrush)
	{
		FSlateResourceHandle RenderResourceHandle = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*(BkBrush));
		if(RenderResourceHandle.IsValid())
		{
			FSlateDrawElement::MakeCustomVerts(
				OutDrawElements,
				LayerId,
				RenderResourceHandle,
				Vertices,
				Indices,
				nullptr, 0, 0
				);
		}
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
