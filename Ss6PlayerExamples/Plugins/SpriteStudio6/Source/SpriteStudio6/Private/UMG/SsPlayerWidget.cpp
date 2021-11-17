#include "SsPlayerWidget.h"

#include "SlateMaterialBrush.h"

#include "SsGameSettings.h"
#include "SSsPlayerWidget.h"
#include "SsPlayerSlot.h"
#include "Ss6Project.h"
#include "SsRenderOffScreen.h"


namespace
{
	UMaterialInterface* GetBaseMaterialUMGInternal(const FSsColorBlendModeMaterials& Mats, SsBlendType::Type ColorBlendMode)
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
	UMaterialInterface* GetBaseMaterialUMG(ESsPlayerWidgetRenderMode::Type RenderMode, SsBlendType::Type AlphaBlendMode, SsBlendType::Type ColorBlendMode)
	{
		switch(RenderMode)
		{
			case ESsPlayerWidgetRenderMode::UMG_Default:
				{
					switch(AlphaBlendMode)
					{
						case SsBlendType::Mix:       return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Default.Mix,       ColorBlendMode);
						case SsBlendType::Mul:       return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Default.Mul,       ColorBlendMode);
						case SsBlendType::Add:       return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Default.Add,       ColorBlendMode);
						case SsBlendType::Sub:       return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Default.Sub,       ColorBlendMode);
						case SsBlendType::MulAlpha:  return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Default.MulAlpha,  ColorBlendMode);
						case SsBlendType::Screen:    return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Default.Screen,    ColorBlendMode);
						case SsBlendType::Exclusion: return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Default.Exclusion, ColorBlendMode);
						case SsBlendType::Invert:    return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Default.Invert,    ColorBlendMode);
						case SsBlendType::Invalid:   return nullptr;
					}
				} break;
			case ESsPlayerWidgetRenderMode::UMG_Masked:
				{
					switch(AlphaBlendMode)
					{
						case SsBlendType::Mix:       return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Masked.Mix,       ColorBlendMode);
						case SsBlendType::Mul:       return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Masked.Mul,       ColorBlendMode);
						case SsBlendType::Add:       return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Masked.Add,       ColorBlendMode);
						case SsBlendType::Sub:       return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Masked.Sub,       ColorBlendMode);
						case SsBlendType::MulAlpha:  return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Masked.MulAlpha,  ColorBlendMode);
						case SsBlendType::Screen:    return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Masked.Screen,    ColorBlendMode);
						case SsBlendType::Exclusion: return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Masked.Exclusion, ColorBlendMode);
						case SsBlendType::Invert:    return GetBaseMaterialUMGInternal(GetDefault<USsGameSettings>()->UMG_Masked.Invert,    ColorBlendMode);
						case SsBlendType::Invalid:   return nullptr;
					}
				} break;
		}
		check(false);
		return nullptr;
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
	, BaseMaterial(nullptr)
	, OffScreenRenderResolution(512, 512)
	, OffScreenClearColor(0, 0, 0, 0)
	, bReflectParentAlpha(false)
{
	this->Clipping = EWidgetClipping::ClipToBounds;

	Player.SetCalcHideParts(true);

	BaseMaterial = GetDefault<USsGameSettings>()->UMG_OffScreen;
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
#if WITH_EDITOR
		// Reimportでインデックスが変わった場合に即座に反映 
		SyncAutoPlayAnimation_NameToIndex();
#endif

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
		PlayerWidget->bReflectParentAlpha = bReflectParentAlpha;

		switch(RenderMode)
		{
			case ESsPlayerWidgetRenderMode::UMG_Default:
			case ESsPlayerWidgetRenderMode::UMG_Masked:
				{
					PlayerWidget->Initialize_Default();
				} break;
			case ESsPlayerWidgetRenderMode::UMG_OffScreen:
				{
					if(BaseMaterial)
					{
						uint32 MaxVertexNum(0), MaxIndexNum(0);
						bool bNeedMask(false);
						if(nullptr != SsProject)
						{
							SsProject->CalcMaxVertexAndIndexNum(MaxVertexNum, MaxIndexNum);
							bNeedMask = SsProject->ContainsMaskParts();
						}
						PlayerWidget->Initialize_OffScreen(
							OffScreenRenderResolution.X, OffScreenRenderResolution.Y,
							MaxVertexNum, MaxIndexNum,
							bNeedMask
							);
						OffScreenRenderTarget = PlayerWidget->GetRenderTarget();
					}
				} break;
		}
	}
}

// 更新 
void USsPlayerWidget::OnSlateTick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerWidget_Tick);

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
	PlayerWidget->bReflectParentAlpha = bReflectParentAlpha;
	PlayerWidget->OnSlateTick.BindUObject(this, &USsPlayerWidget::OnSlateTick);

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
	QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerWidget_UpdatePlayer);

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
			FName AnimPackName   = SsProject->AnimeList[AnimPackIndex].AnimePackName;
			FName AnimationName  = SsProject->AnimeList[AnimPackIndex].AnimeList[AnimationIndex].AnimationName;
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
			case ESsPlayerWidgetRenderMode::UMG_Masked:
				{
					QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerWidget_UpdatePlayer_UMG_Default);

					PlayerWidget->RenderParts = &Player.GetRenderParts();
					const TArray<FSsRenderPart>& RenderParts = *PlayerWidget->RenderParts;
					PlayerWidget->DefaultBrush.Reset(RenderParts.Num());

					for(int32 i = 0; i < RenderParts.Num(); ++i)
					{
						TSharedPtr<FSlateMaterialBrush> Brush;

						if(SsBlendType::Mask != RenderParts[i].ColorBlendType)
						{
							UMaterialInstanceDynamic** ppMID = nullptr;
							{
								UMaterialInterface** ppReplaceBaseMaterial = MaterialReplacementMap.Find(RenderParts[i].PartIndex);
								if(nullptr == ppReplaceBaseMaterial)
								{
									ppReplaceBaseMaterial = MaterialReplacementMapPerBlendMode.Find(SS_BlendTypeKey(RenderParts[i].AlphaBlendType, RenderParts[i].ColorBlendType));
								}
								UMaterialInterface* PartBaseMaterial = (nullptr != ppReplaceBaseMaterial) ? *ppReplaceBaseMaterial : GetBaseMaterialUMG(RenderMode, RenderParts[i].AlphaBlendType, RenderParts[i].ColorBlendType);
								if(nullptr != PartBaseMaterial)
								{
									TMap<UTexture*, UMaterialInstanceDynamic*>& PartsMIDMap = PartsMIDMaps.FindOrAdd(PartBaseMaterial);
									ppMID = PartsMIDMap.Find(RenderParts[i].Texture);
									if((nullptr == ppMID) || (nullptr == *ppMID))
									{
										UMaterialInstanceDynamic* NewMID = UMaterialInstanceDynamic::Create(PartBaseMaterial, GetTransientPackage());
										if(NewMID)
										{
											NewMID->SetFlags(RF_Transient);
											NewMID->SetTextureParameterValue(FName(TEXT("SsCellTexture")), RenderParts[i].Texture);
											ppMID = &PartsMIDMap.Add(RenderParts[i].Texture, NewMID);
											RenderMIDs.Add(NewMID);
										}
									}
								}
							}

							if(nullptr != ppMID)
							{
								TSharedPtr<FSlateMaterialBrush>* pBrush = BrushMap.Find(*ppMID);
								if(pBrush)
								{
									Brush = *pBrush;
								}
								else
								{
									if(nullptr != RenderParts[i].Texture)
									{
										Brush = MakeShareable(new FSlateMaterialBrush(**ppMID, FVector2D(64, 64)));
										BrushMap.Add(*ppMID, Brush);
									}
								}
							}
						}

						PlayerWidget->DefaultBrush.Add(Brush);
					}

					PlayerWidget->SetAnimCanvasSize(Player.GetAnimCanvasSize());
					check(PlayerWidget->RenderParts->Num() == PlayerWidget->DefaultBrush.Num());
				} break;

			case ESsPlayerWidgetRenderMode::UMG_OffScreen:
				{
					QUICK_SCOPE_CYCLE_COUNTER(STAT_SsPlayerWidget_UpdatePlayer_UMG_OffScreen);

					if(nullptr == PlayerWidget->GetRenderOffScreen())
					{
						break;
					}
					if(nullptr == OffScreenMID)
					{
						OffScreenMID = UMaterialInstanceDynamic::Create(BaseMaterial, GetTransientPackage());
						OffScreenMID->SetFlags(RF_Transient);
						OffScreenMID->SetTextureParameterValue(FName(TEXT("SsRenderTarget")), PlayerWidget->GetRenderOffScreen()->GetRenderTarget());
						RenderMIDs.Add(OffScreenMID);
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

					PlayerWidget->RenderParts = &Player.GetRenderParts();
					PlayerWidget->OffScreenBrush = *Brush;
					if(nullptr != PlayerWidget->GetRenderOffScreen())
					{
						PlayerWidget->GetRenderOffScreen()->ClearColor = OffScreenClearColor;
						PlayerWidget->GetRenderOffScreen()->Render(*PlayerWidget->RenderParts);
					}
					PlayerWidget->SetAnimCanvasSize(Player.GetAnimCanvasSize());
				} break;
		}
	}
}

UTexture* USsPlayerWidget::GetRenderTarget()
{
	if(PlayerWidget.IsValid())
	{
		return PlayerWidget->GetRenderTarget();
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

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerWidget::Play() Invalid Animation (%s, %s)"), *(AnimPackName.ToString()), *(AnimationName.ToString()));
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

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerWidget::PlayByIndex() Invalid Animation index (%d, %d)"), AnimPackIndex, AnimationIndex);
	return false;
}
bool USsPlayerWidget::PlaySequence(FName SequencePackName, FName SequenceName, int32 StartFrame, float PlayRate)
{
	int32 SequencePackIndex, SequenceIndex;
	if(Player.GetSequenceIndex(SequencePackName, SequenceName, SequencePackIndex, SequenceIndex))
	{
		return PlaySequenceByIndex(SequencePackIndex, SequenceIndex, StartFrame, PlayRate);
	}

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerWidget::PlaySequence() Invalid Sequence (%s, %s)"), *(SequencePackName.ToString()), *(SequenceName.ToString()));
	return false;
}
bool USsPlayerWidget::PlaySequenceByIndex(int32 SequencePackIndex, int32 SequenceIndex, int32 StartFrame, float PlayRate)
{
	if(Player.PlaySequence(SequencePackIndex, SequenceIndex, StartFrame, PlayRate))
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

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerWidget::PlaySequenceByIndex() Invalid Sequence index (%d, %d)"), SequencePackIndex, SequenceIndex);
	return false;
}
void USsPlayerWidget::GetPlayingAnimationName(FName& OutAnimPackName, FName& OutAnimationName) const
{
	int32 AnimPackIndex  = Player.GetPlayingAnimPackIndex();
	int32 AnimationIndex = Player.GetPlayingAnimationIndex();
	if(SsProject && (0 <= AnimPackIndex) && (0 <= AnimationIndex))
	{
		if(    (AnimPackIndex  < SsProject->AnimeList.Num())
			&& (AnimationIndex < SsProject->AnimeList[AnimPackIndex].AnimeList.Num())
			)
		{
			OutAnimPackName  = SsProject->AnimeList[AnimPackIndex].AnimePackName;
			OutAnimationName = SsProject->AnimeList[AnimPackIndex].AnimeList[AnimationIndex].AnimationName;
			return;
		}
	}
	OutAnimPackName  = NAME_None;
	OutAnimationName = NAME_None;
}
void USsPlayerWidget::GetPlayingAnimationIndex(int32& OutAnimPackIndex, int32& OutAnimationIndex) const
{
	OutAnimPackIndex  = Player.GetPlayingAnimPackIndex();
	OutAnimationIndex = Player.GetPlayingAnimationIndex();
}
void USsPlayerWidget::GetPlayingSequenceName(FName& OutSequencePackName, FName& OutSequenceName) const
{
	int32 SequencePackIndex = Player.GetPlayingSequencePackIndex();
	int32 SequenceIndex     = Player.GetPlayingSequenceIndex();
	if(Player.GetSsProject().IsValid() && (0 <= SequencePackIndex) && (0 <= SequenceIndex))
	{
		if(    (SequencePackIndex < Player.GetSsProject()->SequenceList.Num())
			&& (SequenceIndex     < Player.GetSsProject()->SequenceList[SequencePackIndex].SequenceList.Num())
			)
		{
			OutSequencePackName = Player.GetSsProject()->SequenceList[SequencePackIndex].SequencePackName;
			OutSequenceName     = Player.GetSsProject()->SequenceList[SequencePackIndex].SequenceList[SequenceIndex].SequenceName;
			return;
		}
	}
	OutSequencePackName = NAME_None;
	OutSequenceName     = NAME_None;
}
void USsPlayerWidget::GetPlayingSequenceIndex(int32& OutSequencePackIndex, int32& OutSequenceIndex) const
{
	OutSequencePackIndex = Player.GetPlayingSequencePackIndex();
	OutSequenceIndex     = Player.GetPlayingSequenceIndex();
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
bool USsPlayerWidget::IsPlayingSequence() const
{
	return Player.IsPlayingSequence();
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
int32 USsPlayerWidget::GetNumSequencePacks() const
{
	if(SsProject)
	{
		return SsProject->SequenceList.Num();
	}
	return 0;
}
int32 USsPlayerWidget::GetNumSequences(FName SequencePackName) const
{
	if(SsProject)
	{
		int32 SequencePackIndex = SsProject->FindSequencePackIndex(SequencePackName);
		if(0 <= SequencePackIndex)
		{
			return SsProject->SequenceList[SequencePackIndex].SequenceList.Num();
		}
	}
	return 0;
}
int32 USsPlayerWidget::GetNumSequencesByIndex(int32 SequencePackIndex) const
{
	if(SsProject && (0 <= SequencePackIndex) && (SequencePackIndex < SsProject->SequenceList.Num()))
	{
		return SsProject->SequenceList[SequencePackIndex].SequenceList.Num();
	}
	return 0;
}
bool USsPlayerWidget::GetSequenceIndexById(FName SequencePackName, int32 SequenceId, int32& OutSequencePackIndex, int32& OutSequneceIndex) const
{
	if(SsProject)
	{
		OutSequencePackIndex = SsProject->FindSequencePackIndex(SequencePackName);
		if(0 <= OutSequencePackIndex)
		{
			for(auto It = SsProject->SequenceList[OutSequencePackIndex].SequenceList.CreateConstIterator(); It; ++It)
			{
				if(SequenceId == It->Id)
				{
					OutSequneceIndex = It.GetIndex();
					return true;
				}
			}
		}
	}
	return false;
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
	if(Player.IsPlayingSequence())
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("PlaySequence does not support Loop."));
		return;
	}
	Player.LoopCount = InLoopCount;
}
int32 USsPlayerWidget::GetLoopCount() const
{
	return Player.LoopCount;
}

void USsPlayerWidget::SetRoundTrip(bool bInRoundTrip)
{
	if(Player.IsPlayingSequence())
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("PlaySequence does not support RoundTrip."));
		return;
	}
	Player.bRoundTrip = bInRoundTrip;
}
bool USsPlayerWidget::IsRoundTrip() const
{
	return Player.bRoundTrip;
}

void USsPlayerWidget::SetPlayRate(float InRate)
{
	if(Player.IsPlayingSequence() && (InRate < 0.f))
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("PlaySequence does not support negative PlayRate."));
		InRate = 0.f;
	}
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

void USsPlayerWidget::AddMaterialReplacement(FName PartName, UMaterialInterface* InBaseMaterial)
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if(0 <= PartIndex)
	{
		MaterialReplacementMap.Add(PartIndex, InBaseMaterial);
	}
}
void USsPlayerWidget::AddMaterialReplacementByIndex(int32 PartIndex, UMaterialInterface* InBaseMaterial)
{
	MaterialReplacementMap.Add(PartIndex, InBaseMaterial);
}
void USsPlayerWidget::RemoveMaterialReplacement(FName PartName)
{
	int32 PartIndex = Player.GetPartIndexFromName(PartName);
	if(0 <= PartIndex)
	{
		MaterialReplacementMap.Remove(PartIndex);
	}
}
void USsPlayerWidget::RemoveMaterialReplacementByIndex(int32 PartIndex)
{
	MaterialReplacementMap.Remove(PartIndex);
}
void USsPlayerWidget::RemoveMaterialReplacementAll()
{
	MaterialReplacementMap.Empty();
}

void USsPlayerWidget::AddMaterialReplacementPerBlendMode(EAlphaBlendType AlphaBlendMode, EColorBlendType ColorBlendMode, UMaterialInterface* InBaseMaterial)
{
	MaterialReplacementMapPerBlendMode.Add(SS_BlendTypeKey(AlphaBlendMode, ColorBlendMode), InBaseMaterial);
}
void USsPlayerWidget::RemoveMaterialReplacementPerBlendMode(EAlphaBlendType AlphaBlendMode, EColorBlendType ColorBlendMode)
{
	MaterialReplacementMapPerBlendMode.Remove(SS_BlendTypeKey(AlphaBlendMode, ColorBlendMode));
}
void USsPlayerWidget::RemoveMaterialReplacementAllPerBlendMode()
{
	MaterialReplacementMapPerBlendMode.Empty();
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
int32 USsPlayerWidget::GetPartIndexFromName(FName InPartName)
{
	return Player.GetPartIndexFromName(InPartName);
}
