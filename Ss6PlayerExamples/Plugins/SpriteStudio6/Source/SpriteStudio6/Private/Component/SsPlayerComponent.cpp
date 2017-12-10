#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerComponent.h"

#include "Ss6Project.h"
#include "SsAnimePack.h"
#include "SsPlayer.h"
#include "SsRenderOffScreen.h"
#include "SsRenderPlaneProxy.h"
#include "SsRenderPartsProxy.h"


namespace
{
	// BasePartsMaterials/PartsMIDMap のインデックスを取得
	inline uint32 PartsMatIndex(SsBlendType::Type ColorBlendMode)
	{
		switch(ColorBlendMode)
		{
			case SsBlendType::Mix: { return 0; }
			case SsBlendType::Mul: { return 1; }
			case SsBlendType::Add: { return 2; }
			case SsBlendType::Sub: { return 3; }
			case SsBlendType::Invalid: { return 4; }
			case SsBlendType::Effect:  { return 5; }
		}
		check(false);
		return 0;
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
	, BaseMaterial(NULL)
	, OffScreenPlaneMID(NULL)
	, OffScreenRenderResolution(512.f, 512.f)
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


	// 各種マテリアル参照の取得
	// 参照：https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Reference/Classes/index.html#assetreferences
	struct FConstructorStatics
	{
		// メッシュ用デフォルトマテリアル
		ConstructorHelpers::FObjectFinder<UMaterialInterface> MeshBase;

		// パーツ描画用 (ColorBlendMode毎) 
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartMix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartMul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartAdd;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartSub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartInv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartEffect;

		FConstructorStatics()
			: MeshBase(TEXT("/SpriteStudio6/SsMaterial_MeshDefault"))
			, PartMix(TEXT("/SpriteStudio6/PartMaterials/SsPart_Mix"))
			, PartMul(TEXT("/SpriteStudio6/PartMaterials/SsPart_Mul"))
			, PartAdd(TEXT("/SpriteStudio6/PartMaterials/SsPart_Add"))
			, PartSub(TEXT("/SpriteStudio6/PartMaterials/SsPart_Sub"))
			, PartInv(TEXT("/SpriteStudio6/PartMaterials/SsPart_Inv"))
			, PartEffect(TEXT("/SpriteStudio6/PartMaterials/SsPart_Effect"))
		{}
	};
	static FConstructorStatics CS;

	BaseMaterial = CS.MeshBase.Object;

	BasePartsMaterials[0] = CS.PartMix.Object;
	BasePartsMaterials[1] = CS.PartMul.Object;
	BasePartsMaterials[2] = CS.PartAdd.Object;
	BasePartsMaterials[3] = CS.PartSub.Object;
	BasePartsMaterials[4] = CS.PartInv.Object;
	BasePartsMaterials[5] = CS.PartEffect.Object;
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
	if(Player.GetSsProject().IsValid())
	{
		int32 AnimPackIndex  = Player.GetPlayingAnimPackIndex();
		int32 AnimationIndex = Player.GetPlayingAnimationIndex();
		if((0 <= AnimPackIndex) && (0 <= AnimationIndex))
		{
			FSsAnimation& Animation = Player.GetSsProject()->AnimeList[AnimPackIndex].AnimeList[AnimationIndex];
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
		&& (Player.GetSsProject().IsValid())
		)
	{
		int32 AnimPackIndex  = Player.GetPlayingAnimPackIndex();
		int32 AnimationIndex = Player.GetPlayingAnimationIndex();
		if((0 <= AnimPackIndex) && (0 <= AnimationIndex))
		{
			FSsAnimation& Animation = Player.GetSsProject()->AnimeList[AnimPackIndex].AnimeList[AnimationIndex];
			for(int32 i = 0; i < Animation.PartAnimes.Num(); ++i)
			{
				OutSockets.Add(
					FComponentSocketDescription(
						Animation.PartAnimes[i].PartName,
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
			RenderOffScreen->Initialize(OffScreenRenderResolution.X, OffScreenRenderResolution.Y, SsProject->CalcMaxRenderPartsNum());

			// OffScreenPlane用メッシュの初期化
			if((RenderMode == ESsPlayerComponentRenderMode::OffScreenPlane) && BaseMaterial)
			{
				OffScreenPlaneMID = UMaterialInstanceDynamic::Create(BaseMaterial, this);
				if(OffScreenPlaneMID)
				{
					OffScreenPlaneMID->SetFlags(RF_Transient);
					OffScreenPlaneMID->SetTextureParameterValue(FName(TEXT("SsRenderTarget")), RenderOffScreen->GetRenderTarget());

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

	for(int32 i = 0; i < 6; ++i)
	{
		for(auto It = PartsMIDMap[i].CreateIterator(); It; ++It)
		{
			(*It).Value->RemoveFromRoot();
		}
		PartsMIDMap[i].Empty();
	}
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
	if(NULL == SceneProxy)
	{
		return;
	}

	switch(RenderMode)
	{
		case ESsPlayerComponentRenderMode::Default:
			{
				const TArray<FSsRenderPart> RenderParts = Player.GetRenderParts();
				TArray<FSsRenderPartWithMaterial> NewRenderParts;
				NewRenderParts.Reserve(RenderParts.Num());
				for(int32 i = 0; i < RenderParts.Num(); ++i)
				{
					FSsRenderPartWithMaterial Part;
					FMemory::Memcpy(&Part, &(RenderParts[i]), sizeof(FSsRenderPart));

					uint32 MatIdx = PartsMatIndex(Part.ColorBlendType);
					UMaterialInstanceDynamic** ppMID = PartsMIDMap[MatIdx].Find(Part.Texture);
					
					Part.Material = (ppMID && *ppMID) ? *ppMID : NULL;

					NewRenderParts.Add(Part);
				}

				ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
					FSendSsPartsData,
					FSsRenderPartsProxy*, SsPartsProxy, (FSsRenderPartsProxy*)SceneProxy,
					TArray<FSsRenderPartWithMaterial>, InRenderParts, NewRenderParts,
					FVector2D, Pivot, Player.GetAnimPivot(),
					FVector2D, CanvasSizeUU, (Player.GetAnimCanvasSize() * UUPerPixel),
				{
					SsPartsProxy->CanvasSizeUU = CanvasSizeUU;
					SsPartsProxy->SetPivot(Pivot);
					SsPartsProxy->SetDynamicData_RenderThread(InRenderParts);
				});
			} break;
		case ESsPlayerComponentRenderMode::OffScreenPlane:
			{
				ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
					FSendSsPlaneData,
					FSsRenderPlaneProxy*, SsPlaneProxy, (FSsRenderPlaneProxy*)SceneProxy,
					UMaterialInterface*,  Material, OffScreenPlaneMID,
					FVector2D, Pivot, Player.GetAnimPivot(),
					FVector2D, CanvasSizeUU, (Player.GetAnimCanvasSize() * UUPerPixel),
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
				{
					FSsRenderPartsProxy* NewProxy = new FSsRenderPartsProxy(this, SsProject->CalcMaxRenderPartsNum());
					return NewProxy;
				} break;
			case ESsPlayerComponentRenderMode::OffScreenPlane:
				{
					FSsRenderPlaneProxy* NewProxy = new FSsRenderPlaneProxy(this, OffScreenPlaneMID);
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
				RenderOffScreen->Render(Player.GetRenderParts());
			}
		}
	}

	switch(RenderMode)
	{
		case ESsPlayerComponentRenderMode::Default:
			{
				// パーツ描画用MIDの確保 
				const TArray<FSsRenderPart> RenderParts = Player.GetRenderParts();
				for(int32 i = 0; i < RenderParts.Num(); ++i)
				{
					uint32 MatIdx = PartsMatIndex(RenderParts[i].ColorBlendType);
					UMaterialInstanceDynamic** ppMID = PartsMIDMap[MatIdx].Find(RenderParts[i].Texture);
					if((NULL == ppMID) || (NULL == *ppMID))
					{
						UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(BasePartsMaterials[MatIdx], GetTransientPackage());
						if(NewMID)
						{
							NewMID->AddToRoot();
							NewMID->SetFlags(RF_Transient);
							NewMID->SetTextureParameterValue(FName(TEXT("SsCellTexture")), RenderParts[i].Texture);
							PartsMIDMap[MatIdx].Add(RenderParts[i].Texture, NewMID);
						}
					}
				}
			} // not break
		case ESsPlayerComponentRenderMode::OffScreenPlane:
			{
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

void USsPlayerComponent::RenderToCanvas(UCanvas* Canvas, FVector2D Location, float Rotation, FVector2D Scale)
{
	AHUD* HUD = Cast<AHUD>(this->GetOwner());
	if(NULL == HUD)
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerComponent::RenderToCanvas() This Component's Owner is not HUD."));
		return;
	}

	float R = FMath::DegreesToRadians(Rotation);
	float SinR = FMath::Sin(R);
	float CosR = FMath::Cos(R);

	const FVector2D& AnimPivot = Player.GetAnimPivot();

	const TArray<FSsRenderPart>& RenderParts = Player.GetRenderParts();
	TArray<FCanvasUVTri> TriList;
	for(int32 i = 0; i < RenderParts.Num(); ++i)
	{
		const FSsRenderPart& Part = RenderParts[i];

		FVector2D Vertices[4];
		for(int32 v = 0; v < 4; ++v)
		{
			Vertices[v] = FVector2D(
				(Part.Vertices[v].Position.X - AnimPivot.X - 0.5f) * Player.GetAnimCanvasSize().X,
				(Part.Vertices[v].Position.Y + AnimPivot.Y - 0.5f) * Player.GetAnimCanvasSize().Y
				);
			Vertices[v] = FVector2D(
				Vertices[v].X * CosR - Vertices[v].Y * SinR,
				Vertices[v].X * SinR + Vertices[v].Y * CosR
				);
			Vertices[v].X *= Scale.X;
			Vertices[v].Y *= Scale.Y;
			Vertices[v].X += Location.X;
			Vertices[v].Y += Location.Y;
		}

		{
			FCanvasUVTri Tri;
			Tri.V0_Pos = Vertices[0];
			Tri.V1_Pos = Vertices[1];
			Tri.V2_Pos = Vertices[3];
			Tri.V0_UV = Part.Vertices[0].TexCoord;
			Tri.V1_UV = Part.Vertices[1].TexCoord;
			Tri.V2_UV = Part.Vertices[3].TexCoord;
			Tri.V0_Color = Tri.V1_Color = Tri.V2_Color = FLinearColor::White;
			TriList.Add(Tri);
		}
		{
			FCanvasUVTri Tri;
			Tri.V0_Pos = Vertices[0];
			Tri.V1_Pos = Vertices[3];
			Tri.V2_Pos = Vertices[2];
			Tri.V0_UV = Part.Vertices[0].TexCoord;
			Tri.V1_UV = Part.Vertices[3].TexCoord;
			Tri.V2_UV = Part.Vertices[2].TexCoord;
			Tri.V0_Color = Tri.V1_Color = Tri.V2_Color = FLinearColor::White;
			TriList.Add(Tri);
		}

		// １回のレンダリングにまとめられる場合はまとめる 
		if((i != (RenderParts.Num()-1)) && (Part.Texture == RenderParts[i+1].Texture))
		{
			continue;
		}

		FCanvasTriangleItem TriItem(TriList, Part.Texture->Resource);
		TriItem.BlendMode = SE_BLEND_AlphaBlend;
		Canvas->DrawItem(TriItem);
		TriList.Empty();
	}
}
