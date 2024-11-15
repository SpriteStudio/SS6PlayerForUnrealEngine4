﻿#pragma once

#include "GameFramework/Actor.h"

#include "SsPlayerActor.generated.h"


//
// 
//
UCLASS(ClassGroup=SpriteStudio, ComponentWrapperClass)
class SPRITESTUDIO6_API ASsPlayerActor : public AActor
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(Category=SpriteStudio, BlueprintCallable)
	class USsPlayerComponent* GetSsPlayer() const;

public:
	UPROPERTY()
	bool bAutoDestroy;

private:
	UFUNCTION()
	void OnEndPlay(FName AnimPackName, FName AnimationName, int32 AnimPackIndex, int32 AnimationIndex);

protected:
	UPROPERTY(Category=SpriteStudio, VisibleAnywhere, BlueprintReadOnly)
	class USsPlayerComponent* SsPlayerComponent;
};
