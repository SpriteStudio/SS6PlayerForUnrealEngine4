#include "SsStatics.h"

#include "Ss6Project.h"
#include "SsPlayerComponent.h"
#include "SsPlayerActor.h"


USsStatics::USsStatics(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}


// SpriteStudioアニメーションの単発再生 
ASsPlayerActor* USsStatics::SpawnSsPlayerAtLocation(
	UObject* WorldContextObject,
	USs6Project* SsProject,
	FName AnimPackName,
	FName AnimationName,
	float UUPerPixel,
	FVector Location,
	FRotator Rotation,
	bool bAutoDestroy,
	int32 TranslucencySortPriority
	)
{
	int32 AnimPackIndex  = -1;
	int32 AnimationIndex = -1;
	if(NULL == SsProject)
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SpawnSsPlayerAtLocation() Invalid SsProject"));
		return NULL;
	}
	if(!SsProject->FindAnimationIndex(AnimPackName, AnimationName, AnimPackIndex, AnimationIndex))
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SpawnSsPlayerAtLocation() Invalid Animation (%s, %s)"), *(AnimPackName.ToString()), *(AnimationName.ToString()));
		return NULL;
	}
	return SpawnSsPlayerAtLocationByIndex(
		WorldContextObject,
		SsProject,
		AnimPackIndex,
		AnimationIndex,
		UUPerPixel,
		Location,
		Rotation,
		bAutoDestroy,
		TranslucencySortPriority
		);
}

// SpriteStudioアニメーションの単発再生(インデックス指定) 
ASsPlayerActor* USsStatics::SpawnSsPlayerAtLocationByIndex(
	UObject* WorldContextObject,
	USs6Project* SsProject,
	int32 AnimPackIndex,
	int32 AnimationIndex,
	float UUPerPixel,
	FVector Location,
	FRotator Rotation,
	bool bAutoDestroy,
	int32 TranslucencySortPriority
	)
{
	if(NULL == SsProject)
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SpawnSsPlayerAtLocationByIndex() Invalid SsProject"));
		return NULL;
	}
	if(    (AnimPackIndex  < 0) || (SsProject->AnimeList.Num() <= AnimPackIndex)
		|| (AnimationIndex < 0) || (SsProject->AnimeList[AnimPackIndex].AnimeList.Num() <= AnimationIndex)
		)
	{
		UE_LOG(LogSpriteStudio, Warning, TEXT("SpawnSsPlayerAtLocationByIndex() Invalid Animation index (%d, %d)"), AnimPackIndex, AnimationIndex);
		return NULL;
	}

	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if(NULL == World)
	{
		return NULL;
	}

	ASsPlayerActor* SsPlayer = Cast<ASsPlayerActor>(World->SpawnActor(ASsPlayerActor::StaticClass(), &Location, &Rotation));
	SsPlayer->SetActorLocation(Location);
	SsPlayer->SetActorRotation(Rotation);
	SsPlayer->bAutoDestroy = bAutoDestroy;

	USsPlayerComponent* SsPC = SsPlayer->GetSsPlayer();
	SsPC->SsProject = SsProject;
	SsPC->UUPerPixel = UUPerPixel;
	SsPC->bAutoPlay = false;
	SsPC->TranslucencySortPriority = (int16)TranslucencySortPriority;
	SsPC->UnregisterComponent();
	SsPC->RegisterComponent();

	// ループ回数[1]で再生
	SsPC->PlayByIndex(AnimPackIndex, AnimationIndex, 0, 1.f, 1, false);

	return SsPlayer;
}
