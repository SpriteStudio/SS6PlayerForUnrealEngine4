#include "SpriteStudio6PrivatePCH.h"
#include "SpriteStudio6.h"

#include "Misc/AssertionMacros.h"

DEFINE_LOG_CATEGORY(LogSpriteStudio);


FCustomVersionRegistration GRegisterSspjCustomVersion(
	SSPJ_GUID,
	SSPJ_VERSION,
	TEXT("SsProjectVersion")
	);


class FSpriteStudio6 : public ISpriteStudio6
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FSpriteStudio6, SpriteStudio6)



void FSpriteStudio6::StartupModule()
{
}


void FSpriteStudio6::ShutdownModule()
{
}



