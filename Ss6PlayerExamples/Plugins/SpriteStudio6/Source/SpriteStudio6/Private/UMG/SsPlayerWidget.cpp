#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerWidget.h"

#include "SsProject.h"
#include "SsRenderOffScreen.h"


// コンストラクタ 
USsPlayerWidget::USsPlayerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FSsPlayPropertySync(&SsProject, &AutoPlayAnimPackName, &AutoPlayAnimationName, &AutoPlayAnimPackIndex, &AutoPlayAnimationIndex)
	, RenderOffScreen(NULL)
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
	, OffScreenRenderResolution(512, 512)
{
	// UMG用マテリアル参照の取得
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UMaterialInterface> UMGBase;

		FConstructorStatics()
			: UMGBase(TEXT("/SpriteStudio6/SsMaterial_UMGDefault"))
		{}
	};
	static FConstructorStatics CS;

	BaseMaterial = CS.UMGBase.Object;
}

// デストラクタ 
USsPlayerWidget::~USsPlayerWidget()
{
	if(NULL != RenderOffScreen)
	{
		RenderOffScreen->ReserveTerminate();
		RenderOffScreen = NULL;
	}
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

		if(bAutoPlay && (0 <= AutoPlayAnimPackIndex) && (0 <= AutoPlayAnimationIndex))
		{
			Player.Play(AutoPlayAnimPackIndex, AutoPlayAnimationIndex, AutoPlayStartFrame, AutoPlayRate, AutoPlayLoopCount, bAutoPlayRoundTrip);
		}

		// オフスクリーンレンダリングの初期化 
		uint32 MaxPartsNum = SsProject->CalcMaxRenderPartsNum();
		if((NULL != RenderOffScreen) && !RenderOffScreen->CanReuse(OffScreenRenderResolution.X, OffScreenRenderResolution.Y, MaxPartsNum))
		{
			RenderOffScreen->ReserveTerminate();
			RenderOffScreen = NULL;
		}
		if(NULL == RenderOffScreen)
		{
			RenderOffScreen = new FSsRenderOffScreen();
			RenderOffScreen->Initialize(OffScreenRenderResolution.X, OffScreenRenderResolution.Y, MaxPartsNum);
			RenderOffScreen->Render(Player.GetRenderParts());
		}

		// UImageにマテリアルを設定 
		SetBrushFromMaterial(BaseMaterial);
		UMaterialInstanceDynamic* MID = GetDynamicMaterial();
		MID->SetTextureParameterValue(FName(TEXT("SsRenderTarget")), RenderOffScreen->GetRenderTarget());
	}
}

// 更新 
void USsPlayerWidget::Tick(float DeltaTime)
{
#if WITH_EDITOR
	// １フレームに複数回の呼び出しが来てしまうのに対処. 
	// UObject と FTickableGameObject を併用した際のバグらしい？ 
	if(GetWorld())
	{
		if((GetWorld()->TimeSeconds - BackWorldTime + 0.001f) < DeltaTime)
		{
			return;
		}
		BackWorldTime = this->GetWorld()->TimeSeconds;
	}
#endif

	if(bAutoUpdate)
	{
		UpdatePlayer(DeltaTime);
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

		// オフスクリーンレンダリングの描画命令発行 
		if(NULL != RenderOffScreen)
		{
			RenderOffScreen->Render(Player.GetRenderParts());
		}
	}
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
		if(bAutoUpdate)
		{
			UpdatePlayer(0.f);
		}
		return true;
	}

	UE_LOG(LogSpriteStudio, Warning, TEXT("SsPlayerWidget::PlayByIndex() Invalid Animation index (%d, %d)"), AnimPackIndex, AnimationIndex);
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
