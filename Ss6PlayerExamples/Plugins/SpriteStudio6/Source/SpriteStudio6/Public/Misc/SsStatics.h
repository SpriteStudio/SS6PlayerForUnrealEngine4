#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SsStatics.generated.h"

UCLASS()
class SPRITESTUDIO6_API USsStatics : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()


	// SpriteStudioアニメーションの単発再生 
	UFUNCTION(BlueprintCallable, Category="SpriteStudio", meta=(Keywords = "ss", WorldContext="WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static class ASsPlayerActor* SpawnSsPlayerAtLocation(UObject* WorldContextObject, class USs6Project* SsProject, FName AnimPackName, FName AnimationName, float UUPerPixel = 0.3f, FVector Location = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator, bool bAutoDestroy = true, int32 TranslucencySortPriority = 0);

	// SpriteStudioアニメーションの単発再生(インデックス指定) 
	UFUNCTION(BlueprintCallable, Category="SpriteStudio", meta=(Keywords = "ss", WorldContext="WorldContextObject", UnsafeDuringActorConstruction = "true"))
	static class ASsPlayerActor* SpawnSsPlayerAtLocationByIndex(UObject* WorldContextObject, class USs6Project* SsProject, int32 AnimPackIndex, int32 AnimationIndex, float UUPerPixel = 0.3f, FVector Location = FVector::ZeroVector, FRotator Rotation = FRotator::ZeroRotator, bool bAutoDestroy = true, int32 TranslucencySortPriority = 0);
};
