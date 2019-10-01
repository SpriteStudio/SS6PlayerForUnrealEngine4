#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerComponent.h"

#include "SsGameSettings.h"
#include "Ss6Project.h"
#include "SsAnimePack.h"
#include "SsPlayer.h"
#include "SsRenderOffScreen.h"
#include "SsRenderPlaneProxy.h"
#include "SsRenderPartsProxy.h"


namespace
{
	UMaterialInterface* GetBaseMaterialCompInternal(const FSsColorBlendModeMaterials& Mats, SsBlendType::Type ColorBlendMode)
	{
		switch(ColorBlendMode)
		{
			case SsBlendType::Invalid: return Mats.Inv;
			case SsBlendType::Mix:     return Mats.Mix;
			case SsBlendType::Mul:     return Mats.Mul;
			case SsBlendType::Add:     return Mats.Add;
			case SsBlendType::Sub:     return Mats.Sub;
			case SsBlendType::Effect:  return Mats.Eff;
		}
		check(false);
		return nullptr;
	}
	UMaterialInterface* GetBaseMaterialComp(ESsPlayerComponentRenderMode::Type RenderMode, SsBlendType::Type AlphaBlendMode, SsBlendType::Type ColorBlendMode)
	{
		switch(RenderMode)
		{
			case ESsPlayerComponentRenderMode::Default:
				{
					switch(AlphaBlendMode)
					{
						case SsBlendType::Mix:       return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Default.Mix,       ColorBlendMode);
						case SsBlendType::Mul:       return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Default.Mul,       ColorBlendMode);
						case SsBlendType::Add:       return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Default.Add,       ColorBlendMode);
						case SsBlendType::Sub:       return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Default.Sub,       ColorBlendMode);
						case SsBlendType::MulAlpha:  return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Default.MulAlpha,  ColorBlendMode);
						case SsBlendType::Screen:    return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Default.Screen,    ColorBlendMode);
						case SsBlendType::Exclusion: return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Default.Exclusion, ColorBlendMode);
						case SsBlendType::Invert:    return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Default.Invert,    ColorBlendMode);
						case SsBlendType::Invalid:   return nullptr;
					}
				} break;
			case ESsPlayerComponentRenderMode::Masked:
				{
					switch(AlphaBlendMode)
					{
						case SsBlendType::Mix:       return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Masked.Mix,       ColorBlendMode);
						case SsBlendType::Mul:       return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Masked.Mul,       ColorBlendMode);
						case SsBlendType::Add:       return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Masked.Add,       ColorBlendMode);
						case SsBlendType::Sub:       return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Masked.Sub,       ColorBlendMode);
						case SsBlendType::MulAlpha:  return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Masked.MulAlpha,  ColorBlendMode);
						case SsBlendType::Screen:    return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Masked.Screen,    ColorBlendMode);
						case SsBlendType::Exclusion: return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Masked.Exclusion, ColorBlendMode);
						case SsBlendType::Invert:    return GetBaseMaterialCompInternal(GetDefault<USsGameSettings>()->Component_Masked.Invert,    ColorBlendMode);
						case SsBlendType::Invalid:   return nullptr;
					}
				} break;
		}
		check(false);
		return nullptr;
	}
};

// コンストラクタ
USsPlayerComponent::USsPlayerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FSsPlayPropertySync(&SsProject, &AutoPlayAnimPackName, &AutoPlayAnimationName, &AutoPlayAnimPackIndex, &AutoPlayAnimationIndex)
	, RenderOffScreen(NULL)
	, SsProject(NULL)
	, bAutoUpdate(true)
	, bAutoPlay(true)
	, AutoPlayAnimPackIndex(0)
	, AutoPlayAnimationIndex(0)
	, AutoPlayStartFrame(0)
	, AutoPlayRate(1.f)
	, AutoPlayLoopCount(0)
	, bAutoPlayRoundTrip(false)
	, RenderMode(ESsPlayerComponentRenderMode::Default)
	, PixelDepthOffsetPerPart(0.01f)
	, BaseMaterial(NULL)
	, OffScreenPlaneMID(NULL)
	, OffScreenRenderResolution(512.f, 512.f)
	, OffScreenClearColor(0, 0, 0, 0)
	, UUPerPixel(0.3f)
	, SsBoundsScale(2.f)
{
	// UActorComponent
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
	bTickInEditor = true;
	bAutoActivate = true;

	// UPrimitiveComponent
	CastShadow = false;
	bUseAsOccluder = false;
	bCanEverAffectNavigation = false;


	BaseMaterial = GetDefault<USsGameSettings>()->Component_OffScreen;
}

// シリアライズ 
void USsPlayerComponent::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	FSsPlayPropertySync::OnSerialize(Ar);
}

#if WITH_EDITOR
// プロパティ編集イベント 
void USsPlayerComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FSsPlayPropertySync::OnPostEditChangeProperty(PropertyChangedEvent);
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif


// ソケットを保持しているか 
bool USsPlayerComponent::HasAnySockets() const
{
	if(ESsPlayerComponentRenderMode::OffScreenOnly == RenderMode)
	{
		return false;
	}
	if(nullptr != SsProject)
	{
		int32 AnimPackIndex  = Player.GetPlayingAnimPackIndex();
		int32 AnimationIndex = Player.GetPlayingAnimationIndex();
#if WITH_EDITOR
		if(!Player.GetSsProject().IsValid())
		{
			AnimPackIndex  = AutoPlayAnimPackIndex;
			AnimationIndex = AutoPlayAnimationIndex;
		}
#endif
		if((0 <= AnimPackIndex) && (0 <= AnimationIndex))
		{
			FSsAnimation& Animation = SsProject->AnimeList[AnimPackIndex].AnimeList[AnimationIndex];
			return (0 < Animation.PartAnimes.Num());
		}
	}
	return false;
}
// 指定した名前のソケットが存在するか 
bool USsPlayerComponent::DoesSocketExist(FName InSocketName) const
{
	return (RenderMode != ESsPlayerComponentRenderMode::OffScreenOnly)
		&& (0 <= Player.GetPartIndexFromName(InSocketName));
}
// 全ソケットの情報を取得 
void USsPlayerComponent::QuerySupportedSockets(TArray<FComponentSocketDescription>& OutSockets) const
{
	OutSockets.Empty();
	if(    (RenderMode != ESsPlayerComponentRenderMode::OffScreenOnly)
		&& (nullptr != SsProject)
		)
	{
		int32 AnimPackIndex  = Player.GetPlayingAnimPackIndex();
		int32 AnimationIndex = Player.GetPlayingAnimationIndex();
#if WITH_EDITOR
		if(!Player.GetSsProject().IsValid())
		{
			AnimPackIndex  = AutoPlayAnimPackIndex;
			AnimationIndex = AutoPlayAnimationIndex;
		}
#endif
		if((0 <= AnimPackIndex) && (0 <= AnimationIndex))
		{
			TArray<FSsPart>& PartList = SsProject->AnimeList[AnimPackIndex].Model.PartList;
			OutSockets.Reserve(PartList.Num());
			for(auto ItPart = PartList.CreateConstIterator(); ItPart; ++ItPart)
			{
				OutSockets.Add(
					FComponentSocketDescription(
						ItPart->PartName,
						EComponentSocketType::Socket
						));
			}
		}
	}
}
// ソケットのTransformを取得 
FTransform USsPlayerComponent::GetSocketTransform(FName InSocketName, ERelativeTransformSpace TransformSpace) const
{
	if(InSocketName.IsNone())
	{
		return Super::GetSocketTransform(InSocketName, TransformSpace);
	}

	if(RenderMode != ESsPlayerComponentRenderMode::OffScreenOnly)
	{
		int32 PartIndex = Player.GetPartIndexFromName(InSocketName);	
		FTransform Trans;
		if(GetPartAttachTransform(PartIndex, Trans))
		{
			switch(TransformSpace)
			{
			case ERelativeTransformSpace::RTS_World:
				{
					return Trans * GetComponentTransform();
				} break;
			case ERelativeTransformSpace::RTS_Actor:
				{
					AActor* Actor = GetOwner();
					return (NULL == Actor) ? Trans : (GetComponentTransform() *  Trans).GetRelativeTransform(Actor->GetTransform());
				} break;
			case ERelativeTransformSpace::RTS_Component:
				{
					return Trans;
				} break;
			}
		}
	}

	if(RenderMode == ESsPlayerComponentRenderMode::OffScreenOnly)
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerComponent::GetSocketTransform() Can't Attach. RenderMode is OffScreenOnly"), *(InSocketName.ToString()));
	}
	else
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerComponent::GetSocketTransform() Invalid Socket Name (%s)"), *(InSocketName.ToString()));
	}
	return Super::GetSocketTransform(InSocketName, TransformSpace);
}


// コンポーネント登録時の初期化
void USsPlayerComponent::OnRegister()
{
	Super::OnRegister();

	if(FApp::CanEverRender() && SsProject)	// FApp::CanEverRender() : コマンドラインからのCook時にも呼び出され、テクスチャリソースが確保されていない状態で処理が流れてしまうのを防ぐため 
	{
#if WITH_EDITOR
		// Reimportでインデックスが変わった場合に即座に反映 
		SyncAutoPlayAnimation_NameToIndex();
#endif

		// Playerの初期化
		Player.SetSsProject(SsProject);

		// 自動再生
		if(bAutoPlay)
		{
			Player.Play(AutoPlayAnimPackIndex, AutoPlayAnimationIndex, AutoPlayStartFrame, AutoPlayRate, AutoPlayLoopCount, bAutoPlayRoundTrip);
			Player.bFlipH = bAutoPlayFlipH;
			Player.bFlipV = bAutoPlayFlipV;
			UpdateBounds();
		}

		// オフスクリーンレンダリングの初期化
		if(    (NULL == RenderOffScreen)
			&& ((RenderMode == ESsPlayerComponentRenderMode::OffScreenPlane) || (RenderMode == ESsPlayerComponentRenderMode::OffScreenOnly))
			)
		{
			RenderOffScreen = new FSsRenderOffScreen();
		}
		if(    (NULL != RenderOffScreen)
			&& !RenderOffScreen->IsInitialized()
			)
		{
			uint32 MaxVertexNum(0), MaxIndexNum(0);
			bool bNeedMask(false);
			if(nullptr != SsProject)
			{
				SsProject->CalcMaxVertexAndIndexNum(MaxVertexNum, MaxIndexNum);
				bNeedMask = SsProject->ContainsMaskParts();
			}
			RenderOffScreen->Initialize(OffScreenRenderResolution.X, OffScreenRenderResolution.Y, MaxVertexNum, MaxIndexNum, bNeedMask);

			// OffScreenPlane用メッシュの初期化
			if((RenderMode == ESsPlayerComponentRenderMode::OffScreenPlane) && BaseMaterial)
			{
				OffScreenPlaneMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
				if(OffScreenPlaneMID)
				{
					OffScreenPlaneMID->SetFlags(RF_Transient);
					OffScreenPlaneMID->SetTextureParameterValue(FName(TEXT("SsRenderTarget")), RenderOffScreen->GetRenderTarget());
					RenderMIDs.Add(OffScreenPlaneMID);

					if(SceneProxy)
					{
						((FSsRenderPlaneProxy*)SceneProxy)->SetMaterial(OffScreenPlaneMID);
					}
				}
			}
		}
	}
}
// コンポーネントの登録解除
void USsPlayerComponent::OnUnregister()
{
	Super::OnUnregister();

	PartsMIDMaps.Empty();
	PartsMIDRef.Empty();
	MaterialReplacementMap.Empty();
	MapterialReplacementMapPerBlendMode.Empty();
	OffScreenPlaneMID = NULL;

	if(RenderOffScreen)
	{
		RenderOffScreen->ReserveTerminate();
		RenderOffScreen = NULL;
	}
}

// ゲーム開始時の初期化
void USsPlayerComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

// 更新
void USsPlayerComponent::TickComponent(float DeltaTime, enum ELevelTick /*TickType*/, FActorComponentTickFunction* /*ThisTickFunction*/)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerComponent_Tick);

#if WITH_EDITOR
	// SsProjectがReimportされたら、ActorComponentを再登録させる
	if(Player.GetSsProject().IsStale())
	{
		GetOwner()->ReregisterAllComponents();
		return;
	}
#endif

	if(bAutoUpdate)
	{
		UpdatePlayer(DeltaTime);
	}
}

void USsPlayerComponent::SendRenderDynamicData_Concurrent()
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerComponent_SetRenderDynamicData_Concurrent);

	if(NULL == SceneProxy)
	{
		return;
	}

	switch(RenderMode)
	{
		case ESsPlayerComponentRenderMode::Default:
		case ESsPlayerComponentRenderMode::Masked:
			{
				TArray<FSsRenderPartsProxy::FSsPartVertex> RenderVertices;
				TArray<uint16> RenderIndices;
				TArray<FSsRenderPartsProxy::FSsPartPrimitive> RenderPrimitives;
				{
					const TArray<FSsRenderPart>& RenderParts = Player.GetRenderParts();
					FVector2D Pivot = Player.GetAnimPivot();
					FVector2D CanvasSizeUU = (Player.GetAnimCanvasSize() * UUPerPixel);

					TArray<UMaterialInterface*> PartsMaterials;
					PartsMaterials.Reserve(RenderParts.Num());
					for(auto ItPart = RenderParts.CreateConstIterator(); ItPart; ++ItPart)
					{
						// マスクパーツ（未対応） 
						if(SsBlendType::Mask == ItPart->ColorBlendType)
						{
							PartsMaterials.Add(nullptr);
						}
						else
						{
							UMaterialInstanceDynamic** ppMID = nullptr;
							{
								UMaterialInterface** ppReplaceBaseMaterial = MaterialReplacementMap.Find(ItPart->PartIndex);
								if(nullptr == ppReplaceBaseMaterial)
								{
									ppReplaceBaseMaterial = MapterialReplacementMapPerBlendMode.Find(SS_BlendTypeKey(ItPart->AlphaBlendType, ItPart->ColorBlendType));
								}
								UMaterialInterface* PartBaseMaterial = (nullptr != ppReplaceBaseMaterial) ? *ppReplaceBaseMaterial : GetBaseMaterialComp(RenderMode, ItPart->AlphaBlendType, ItPart->ColorBlendType);
								if(nullptr != PartBaseMaterial)
								{
									TMap<UTexture*, UMaterialInstanceDynamic*>* PartsMIDMap = PartsMIDMaps.Find(PartBaseMaterial);
									if(nullptr != PartsMIDMap)
									{
										ppMID = PartsMIDMap->Find(ItPart->Texture);
									}
								}
							}
							PartsMaterials.Add((ppMID && *ppMID) ? *ppMID : nullptr);
						}
					}

					uint32 VertexCnt = 0;
					uint32 IndexCnt  = 0;

					// Vertex, Index 
					FSsRenderPartsProxy::FSsPartVertex Vertex;
					for(auto ItPart = RenderParts.CreateConstIterator(); ItPart; ++ItPart)
					{
						if(nullptr == ItPart->Texture)
						{
							continue;
						}

						// マスクパーツ（未対応） 
						if(SsBlendType::Mask == ItPart->ColorBlendType)
						{
							continue;
						}
						// 通常パーツ 
						else if(0 == ItPart->Mesh.Num())
						{
							check((4 == ItPart->Vertices.Num()) || (5 == ItPart->Vertices.Num()));
							for(int32 v = 0; v < ItPart->Vertices.Num(); ++v)
							{
								Vertex.Position = FVector(
									0.f,
									( ItPart->Vertices[v].Position.X - 0.5f - Pivot.X) * CanvasSizeUU.X,
									(-ItPart->Vertices[v].Position.Y + 0.5f - Pivot.Y) * CanvasSizeUU.Y
									);
								Vertex.TexCoord   = ItPart->Vertices[v].TexCoord;
								Vertex.Color      = ItPart->Vertices[v].Color;
								Vertex.ColorBlend = FVector2D((RenderParts.Num() - ItPart.GetIndex()) * PixelDepthOffsetPerPart, ItPart->Vertices[v].ColorBlendRate);
								RenderVertices.Add(Vertex);
							}

							if(4 == ItPart->Vertices.Num())
							{
								RenderIndices.Add(VertexCnt + 0);
								RenderIndices.Add(VertexCnt + 1);
								RenderIndices.Add(VertexCnt + 3);

								RenderIndices.Add(VertexCnt + 0);
								RenderIndices.Add(VertexCnt + 3);
								RenderIndices.Add(VertexCnt + 2);

								IndexCnt += 6;
							}
							else
							{
								RenderIndices.Add(VertexCnt + 0);
								RenderIndices.Add(VertexCnt + 1);
								RenderIndices.Add(VertexCnt + 4);

								RenderIndices.Add(VertexCnt + 1);
								RenderIndices.Add(VertexCnt + 3);
								RenderIndices.Add(VertexCnt + 4);

								RenderIndices.Add(VertexCnt + 3);
								RenderIndices.Add(VertexCnt + 2);
								RenderIndices.Add(VertexCnt + 4);

								RenderIndices.Add(VertexCnt + 2);
								RenderIndices.Add(VertexCnt + 0);
								RenderIndices.Add(VertexCnt + 4);

								IndexCnt += 12;
							}
							VertexCnt += ItPart->Vertices.Num();
						}
						// メッシュパーツ 
						else
						{
							for(auto ItMesh = ItPart->Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
							{
								for(auto ItVert = ItMesh->Vertices.CreateConstIterator(); ItVert; ++ItVert)
								{
									Vertex.Position = FVector(
										0.f,
										( ItVert->Position.X - 0.5f - Pivot.X) * CanvasSizeUU.X,
										(-ItVert->Position.Y + 0.5f - Pivot.Y) * CanvasSizeUU.Y
										);
									Vertex.TexCoord   = ItVert->TexCoord;
									Vertex.Color      = ItMesh->Color;
									Vertex.ColorBlend = FVector2D((RenderParts.Num() - ItPart.GetIndex()) * PixelDepthOffsetPerPart, ItMesh->ColorBlendRate);
									RenderVertices.Add(Vertex);
								}
								for(auto ItIndex = ItMesh->Indices.CreateConstIterator(); ItIndex; ++ItIndex)
								{
									RenderIndices.Add(VertexCnt + *ItIndex);
								}

								VertexCnt += ItMesh->Vertices.Num();
								IndexCnt  += ItMesh->Indices.Num();
							}
						}
					}

					// Primitive 
					uint32 FirstIndex = 0;
					uint32 MinVertexIndex = 0;
					VertexCnt = 0;
					IndexCnt  = 0;
					for(int32 i = 0; i < RenderParts.Num(); ++i)
					{
						if(nullptr == RenderParts[i].Texture)
						{
							continue;
						}

						// マスクパーツ（未対応） 
						if(SsBlendType::Mask == RenderParts[i].ColorBlendType)
						{
							continue;
						}
						// 通常パーツ 
						else if(0 == RenderParts[i].Mesh.Num())
						{
							check((4 == RenderParts[i].Vertices.Num()) || (5 == RenderParts[i].Vertices.Num()));
							if(4 == RenderParts[i].Vertices.Num())
							{
								VertexCnt += 4;
								IndexCnt  += 6;
							}
							else
							{
								VertexCnt += 5;
								IndexCnt  += 12;
							}
						}
						// メッシュパーツ 
						else
						{
							for(auto ItMesh = RenderParts[i].Mesh.CreateConstIterator(); ItMesh; ++ItMesh)
							{
								VertexCnt += ItMesh->Vertices.Num();
								IndexCnt  += ItMesh->Indices.Num();
							}
						}

						// 次パーツが同時に描画出来るか 
						if(    (i != (RenderParts.Num()-1))											// 最後の１つでない 
							&& (RenderParts[i].AlphaBlendType == RenderParts[i+1].AlphaBlendType)	// アルファブレンドモード
							&& (PartsMaterials[i] == PartsMaterials[i+1])							// マテリアルが一致(参照セル/カラーブレンド毎にマテリアルが別れる) 
							&& (RenderParts[i+1].ColorBlendType != SsBlendType::Mask)				// 次がマスクパーツでない
							)
						{
							continue;
						}

						FSsRenderPartsProxy::FSsPartPrimitive RenderPrimitive;
						RenderPrimitive.Material       = PartsMaterials[i];
						RenderPrimitive.AlphaBlendType = RenderParts[i].AlphaBlendType;
						RenderPrimitive.FirstIndex     = FirstIndex;
						RenderPrimitive.MinVertexIndex = MinVertexIndex;
						RenderPrimitive.MaxVertexIndex = MinVertexIndex + VertexCnt - 1;
						RenderPrimitive.NumPrimitives  = IndexCnt / 3;
						RenderPrimitives.Add(RenderPrimitive);

						FirstIndex     = FirstIndex + IndexCnt;
						MinVertexIndex = MinVertexIndex + VertexCnt;
						VertexCnt = 0;
						IndexCnt  = 0;
					}
				}

				FSsRenderPartsProxy* SsPartsProxy = (FSsRenderPartsProxy*)SceneProxy;
				ENQUEUE_RENDER_COMMAND(FSendSsRenderData)(
					[SsPartsProxy, RenderVertices, RenderIndices, RenderPrimitives](FRHICommandListImmediate& RHICmdList)
					{
						SsPartsProxy->SetDynamicData_RenderThread(RenderVertices, RenderIndices, RenderPrimitives);
					});

			} break;
		case ESsPlayerComponentRenderMode::OffScreenPlane:
			{
				FSsRenderPlaneProxy* SsPlaneProxy = (FSsRenderPlaneProxy*)SceneProxy;
				UMaterialInterface* Material = OffScreenPlaneMID;
				FVector2D Pivot(Player.GetAnimPivot());
				FVector2D CanvasSizeUU(Player.GetAnimCanvasSize() * UUPerPixel);
				ENQUEUE_RENDER_COMMAND(FSendSsPlaneData)(
					[SsPlaneProxy, Material, Pivot, CanvasSizeUU](FRHICommandListImmediate& RHICmdList)
					{
						if(Material)
						{
							SsPlaneProxy->SetMaterial(Material);
						}
						SsPlaneProxy->CanvasSizeUU = CanvasSizeUU;
						SsPlaneProxy->SetPivot(Pivot);
						SsPlaneProxy->SetDynamicData_RenderThread();
					});
			} break;
	}
}


FPrimitiveSceneProxy* USsPlayerComponent::CreateSceneProxy()
{
	if(SsProject)
	{
		switch(RenderMode)
		{
			case ESsPlayerComponentRenderMode::Default:
			case ESsPlayerComponentRenderMode::Masked:
				{
					uint32 MaxVertexNum, MaxIndexNum;
					SsProject->CalcMaxVertexAndIndexNum(MaxVertexNum, MaxIndexNum);
					FSsRenderPartsProxy* NewProxy = new FSsRenderPartsProxy(this, MaxVertexNum, MaxIndexNum);
					return NewProxy;
				} break;
			case ESsPlayerComponentRenderMode::OffScreenPlane:
				{
					FSsRenderPlaneProxy* NewProxy = new FSsRenderPlaneProxy(this, OffScreenPlaneMID);
					NewProxy->CanvasSizeUU = (Player.GetAnimCanvasSize() * UUPerPixel);
					NewProxy->SetPivot(Player.GetAnimPivot());
					return NewProxy;
				} break;
		}
	}
	return NULL;
}

FBoxSphereBounds USsPlayerComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	float LocalBoundsScale = 1.f;
	switch(RenderMode)
	{
		case ESsPlayerComponentRenderMode::Default:
		case ESsPlayerComponentRenderMode::Masked:
			{
				LocalBoundsScale = SsBoundsScale;
			} //not break
		case ESsPlayerComponentRenderMode::OffScreenPlane:
			{
				const FVector2D CanvasSizeUU = (Player.GetAnimCanvasSize() * UUPerPixel);
				const FVector2D& Pivot = Player.GetAnimPivot();
				FVector2D PivotOffSet = -(Pivot * CanvasSizeUU);

				FBox BoundsBox(EForceInit::ForceInit);
				BoundsBox += FVector(0.f, PivotOffSet.X - CanvasSizeUU.X/2.f, PivotOffSet.Y + CanvasSizeUU.Y/2.f);
				BoundsBox += FVector(0.f, PivotOffSet.X + CanvasSizeUU.X/2.f, PivotOffSet.Y + CanvasSizeUU.Y/2.f);
				BoundsBox += FVector(0.f, PivotOffSet.X - CanvasSizeUU.X/2.f, PivotOffSet.Y - CanvasSizeUU.Y/2.f);
				BoundsBox += FVector(0.f, PivotOffSet.X + CanvasSizeUU.X/2.f, PivotOffSet.Y - CanvasSizeUU.Y/2.f);

				BoundsBox.Min *= LocalBoundsScale;
				BoundsBox.Max *= LocalBoundsScale;

				return FBoxSphereBounds(BoundsBox).TransformBy(LocalToWorld);
			} break;
	}
	return FBoxSphereBounds(EForceInit::ForceInitToZero);
}

// アニメーションの更新 
void USsPlayerComponent::UpdatePlayer(float DeltaSeconds)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerComponent_UpdatePlayer);

	if(!bIsActive)
	{
		return;
	}

	//	アニメーションの再生更新
	{
		FSsPlayerTickResult Result = Player.Tick(DeltaSeconds);
		if(Result.bUpdate)
		{
			// ユーザーデータイベント
			for(int32 i = 0; i < Result.UserData.Num(); ++i)
			{
				OnSsUserData.Broadcast(
					Result.UserData[i].PartName,
					Result.UserData[i].PartIndex,
					Result.UserData[i].KeyFrame,
					Result.UserData[i].Value
					);
			}

			// 再生終了イベント
			if(Result.bEndPlay)
			{
				int32 AnimPackIndex  = Player.GetPlayingAnimPackIndex();
				int32 AnimationIndex = Player.GetPlayingAnimationIndex();
				FName AnimPackName   = Player.GetSsProject()->AnimeList[AnimPackIndex].AnimePackName;
				FName AnimationName  = Player.GetSsProject()->AnimeList[AnimPackIndex].AnimeList[AnimationIndex].AnimationName;
				OnSsEndPlay.Broadcast(
					AnimPackName, AnimationName,
					AnimPackIndex, AnimationIndex
					);
			}

			// オフスクリーンレンダリングの描画命令発行
			if(NULL != RenderOffScreen)
			{
				RenderOffScreen->ClearColor = OffScreenClearColor;
				RenderOffScreen->Render(Player.GetRenderParts());
			}
		}
	}

	switch(RenderMode)
	{
		case ESsPlayerComponentRenderMode::Default:
		case ESsPlayerComponentRenderMode::Masked:
			{
				QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerComponent_UpdatePlayer_Default);

				// パーツ描画用MIDの確保 
				const TArray<FSsRenderPart>& RenderParts = Player.GetRenderParts();
				for(int32 i = 0; i < RenderParts.Num(); ++i)
				{
					// マスクパーツ（未対応） 
					if(SsBlendType::Mask == RenderParts[i].ColorBlendType)
					{
						continue;
					}

					UMaterialInterface** ppReplaceBaseMaterial = MaterialReplacementMap.Find(RenderParts[i].PartIndex);
					if(nullptr == ppReplaceBaseMaterial)
					{
						ppReplaceBaseMaterial = MapterialReplacementMapPerBlendMode.Find(SS_BlendTypeKey(RenderParts[i].AlphaBlendType, RenderParts[i].ColorBlendType));
					}
					UMaterialInterface* PartBaseMaterial = (nullptr != ppReplaceBaseMaterial) ? *ppReplaceBaseMaterial : GetBaseMaterialComp(RenderMode, RenderParts[i].AlphaBlendType, RenderParts[i].ColorBlendType);
					if(nullptr != PartBaseMaterial)
					{
						TMap<UTexture*, UMaterialInstanceDynamic*>& PartsMIDMap = PartsMIDMaps.FindOrAdd(PartBaseMaterial);
						UMaterialInstanceDynamic** ppMID = PartsMIDMap.Find(RenderParts[i].Texture);
						if((nullptr == ppMID) || (nullptr == *ppMID))
						{
							UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(PartBaseMaterial, GetTransientPackage());
							if(NewMID)
							{
								PartsMIDRef.Add(NewMID);
								NewMID->SetFlags(RF_Transient);
								NewMID->SetTextureParameterValue(FName(TEXT("SsCellTexture")), RenderParts[i].Texture);
								PartsMIDMap.Add(RenderParts[i].Texture, NewMID);
								RenderMIDs.Add(NewMID);
							}
						}
					}
				}
			} // not break
		case ESsPlayerComponentRenderMode::OffScreenPlane:
			{
				QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerComponent_UpdatePlayer_OffScreenPlane);

				// 描画更新 
				MarkRenderDynamicDataDirty();

				// アタッチされたコンポーネントのTransform更新 
				UpdateChildTransforms();
				UpdateOverlaps();
			} break;
	}
}

// テクスチャの取得
UTexture* USsPlayerComponent::GetRenderTarget()
{
	if(NULL != RenderOffScreen)
	{
		return RenderOffScreen->GetRenderTarget();
	}
	return NULL;
}

// パーツのアタッチ用Transformを取得 
bool USsPlayerComponent::GetPartAttachTransform(int32 PartIndex, FTransform& OutTransform) const
{
	FVector2D Position, Scale;
	float Rotate;
	if(!Player.GetPartTransform(PartIndex, Position, Rotate, Scale))
	{
		OutTransform = FTransform::Identity;
		return false;
	}

	if(Player.bFlipH){ Position.X =  1.f - Position.X; }
	if(Player.bFlipV){ Position.Y = -1.f - Position.Y; }
	Position.X = (Position.X - Player.GetAnimPivot().X - 0.5f);
	Position.Y = (Position.Y - Player.GetAnimPivot().Y + 0.5f);

	FRotator R = FRotator(0.f, 0.f, Rotate) * -1.f;
	if(Player.bFlipH)
	{
		R = FRotator(0.f, 180.f, 0.f) + R;
	}
	if(Player.bFlipV)
	{
		R = FRotator(180.f, 0.f, 0.f) + R;
	}

	OutTransform = FTransform(
		R,
		FVector(
			0.f,
			Position.X * Player.GetAnimCanvasSize().X * UUPerPixel,
			Position.Y * Player.GetAnimCanvasSize().Y * UUPerPixel
			),
		FVector(1.f, Scale.X, Scale.Y)
		);
	return true;
}


#if WITH_EDITOR
void USsPlayerComponent::OnSetSsProject()
{
	SyncAutoPlayAnimation_IndexToName();
}
#endif


//
// Blueprint公開関数 
//

bool USsPlayerComponent::Play(FName AnimPackName, FName AnimationName, int32 StartFrame, float PlayRate, int32 LoopCount, bool bRoundTrip)
{
	int32 AnimPackIndex, AnimationIndex;
	if(Player.GetAnimationIndex(AnimPackName, AnimationName, AnimPackIndex, AnimationIndex))
	{
		return PlayByIndex(AnimPackIndex, AnimationIndex, StartFrame, PlayRate, LoopCount, bRoundTrip);
	}

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerComponent::Play() Invalid Animation (%s, %s)"), *(AnimPackName.ToString()), *(AnimationName.ToString()));
	return false;
}
bool USsPlayerComponent::PlayByIndex(int32 AnimPackIndex, int32 AnimationIndex, int32 StartFrame, float PlayRate, int32 LoopCount, bool bRoundTrip)
{
	if(Player.Play(AnimPackIndex, AnimationIndex, StartFrame, PlayRate, LoopCount, bRoundTrip))
	{
		UpdateBounds();

		if(bAutoUpdate)
		{
			UpdatePlayer(0.f);
		}
		return true;
	}

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerComponent::PlayByIndex() Invalid Animation index (%d, %d)"), AnimPackIndex, AnimationIndex);
	return false;
}
void USsPlayerComponent::GetPlayingAnimationName(FName& OutAnimPackName, FName& OutAnimationName) const
{
	int32 AnimPackIndex  = Player.GetPlayingAnimPackIndex();
	int32 AnimationIndex = Player.GetPlayingAnimationIndex();
	if(Player.GetSsProject().IsValid() && (0 <= AnimPackIndex) && (0 <= AnimationIndex))
	{
		if(    (AnimPackIndex < Player.GetSsProject()->AnimeList.Num())
			&& (AnimationIndex < Player.GetSsProject()->AnimeList[AnimPackIndex].AnimeList.Num())
			)
		{
			OutAnimPackName  = Player.GetSsProject()->AnimeList[AnimPackIndex].AnimePackName;
			OutAnimationName = Player.GetSsProject()->AnimeList[AnimPackIndex].AnimeList[AnimationIndex].AnimationName;
			return;
		}
	}
	OutAnimPackName  = FName();
	OutAnimationName = FName();
}
void USsPlayerComponent::GetPlayingAnimationIndex(int32& OutAnimPackIndex, int32& OutAnimationIndex) const
{
	OutAnimPackIndex  = Player.GetPlayingAnimPackIndex();
	OutAnimationIndex = Player.GetPlayingAnimationIndex();
}
void USsPlayerComponent::Pause()
{
	Player.Pause();
}
bool USsPlayerComponent::Resume()
{
	return Player.Resume();
}
bool USsPlayerComponent::IsPlaying() const
{
	return Player.IsPlaying();
}

int32 USsPlayerComponent::GetNumAnimPacks() const
{
	if(SsProject)
	{
		return SsProject->AnimeList.Num();
	}
	return 0;
}
int32 USsPlayerComponent::GetNumAnimations(FName AnimPackName) const
{
	if(SsProject)
	{
		int32 AnimPackIndex = SsProject->FindAnimePackIndex(AnimPackName);
		if(0 <= AnimPackIndex)
		{
			return SsProject->AnimeList[AnimPackIndex].AnimeList.Num();
		}
	}
	return 0;
}
int32 USsPlayerComponent::GetNumAnimationsByIndex(int32 AnimPackIndex) const
{
	if(SsProject && (AnimPackIndex < SsProject->AnimeList.Num()))
	{
		return SsProject->AnimeList[AnimPackIndex].AnimeList.Num();
	}
	return 0;
}

void USsPlayerComponent::SetPlayFrame(float Frame)
{
	Player.SetPlayFrame(Frame);
}
float USsPlayerComponent::GetPlayFrame() const
{
	return Player.GetPlayFrame();
}

void USsPlayerComponent::SetLoopCount(int32 InLoopCount)
{
	Player.LoopCount = InLoopCount;
}
int32 USsPlayerComponent::GetLoopCount() const
{
	return Player.LoopCount;
}

void USsPlayerComponent::SetRoundTrip(bool bInRoundTrip)
{
	Player.bRoundTrip = bInRoundTrip;
}
bool USsPlayerComponent::IsRoundTrip() const
{
	return Player.bRoundTrip;
}

void USsPlayerComponent::SetPlayRate(float InRate)
{
	Player.PlayRate = InRate;
}
float USsPlayerComponent::GetPlayRate() const
{
	return Player.PlayRate;
}

void USsPlayerComponent::SetFlipH(bool InFlipH)
{
	Player.bFlipH = InFlipH;
}
bool USsPlayerComponent::GetFlipH() const
{
	return Player.bFlipH;
}
void USsPlayerComponent::SetFlipV(bool InFlipV)
{
	Player.bFlipV = InFlipV;
}
bool USsPlayerComponent::GetFlipV() const
{
	return Player.bFlipV;
}

void USsPlayerComponent::AddTextureReplacement(FName PartName, UTexture* Texture)
{
	if(Texture)
	{
		int32 PartIndex = Player.GetPartIndexFromName(PartName);
		if(0 <= PartIndex)
		{
			Player.TextureReplacements.Add(PartIndex, TWeakObjectPtr<UTexture>(Texture));
		}
		else
		{
			UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerComponent::AddTextureReplacement() Invalid Part Name(%s)"), *(PartName.ToString()));
		}
	}
	else
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerComponent::AddTextureReplacement() Texture is NULL"));
	}
}
void USsPlayerComponent::AddTextureReplacementByIndex(int32 PartIndex, UTexture* Texture)
{
	if(Texture)
	{
		Player.TextureReplacements.Add(PartIndex, TWeakObjectPtr<UTexture>(Texture));
	}
	else
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerComponent::AddTextureReplacementByIndex() Texture is NULL"));
	}
}
void USsPlayerComponent::RemoveTextureReplacement(FName PartName)
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if(0 <= PartIndex)
	{
		Player.TextureReplacements.Remove(PartIndex);
	}
}
void USsPlayerComponent::RemoveTextureReplacementByIndex(int32 PartIndex)
{
	Player.TextureReplacements.Remove(PartIndex);
}
void USsPlayerComponent::RemoveTextureReplacementAll()
{
	Player.TextureReplacements.Empty();
}

void USsPlayerComponent::AddMaterialReplacement(FName PartName, UMaterialInterface* InBaseMaterial)
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if(0 <= PartIndex)
	{
		MaterialReplacementMap.Add(PartIndex, InBaseMaterial);
	}
}
void USsPlayerComponent::AddMaterialReplacementByIndex(int32 PartIndex, UMaterialInterface* InBaseMaterial)
{
	MaterialReplacementMap.Add(PartIndex, InBaseMaterial);
}
void USsPlayerComponent::RemoveMaterialReplacement(FName PartName)
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if(0 <= PartIndex)
	{
		MaterialReplacementMap.Remove(PartIndex);
	}
}
void USsPlayerComponent::RemoveMaterialReplacementByIndex(int32 PartIndex)
{
	MaterialReplacementMap.Remove(PartIndex);
}
void USsPlayerComponent::RemoveMaterialReplacementAll()
{
	MaterialReplacementMap.Empty();
}

void USsPlayerComponent::AddMaterialReplacementPerBlendMode(EAlphaBlendType AlphaBlendMode, EColorBlendType ColorBlendMode, UMaterialInterface* InBaseMaterial)
{
	MapterialReplacementMapPerBlendMode.Add(SS_BlendTypeKey(AlphaBlendMode, ColorBlendMode), InBaseMaterial);
}
void USsPlayerComponent::RemoveMaterialReplacementPerBlendMode(EAlphaBlendType AlphaBlendMode, EColorBlendType ColorBlendMode)
{
	MapterialReplacementMapPerBlendMode.Remove(SS_BlendTypeKey(AlphaBlendMode, ColorBlendMode));
}
void USsPlayerComponent::RemoveMaterialReplacementAllPerBlendMode()
{
	MapterialReplacementMapPerBlendMode.Empty();
}

FName USsPlayerComponent::GetPartColorLabel(FName PartName)
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if (0 <= PartIndex)
	{
		return Player.GetPartColorLabel(PartIndex);
	}
	return FName();
}
FName USsPlayerComponent::GetPartColorLabelByIndex(int32 PartIndex)
{
	return Player.GetPartColorLabel(PartIndex);
}

