#pragma once

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

private:
	UPROPERTY(Category=SpriteStudio, VisibleAnywhere)
	class USsPlayerComponent* SsPlayerComponent;
};
