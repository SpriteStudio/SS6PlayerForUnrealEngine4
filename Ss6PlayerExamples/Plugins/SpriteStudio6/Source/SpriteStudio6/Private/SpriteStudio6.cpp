#include "SpriteStudio6PrivatePCH.h"
#include "SpriteStudio6.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/AssertionMacros.h"
#include "ISettingsModule.h"

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

	void PostEngineInit();
};

IMPLEMENT_MODULE(FSpriteStudio6, SpriteStudio6)



void FSpriteStudio6::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("SpriteStudio6"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/SpriteStudio6"), PluginShaderDir);

	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FSpriteStudio6::PostEngineInit);
}

void FSpriteStudio6::PostEngineInit()
{
	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "SpriteStudio6",
			LOCTEXT("SsGameSettingsName", "Sprite Studio 6"),
			LOCTEXT("SsGameSettingsDescription", "Sprite Studio 6 Game Settings"),
			GetMutableDefault<USsGameSettings>()
		);
	}
}

void FSpriteStudio6::ShutdownModule()
{
	FCoreDelegates::OnPostEngineInit.RemoveAll(this);

	if(UObjectInitialized())
	{
		if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "SpriteStudio6");
		}
	}
}


#undef LOCTEXT_NAMESPACE
