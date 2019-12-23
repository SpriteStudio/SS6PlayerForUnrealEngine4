#include "SsPlayerActorFactory.h"

#include "Ss6Project.h"
#include "SsPlayerActor.h"
#include "SsPlayerComponent.h"


#define LOCTEXT_NAMESPACE "SpriteStudio6Ed"


USsPlayerActorFactory::USsPlayerActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = LOCTEXT("SsPlayerActorDisplayName", "SsPlayer Actor");
	NewActorClass = ASsPlayerActor::StaticClass();
}

bool USsPlayerActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if(!AssetData.IsValid() || !AssetData.GetClass()->IsChildOf(USs6Project::StaticClass()))
	{
		OutErrorMsg = NSLOCTEXT("CanCreateActor", "NoSsProject", "A valid SsProject must be specified.");
		return false;
	}

	return true;
}

void USsPlayerActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	USs6Project* SsProject = CastChecked<USs6Project>(Asset);
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
		USs6Project* SsProject = CastChecked<USs6Project>(Asset);
		ASsPlayerActor* SsPlayerActor = CastChecked<ASsPlayerActor>(CDO);
		USsPlayerComponent* SsPlayerComponent = SsPlayerActor->GetSsPlayer();

		SsPlayerComponent->SsProject = SsProject;
	}
}

#undef LOCTEXT_NAMESPACE
