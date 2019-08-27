#include "SpriteStudio6PrivatePCH.h"
#include "SpriteStudio6.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/AssertionMacros.h"

#include "SsGameSettings.h"


DEFINE_LOG_CATEGORY(LogSpriteStudio);
#define LOCTEXT_NAMESPACE "SpriteStudio6"

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
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("SpriteStudio6"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/SpriteStudio6"), PluginShaderDir);
}

void FSpriteStudio6::ShutdownModule()
{
}


#undef LOCTEXT_NAMESPACE
