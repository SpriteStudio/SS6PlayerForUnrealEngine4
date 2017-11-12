#pragma once

#include "EditorReimportHandler.h"
#include "SspjFactory.h"

#include "ReimportSspjFactory.generated.h"


UCLASS()
class UReimportSspjFactory : public USspjFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

	// FReimportHandler interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const { return ImportPriority; }

public:
	bool bReimporting;
	TMap<FString, UTexture*> ExistImages;
};
