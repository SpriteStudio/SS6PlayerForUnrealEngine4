#pragma once

#include "Factories.h"

#include "SspjFactory.generated.h"

UCLASS()
class USspjFactory : public UFactory
{
	GENERATED_UCLASS_BODY()

	// UFactory Interface
	virtual bool DoesSupportClass(UClass * Class) override;
	virtual UClass* ResolveSupportedClass() override;
	virtual UObject* FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* InBufferEnd, FFeedbackContext* Warn) override;
};



