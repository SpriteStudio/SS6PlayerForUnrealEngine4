#pragma once

#include "ActorFactories/ActorFactory.h"
#include "SsPlayerActorFactory.generated.h"

UCLASS()
class USsPlayerActorFactory : public UActorFactory
{
	GENERATED_UCLASS_BODY()

	// UActorFactory interface 
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
};

