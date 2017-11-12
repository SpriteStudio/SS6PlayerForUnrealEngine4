#include "SpriteStudio6EdPrivatePCH.h"
#include "SsPlayerActorFactory.h"

#include "SsProject.h"
#include "SsPlayerActor.h"
#include "SsPlayerComponent.h"


#define LOCTEXT_NAMESPACE "SsPlayerActorFactory"


USsPlayerActorFactory::USsPlayerActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("SsPlayerActorDisplayName", "SsPlayer Actor");
	NewActorClass = ASsPlayerActor::StaticClass();
}

bool USsPlayerActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if(!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(USsProject::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoSsProject", "A valid SsProject must be specified.");
		return false;
	}

	return true;
}

void USsPlayerActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	USsProject* SsProject = CastChecked<USsProject>(Asset);
	FActorLabelUtilities::SetActorLabelUnique(NewActor, SsProject->GetName());

	ASsPlayerActor* SsPlayerActor = CastChecked<ASsPlayerActor>(NewActor);
	USsPlayerComponent* SsPlayerComponent = SsPlayerActor->GetSsPlayer();
	check(SsPlayerComponent);

	SsPlayerComponent->UnregisterComponent();
	SsPlayerComponent->SsProject = SsProject;
	SsPlayerComponent->OnSetSsProject();
	SsPlayerComponent->RegisterComponent();
}

UObject* USsPlayerActorFactory::GetAssetFromActorInstance(AActor* Instance)
{
	check(Instance->IsA(NewActorClass));
	ASsPlayerActor* SsPlayerActor = CastChecked<ASsPlayerActor>(Instance);

	check(SsPlayerActor->GetSsPlayer());
	return SsPlayerActor->GetSsPlayer()->SsProject;
}

void USsPlayerActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	if(Asset != NULL && CDO != NULL)
	{
		USsProject* SsProject = CastChecked<USsProject>(Asset);
		ASsPlayerActor* SsPlayerActor = CastChecked<ASsPlayerActor>(CDO);
		USsPlayerComponent* SsPlayerComponent = SsPlayerActor->GetSsPlayer();

		SsPlayerComponent->SsProject = SsProject;
	}
}

#undef LOCTEXT_NAMESPACE
