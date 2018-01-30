#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerWidget.h"

#include "SlateMaterialBrush.h"

#include "SSsPlayerWidget.h"
#include "SsPlayerSlot.h"
#include "Ss6Project.h"
#include "SsRenderOffScreen.h"


namespace
{
	// BasePartsMaterials/PartsMIDMap のインデックスを取得
	inline uint32 UMGMatIndex(SsBlendType::Type ColorBlendMode)
	{
		switch (ColorBlendMode)
		{
			case SsBlendType::Mix: { return 0; }
			case SsBlendType::Mul: { return 1; }
			case SsBlendType::Add: { return 2; }
			case SsBlendType::Sub: { return 3; }
			case SsBlendType::Invalid:   { return 4; }
			case SsBlendType::Effect:    { return 5; }
			case SsBlendType::MixVertex: { return 6; }
		}
		check(false);
		return 0;
	}
};


// コンストラクタ 
USsPlayerWidget::USsPlayerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FSsPlayPropertySync(&SsProject, &AutoPlayAnimPackName, &AutoPlayAnimationName, &AutoPlayAnimPackIndex, &AutoPlayAnimationIndex)
	, OffScreenMID(nullptr)
#if WITH_EDITOR
	, BackWorldTime(-1.f)
#endif
	, SsProject(NULL)
	, bAutoUpdate(true)
	, bAutoPlay(true)
	, AutoPlayStartFrame(0)
	, AutoPlayRate(1.f)
	, AutoPlayLoopCount(0)
	, bAutoPlayRoundTrip(false)
	, bDontUpdateIfHidden(false)
	, bTickableWhenPaused(true)
	, RenderMode(ESsPlayerWidgetRenderMode::UMG_Default)
	, bIgnoreClipRect(false)
	, BaseMaterial(nullptr)
	, OffScreenRenderResolution(512, 512)
	, bReflectParentAlpha(false)
{
	this->Clipping = EWidgetClipping::ClipToBounds;

	Player.SetCalcHideParts(true);

	// 各種マテリアル参照の取得
	// 参照：https://docs.unrealengine.com/latest/INT/Programming/UnrealArchitecture/Reference/Classes/index.html#assetreferences
	struct FConstructorStatics
	{
		// オフスクリーン用デフォルトマテリアル 
		ConstructorHelpers::FObjectFinder<UMaterialInterface> OffScreenBase;

		// パーツ描画用 (ColorBlendMode毎) 
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartMix;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartMul;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartAdd;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartSub;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartInv;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartEffect;
		ConstructorHelpers::FObjectFinder<UMaterialInterface> PartMixVertex;

		FConstructorStatics()
			: OffScreenBase(TEXT("/SpriteStudio6/SsMaterial_UMGDefault"))
			, PartMix(TEXT("/SpriteStudio6/UMGMaterials/SsUMG_Mix"))
			, PartMul(TEXT("/SpriteStudio6/UMGMaterials/SsUMG_Mul"))
			, PartAdd(TEXT("/SpriteStudio6/UMGMaterials/SsUMG_Add"))
			, PartSub(TEXT("/SpriteStudio6/UMGMaterials/SsUMG_Sub"))
			, PartInv(TEXT("/SpriteStudio6/UMGMaterials/SsUMG_Inv"))
			, PartEffect(TEXT("/SpriteStudio6/UMGMaterials/SsUMG_Effect"))
			, PartMixVertex(TEXT("/SpriteStudio6/UMGMaterials/SsUMG_MixVertex"))
		{}
	};
	static FConstructorStatics CS;

	BaseMaterial = CS.OffScreenBase.Object;

	BasePartsMaterials[0] = CS.PartMix.Object;
	BasePartsMaterials[1] = CS.PartMul.Object;
	BasePartsMaterials[2] = CS.PartAdd.Object;
	BasePartsMaterials[3] = CS.PartSub.Object;
	BasePartsMaterials[4] = CS.PartInv.Object;
	BasePartsMaterials[5] = CS.PartEffect.Object;
	BasePartsMaterials[6] = CS.PartMixVertex.Object;
}

// Destroy 
void USsPlayerWidget::BeginDestroy()
{
	Super::BeginDestroy();
	BrushMap.Empty();	// ココで先に参照を切っておく. Brush -> MID の順で開放されるように 
}

// シリアライズ 
void USsPlayerWidget::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);
	FSsPlayPropertySync::OnSerialize(Ar);
}

#if WITH_EDITOR
// プロパティ編集イベント 
void USsPlayerWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	FSsPlayPropertySync::OnPostEditChangeProperty(PropertyChangedEvent);
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.Property)
	{
	
		if(0 == PropertyChangedEvent.Property->GetNameCPP().Compare(TEXT("bIgnoreClipRect")))
		{
			if(PlayerWidget.IsValid())
			{
				PlayerWidget->bIgnoreClipRect = bIgnoreClipRect;
			}
		}
		if(0 == PropertyChangedEvent.Property->GetNameCPP().Compare(TEXT("bReflectParentAlpha")))
		{
			if(PlayerWidget.IsValid())
			{
				PlayerWidget->bReflectParentAlpha = bReflectParentAlpha;
			}
		}
	}
}
#endif

// Wigetプロパティ同期 
void USsPlayerWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if(SsProject)
	{
		// Playerの初期化 
		if(Player.GetSsProject().Get() != SsProject)
		{
			Player.SetSsProject(SsProject);
		}

		// 自動再生 
		if(bAutoPlay && (0 <= AutoPlayAnimPackIndex) && (0 <= AutoPlayAnimationIndex))
		{
			Player.Play(AutoPlayAnimPackIndex, AutoPlayAnimationIndex, AutoPlayStartFrame, AutoPlayRate, AutoPlayLoopCount, bAutoPlayRoundTrip);
			for(auto It = Slots.CreateConstIterator(); It; ++It)
			{
				USsPlayerSlot* PlayerSlot = Cast<USsPlayerSlot>(*It);
				PlayerSlot->SetupSlateWidget(Player.GetPartIndexFromName(PlayerSlot->PartName));
			}
		}
	}

	if(PlayerWidget.IsValid())
	{
		PlayerWidget->bIgnoreClipRect = bIgnoreClipRect;
		PlayerWidget->bReflectParentAlpha = bReflectParentAlpha;

		switch(RenderMode)
		{
			case ESsPlayerWidgetRenderMode::UMG_Default:
				{
					PlayerWidget->Initialize_Default();
				} break;
			case ESsPlayerWidgetRenderMode::UMG_OffScreen:
				{
					if(BaseMaterial)
					{
						uint32 MaxVertexNum, MaxIndexNum;
						SsProject->CalcMaxVertexAndIndexNum(MaxVertexNum, MaxIndexNum);
						PlayerWidget->Initialize_OffScreen(
							OffScreenRenderResolution.X, OffScreenRenderResolution.Y,
							MaxVertexNum, MaxIndexNum
							);
						OffScreenRenderTarget = PlayerWidget->GetRenderTarget();
					}
				} break;
		}
	}
}

// 更新 
void USsPlayerWidget::Tick(float DeltaTime)
{
#if WITH_EDITOR
	// SsProjectがReimportされたら、再初期化する 
	if(Player.GetSsProject().IsStale())
	{
		SynchronizeProperties();
	}
#endif

	if(bAutoUpdate)
	{
		bool bUpdate = true;
		if(bDontUpdateIfHidden)
		{
			// ウィジェットが非表示の場合はアニメーションを更新しない 
			for(UPanelWidget* Widget = this; nullptr != Widget; Widget = Widget->GetParent())
			{
				if(!Widget->IsVisible())
				{
					bUpdate = false;
					break;
				}
			}
		}
		if(bUpdate)
		{
			UpdatePlayer(DeltaTime);
		}
	}
}

//
void USsPlayerWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	PlayerWidget.Reset();
}

//
TSharedRef<SWidget> USsPlayerWidget::RebuildWidget()
{
	PlayerWidget = SNew(SSsPlayerWidget);
	PlayerWidget->bIgnoreClipRect = bIgnoreClipRect;
	PlayerWidget->bReflectParentAlpha = bReflectParentAlpha;

	for(auto It = Slots.CreateConstIterator(); It; ++It)
	{
		USsPlayerSlot* PlayerSlot = Cast<USsPlayerSlot>(*It);
		PlayerSlot->Parent = this;
		PlayerSlot->BuildSlot(PlayerWidget.ToSharedRef());
	}

	BrushMap.Empty();
	OffScreenMID = nullptr;
	OffScreenRenderTarget = nullptr;

	return PlayerWidget.ToSharedRef();
}

//
UClass* USsPlayerWidget::GetSlotClass() const
{
	return USsPlayerSlot::StaticClass();
}
void USsPlayerWidget::OnSlotAdded(UPanelSlot* InSlot)
{
	if(PlayerWidget.IsValid())
	{
		USsPlayerSlot* PlayerSlot = Cast<USsPlayerSlot>(InSlot);
		PlayerSlot->BuildSlot(PlayerWidget.ToSharedRef());
	}
}
void USsPlayerWidget::OnSlotRemoved(UPanelSlot* InSlot)
{
	if(PlayerWidget.IsValid())
	{
		TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if(Widget.IsValid())
		{
			PlayerWidget->RemoveSlot(Widget.ToSharedRef());
		}
	}
}


//
// Blueprint公開関数 
//

void USsPlayerWidget::UpdatePlayer(float DeltaSeconds)
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
	}


	if(PlayerWidget.IsValid())
	{
		switch(RenderMode)
		{
			case ESsPlayerWidgetRenderMode::UMG_Default:
				{
					TArray<FSsRenderPartWithSlateBrush> RenderPartWithSlateBrush;
					const TArray<FSsRenderPart> RenderParts = Player.GetRenderParts();
					RenderPartWithSlateBrush.Reserve(RenderParts.Num());

					for(int32 i = 0; i < RenderParts.Num(); ++i)
					{
						FSsRenderPartWithSlateBrush Part;
						FMemory::Memcpy(&Part, &(RenderParts[i]), sizeof(FSsRenderPart));

						uint32 MatIdx = UMGMatIndex(RenderParts[i].ColorBlendType);
						UMaterialInstanceDynamic** ppMID = PartsMIDMap[MatIdx].Find(RenderParts[i].Texture);
						if((NULL == ppMID) || (NULL == *ppMID))
						{
							if(nullptr != RenderParts[i].Texture)
							{
								UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(BasePartsMaterials[MatIdx], GetTransientPackage());
								if(NewMID)
								{
									PartsMIDRef.Add(NewMID);
									NewMID->SetFlags(RF_Transient);
									NewMID->SetTextureParameterValue(FName(TEXT("SsCellTexture")), RenderParts[i].Texture);
									ppMID = &(PartsMIDMap[MatIdx].Add(RenderParts[i].Texture, NewMID));
								}
							}
						}

						if(nullptr != ppMID)
						{
							TSharedPtr<FSlateMaterialBrush>* pBrush = BrushMap.Find(*ppMID);
							if(pBrush)
							{
								Part.Brush = *pBrush;
							}
							else
							{
								if(nullptr != RenderParts[i].Texture)
								{
									Part.Brush = MakeShareable(new FSlateMaterialBrush(**ppMID, FVector2D(64, 64)));
									BrushMap.Add(*ppMID, Part.Brush);
								}
							}
						}

						RenderPartWithSlateBrush.Add(Part);
					}

					PlayerWidget->SetRenderParts_Default(RenderPartWithSlateBrush);
					PlayerWidget->SetAnimCanvasSize(Player.GetAnimCanvasSize());
				} break;

			case ESsPlayerWidgetRenderMode::UMG_OffScreen:
				{
					if(nullptr == PlayerWidget->GetRenderOffScreen())
					{
						break;
					}
					if(nullptr == OffScreenMID)
					{
						OffScreenMID = UMaterialInstanceDynamic::Create(BaseMaterial, GetTransientPackage());
						OffScreenMID->SetFlags(RF_Transient);
						OffScreenMID->SetTextureParameterValue(FName(TEXT("SsRenderTarget")), PlayerWidget->GetRenderOffScreen()->GetRenderTarget());
					}

					TSharedPtr<FSlateMaterialBrush>* Brush = BrushMap.Find(OffScreenMID);
					if(nullptr == Brush)
					{
						BrushMap.Add(
							OffScreenMID,
							MakeShareable(new FSlateMaterialBrush(*OffScreenMID, FVector2D(64, 64)))
							);
						Brush = BrushMap.Find(OffScreenMID);
					}

					PlayerWidget->SetRenderParts_OffScreen(Player.GetRenderParts(), *Brush);
					PlayerWidget->SetAnimCanvasSize(Player.GetAnimCanvasSize());
				} break;
		}
	}
}

UTexture* USsPlayerWidget::GetRenderTarget()
{
	if(PlayerWidget.IsValid())
	{
		PlayerWidget->GetRenderTarget();
	}
	return nullptr;
}

bool USsPlayerWidget::Play(FName AnimPackName, FName AnimationName, int32 StartFrame, float PlayRate, int32 LoopCount, bool bRoundTrip)
{
	int32 AnimPackIndex, AnimationIndex;
	if(Player.GetAnimationIndex(AnimPackName, AnimationName, AnimPackIndex, AnimationIndex))
	{
		return PlayByIndex(AnimPackIndex, AnimationIndex, StartFrame, PlayRate, LoopCount, bRoundTrip);
	}

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerWidget2::Play() Invalid Animation (%s, %s)"), *(AnimPackName.ToString()), *(AnimationName.ToString()));
	return false;
}
bool USsPlayerWidget::PlayByIndex(int32 AnimPackIndex, int32 AnimationIndex, int32 StartFrame, float PlayRate, int32 LoopCount, bool bRoundTrip)
{
	if(Player.Play(AnimPackIndex, AnimationIndex, StartFrame, PlayRate, LoopCount, bRoundTrip))
	{
		for(auto It = Slots.CreateConstIterator(); It; ++It)
		{
			USsPlayerSlot* PlayerSlot = Cast<USsPlayerSlot>(*It);
			PlayerSlot->SetupSlateWidget(Player.GetPartIndexFromName(PlayerSlot->PartName));
		}

		if(bAutoUpdate)
		{
			UpdatePlayer(0.f);
		}
		return true;
	}

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerWidget2::PlayByIndex() Invalid Animation index (%d, %d)"), AnimPackIndex, AnimationIndex);
	return false;
}
void USsPlayerWidget::GetPlayingAnimationName(FName& OutAnimPackName, FName& OutAnimationName) const
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
void USsPlayerWidget::GetPlayingAnimationIndex(int32& OutAnimPackIndex, int32& OutAnimationIndex) const
{
	OutAnimPackIndex  = Player.GetPlayingAnimPackIndex();
	OutAnimationIndex = Player.GetPlayingAnimationIndex();
}
void USsPlayerWidget::Pause()
{
	Player.Pause();
}
bool USsPlayerWidget::Resume()
{
	return Player.Resume();
}
bool USsPlayerWidget::IsPlaying() const
{
	return Player.IsPlaying();
}

int32 USsPlayerWidget::GetNumAnimPacks() const
{
	if(SsProject)
	{
		return SsProject->AnimeList.Num();
	}
	return 0;
}
int32 USsPlayerWidget::GetNumAnimations(FName AnimPackName) const
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
int32 USsPlayerWidget::GetNumAnimationsByIndex(int32 AnimPackIndex) const
{
	if(SsProject && (AnimPackIndex < SsProject->AnimeList.Num()))
	{
		return SsProject->AnimeList[AnimPackIndex].AnimeList.Num();
	}
	return 0;
}

void USsPlayerWidget::SetPlayFrame(float Frame)
{
	Player.SetPlayFrame(Frame);
}
float USsPlayerWidget::GetPlayFrame() const
{
	return Player.GetPlayFrame();
}

void USsPlayerWidget::SetLoopCount(int32 InLoopCount)
{
	Player.LoopCount = InLoopCount;
}
int32 USsPlayerWidget::GetLoopCount() const
{
	return Player.LoopCount;
}

void USsPlayerWidget::SetRoundTrip(bool bInRoundTrip)
{
	Player.bRoundTrip = bInRoundTrip;
}
bool USsPlayerWidget::IsRoundTrip() const
{
	return Player.bRoundTrip;
}

void USsPlayerWidget::SetPlayRate(float InRate)
{
	Player.PlayRate = InRate;
}
float USsPlayerWidget::GetPlayRate() const
{
	return Player.PlayRate;
}

void USsPlayerWidget::SetFlipH(bool InFlipH)
{
	Player.bFlipH = InFlipH;
}
bool USsPlayerWidget::GetFlipH() const
{
	return Player.bFlipH;
}
void USsPlayerWidget::SetFlipV(bool InFlipV)
{
	Player.bFlipV = InFlipV;
}
bool USsPlayerWidget::GetFlipV() const
{
	return Player.bFlipV;
}

void USsPlayerWidget::AddTextureReplacement(FName PartName, UTexture* Texture)
{
	if(Texture)
	{
		int32 PartIndex = Player.GetPartIndexFromName(PartName);
		if(0 <= PartIndex)
		{
			Player.TextureReplacements.Add(PartIndex, TWeakObjectPtr<UTexture>(Texture));
		}
	}
}
void USsPlayerWidget::AddTextureReplacementByIndex(int32 PartIndex, UTexture* Texture)
{
	Player.TextureReplacements.Add(PartIndex, TWeakObjectPtr<UTexture>(Texture));
}
void USsPlayerWidget::RemoveTextureReplacement(FName PartName)
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if(0 <= PartIndex)
	{
		Player.TextureReplacements.Remove(PartIndex);
	}
}
void USsPlayerWidget::RemoveTextureReplacementByIndex(int32 PartIndex)
{
	Player.TextureReplacements.Remove(PartIndex);
}
void USsPlayerWidget::RemoveTextureReplacementAll()
{
	Player.TextureReplacements.Empty();
}

FName USsPlayerWidget::GetPartColorLabel(FName PartName)
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if (0 <= PartIndex)
	{
		return Player.GetPartColorLabel(PartIndex);
	}
	return FName();
}
FName USsPlayerWidget::GetPartColorLabelByIndex(int32 PartIndex)
{
	return Player.GetPartColorLabel(PartIndex);
}

bool USsPlayerWidget::GetPartTransform(FName PartName, FVector2D& OutPosition, float& OutAngle, FVector2D& OutScale) const
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if(0 <= PartIndex)
	{
		return GetPartTransformByIndex(PartIndex, OutPosition, OutAngle, OutScale);
	}
	return false;
}
bool USsPlayerWidget::GetPartTransformByIndex(int32 PartIndex, FVector2D& OutPosition, float& OutAngle, FVector2D& OutScale) const
{
	if(!Player.GetPartTransform(PartIndex, OutPosition, OutAngle, OutScale))
	{
		return false;
	}

	// UMGの座標系に合わせる 
	OutPosition.Y *= -1.f;
	OutAngle *= -1.f;
	if(Player.bFlipH)
	{
		OutPosition.X = 1.f - OutPosition.X;
		OutAngle *= -1.f;
	}
	if(Player.bFlipV)
	{
		OutPosition.Y = 1.f - OutPosition.Y;
		OutAngle *= -1.f;
	}


	return true;
}
