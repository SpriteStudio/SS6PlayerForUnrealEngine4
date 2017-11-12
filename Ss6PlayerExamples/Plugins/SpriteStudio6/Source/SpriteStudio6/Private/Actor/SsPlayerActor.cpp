#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerActor.h"

#include "SsPlayerComponent.h"


//
ASsPlayerActor::ASsPlayerActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bAutoDestroy(false)
{
	SsPlayerComponent = ObjectInitializer.CreateDefaultSubobject<USsPlayerComponent>(this, TEXT("SsPlayerComponent"));
	RootComponent = SsPlayerComponent;

	SsPlayerComponent->OnSsEndPlay.AddDynamic(this, &ASsPlayerActor::OnEndPlay);
}

USsPlayerComponent* ASsPlayerActor::GetSsPlayer() const
{
	return SsPlayerComponent;
}

void ASsPlayerActor::OnEndPlay(FName, FName, int32, int32)
{
	if(bAutoDestroy)
	{
		Destroy();
	}
}