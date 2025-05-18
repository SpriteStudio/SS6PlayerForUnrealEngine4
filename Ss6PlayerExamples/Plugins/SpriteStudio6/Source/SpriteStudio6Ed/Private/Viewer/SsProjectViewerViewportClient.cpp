#include "SsProjectViewerViewportClient.h"

#include "Engine/Canvas.h"
#include "AssetRegistry/AssetRegistryModule.h"

#include "SsPlayer.h"
#include "SsRenderOffScreen.h"


FSsProjectViewerViewportClient::FSsProjectViewerViewportClient()
	: bDrawGrid(false)
	, bDrawCollision(false)
	, GridSize(64)
	, GridColor(FLinearColor::Green)
	, RenderScale(1.f)
	, RenderScaleStep(0.05f)
	, RenderOffset(0.f, 0.f)
	, BackgroundColor(.2f, .2f, .2f)
	, Player(NULL)
	, Render(NULL)
	, bDragging(false)
	, LastMouseX(-1)
	, LastMouseY(-1)
{
}
FSsProjectViewerViewportClient::~FSsProjectViewerViewportClient()
{
	if(ViewerMID.IsValid())
	{
		ViewerMID->RemoveFromRoot();
		ViewerMID = nullptr;
	}
	if(CollisionBoxMat.IsValid())
	{
		CollisionBoxMat->RemoveFromRoot();
		CollisionBoxMat = nullptr;
	}
	if(CollisionCircleMat.IsValid())
	{
		CollisionCircleMat->RemoveFromRoot();
		CollisionCircleMat = nullptr;
	}
}

void FSsProjectViewerViewportClient::Draw(FViewport* Viewport, FCanvas* Canvas)
{
	FIntPoint ViewportSize = Viewport->GetSizeXY();

	FVector2D AnimCanvasSize(Player->GetAnimCanvasSize().X, Player->GetAnimCanvasSize().Y);
	FVector2D AnimPivot(Player->GetAnimPivot().X, Player->GetAnimPivot().Y);
	FVector2D CanvasScale = AnimCanvasSize * RenderScale;

	// アニメーション範囲外は黒塗りつぶし 
	Canvas->Clear(FLinearColor::Black);

	// Ssの描画
	UTexture* Texture = Render->GetRenderTarget();
	if(Texture)
	{
		if(!ViewerMID.IsValid())
		{
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(TEXT("/SpriteStudio6/Editor/M_Ss_Viewer.M_Ss_Viewer")));
			UMaterialInterface* Mat = Cast<UMaterialInterface>(AssetData.GetAsset());
			if(nullptr != Mat)
			{
				ViewerMID = UMaterialInstanceDynamic::Create(Mat, nullptr);
				ViewerMID->AddToRoot();
			}
		}
		if(ViewerMID.IsValid())
		{
			ViewerMID->SetTextureParameterValue(FName("SsRenderTarget"), Texture);
			FCanvasTileItem Tile(
				FVector2D(
					ViewportSize.X/2 - (CanvasScale.X/2) + RenderOffset.X,
					ViewportSize.Y/2 - (CanvasScale.Y/2) + RenderOffset.Y
					),
				ViewerMID->GetRenderProxy(),
				CanvasScale
				);
			Canvas->DrawItem(Tile);
		}
	}

	// グリッド
	if(bDrawGrid && (0 < GridSize))
	{
		FVector2D GridCenter(
			ViewportSize.X/2 + (AnimPivot.X * CanvasScale.X) + RenderOffset.X,
			ViewportSize.Y/2 - (AnimPivot.Y * CanvasScale.Y) + RenderOffset.Y
			);
		DrawGrid(Viewport, Canvas, GridCenter);
	}

	// 当たり判定
	if(bDrawCollision)
	{
		if(!CollisionBoxMat.IsValid())
		{
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(TEXT("/SpriteStudio6/Editor/M_Ss_Viewer_BoxCollision.M_Ss_Viewer_BoxCollision")));
			CollisionBoxMat = Cast<UMaterialInterface>(AssetData.GetAsset());
			CollisionBoxMat->AddToRoot();
		}
		if(!CollisionCircleMat.IsValid())
		{
			FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
			FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(TEXT("/SpriteStudio6/Editor/M_Ss_Viewer_CircleCollision.M_Ss_Viewer_CircleCollision")));
			CollisionCircleMat = Cast<UMaterialInterface>(AssetData.GetAsset());
			CollisionCircleMat->AddToRoot();
		}

		const TArray<FSsCollisionPart>& ColParts = Player->GetCollisionParts();
		for(auto ItColPart = ColParts.CreateConstIterator(); ItColPart; ++ItColPart)
		{
			switch(ItColPart->BoundsType)
			{
				case SsBoundsType::Quad:
					{
						FVector3f Center(ItColPart->Center.X, ItColPart->Center.Y, ItColPart->Center.Z);
						FVector3f LT = Center + ItColPart->Rotation.RotateVector(FVector3f(-ItColPart->Size.X/2.f, -ItColPart->Size.Y/2.f, 0.f));
						FVector3f RT = Center + ItColPart->Rotation.RotateVector(FVector3f( ItColPart->Size.X/2.f, -ItColPart->Size.Y/2.f, 0.f));
						FVector3f LB = Center + ItColPart->Rotation.RotateVector(FVector3f(-ItColPart->Size.X/2.f,  ItColPart->Size.Y/2.f, 0.f));
						FVector3f RB = Center + ItColPart->Rotation.RotateVector(FVector3f( ItColPart->Size.X/2.f,  ItColPart->Size.Y/2.f, 0.f));

						FVector2D LT2(	ViewportSize.X/2 - (CanvasScale.X / 2.f) + RenderOffset.X + ( LT.X * CanvasScale.X),
										ViewportSize.Y/2 - (CanvasScale.Y / 2.f) + RenderOffset.Y + (-LT.Y * CanvasScale.Y));
						FVector2D RT2(	ViewportSize.X/2 - (CanvasScale.X / 2.f) + RenderOffset.X + ( RT.X * CanvasScale.X),
										ViewportSize.Y/2 - (CanvasScale.Y / 2.f) + RenderOffset.Y + (-RT.Y * CanvasScale.Y));
						FVector2D LB2(	ViewportSize.X/2 - (CanvasScale.X / 2.f) + RenderOffset.X + ( LB.X * CanvasScale.X),
										ViewportSize.Y/2 - (CanvasScale.Y / 2.f) + RenderOffset.Y + (-LB.Y * CanvasScale.Y));
						FVector2D RB2(	ViewportSize.X/2 - (CanvasScale.X / 2.f) + RenderOffset.X + ( RB.X * CanvasScale.X),
										ViewportSize.Y/2 - (CanvasScale.Y / 2.f) + RenderOffset.Y + (-RB.Y * CanvasScale.Y));

						TArray<FCanvasUVTri> Tris;
						FCanvasUVTri Tri;
						Tri.V0_UV = Tri.V1_UV = Tri.V2_UV = FVector2D::ZeroVector;
						Tri.V0_Color = Tri.V1_Color = Tri.V2_Color = FLinearColor(1.f, 0.f, 0.f, 0.2f);

						Tri.V0_Pos = RT2;
						Tri.V1_Pos = LB2;
						Tri.V2_Pos = LT2;
						Tris.Add(Tri);

						Tri.V2_Pos = RB2;
						Tris.Add(Tri);

						FCanvasTriangleItem TriItem(Tris, nullptr);
						TriItem.MaterialRenderProxy = CollisionBoxMat->GetRenderProxy();
						Canvas->DrawItem(TriItem);
					} break;
				case SsBoundsType::Aabb:
					{
						FVector2D Center(
							ViewportSize.X/2 - (CanvasScale.X / 2.f) + RenderOffset.X + ( ItColPart->Center.X * CanvasScale.X),
							ViewportSize.Y/2 - (CanvasScale.Y / 2.f) + RenderOffset.Y + (-ItColPart->Center.Y * CanvasScale.Y)
							);
						FVector2D Size(ItColPart->Size.X * CanvasScale.X, ItColPart->Size.Y * CanvasScale.Y);
						FVector2D LeftTop = Center - (Size / 2.f);
						FCanvasTileItem Tile(LeftTop, CollisionBoxMat->GetRenderProxy(), Size);
						Canvas->DrawItem(Tile);
					} break;
				case SsBoundsType::Circle:
				case SsBoundsType::CircleSmin:
				case SsBoundsType::CircleSmax:
					{
						FVector2D Center(
							ViewportSize.X/2 - (CanvasScale.X / 2.f) + RenderOffset.X + ( ItColPart->Center.X * CanvasScale.X),
							ViewportSize.Y/2 - (CanvasScale.Y / 2.f) + RenderOffset.Y + (-ItColPart->Center.Y * CanvasScale.Y)
							);
						FVector2D Size(ItColPart->Size.X * 2.f * CanvasScale.X, ItColPart->Size.X * 2.f * CanvasScale.X);
						FVector2D LeftTop = Center - (Size / 2.f);
						FCanvasTileItem Tile(LeftTop, CollisionCircleMat->GetRenderProxy(), Size);
						Canvas->DrawItem(Tile);
					} break;
			}
		}
	}
}

bool FSsProjectViewerViewportClient::InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed, bool bGamepad)
{
	if(Key == EKeys::MouseScrollUp)
	{
		RenderScale += RenderScaleStep;
		return true;
	}
	else if(Key == EKeys::MouseScrollDown)
	{
		RenderScale -= RenderScaleStep;
		if(RenderScale < 0.1f)
		{
			RenderScale = 0.1f;
		}
		return true;
	}
	else if((Key == EKeys::F) || (Key == EKeys::A))
	{
		RenderScale = 1.f;
		RenderOffset.X = RenderOffset.Y = 0;
		return true;
	}
	else if(Key == EKeys::LeftMouseButton)
	{
		if(Event == EInputEvent::IE_Pressed)
		{
			LastMouseX = Viewport->GetMouseX();
			LastMouseY = Viewport->GetMouseY();
			bDragging = true;
		}
		else if(Event == EInputEvent::IE_Released)
		{
			bDragging = false;
		}
	}
	return false;
}

void FSsProjectViewerViewportClient::DrawGrid(FViewport* Viewport, FCanvas* Canvas, const FVector2D& Center)
{
	FIntPoint Size = Viewport->GetSizeXY();

	int32 RenderGridSize = FMath::Max(2, (int32)(GridSize * RenderScale));

	for(int32 X = Center.X+RenderGridSize; X <= Size.X; X+=RenderGridSize)
	{
		FCanvasLineItem Line(FVector2D(X,0), FVector2D(X,Size.Y));
		Line.SetColor(GridColor);
		Canvas->DrawItem(Line);
	}
	for(int32 X = Center.X-RenderGridSize; 0 <= X; X-=RenderGridSize)
	{
		FCanvasLineItem Line(FVector2D(X,0), FVector2D(X,Size.Y));
		Line.SetColor(GridColor);
		Canvas->DrawItem(Line);
	}
	{
		FCanvasLineItem Line(FVector2D(Center.X,0), FVector2D(Center.X,Size.Y));
		Line.SetColor(GridColor);
		Line.LineThickness = 3.f;
		Canvas->DrawItem(Line);
	}

	for(int32 Y = Center.Y+RenderGridSize; Y <= Size.Y; Y+=RenderGridSize)
	{
		FCanvasLineItem Line(FVector2D(0,Y), FVector2D(Size.X,Y));
		Line.SetColor(GridColor);
		Canvas->DrawItem(Line);
	}
	for(int32 Y = Center.Y-RenderGridSize; 0 <= Y; Y-=RenderGridSize)
	{
		FCanvasLineItem Line(FVector2D(0,Y), FVector2D(Size.X,Y));
		Line.SetColor(GridColor);
		Canvas->DrawItem(Line);
	}
	{
		FCanvasLineItem Line(FVector2D(0,Center.Y), FVector2D(Size.X,Center.Y));
		Line.SetColor(GridColor);
		Line.LineThickness = 3.f;
		Canvas->DrawItem(Line);
	}
}

void FSsProjectViewerViewportClient::UpdateMouse(int32 InMouseX, int32 InMouseY)
{
	if(bDragging)
	{
		RenderOffset.X += (InMouseX - LastMouseX);
		RenderOffset.Y += (InMouseY - LastMouseY);
		LastMouseX = InMouseX;
		LastMouseY = InMouseY;
	}
}
void FSsProjectViewerViewportClient::SetPlayer(FSsPlayer* InPlayer, FSsRenderOffScreen* InRender)
{
	Player = InPlayer;
	Render = InRender;
	SetBackgroundColor(BackgroundColor);
}

void FSsProjectViewerViewportClient::SetBackgroundColor(const FLinearColor& InBackgroundColor)
{
	BackgroundColor = InBackgroundColor;

	if(Render)
	{
		Render->ClearColor = BackgroundColor.ToFColor(true);
	}
}
