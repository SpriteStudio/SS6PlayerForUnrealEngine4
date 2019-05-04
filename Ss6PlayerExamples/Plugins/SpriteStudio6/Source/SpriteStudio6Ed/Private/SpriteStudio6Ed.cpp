#include "SpriteStudio6EdPrivatePCH.h"

#include "MessageLogModule.h"
#include "ISettingsModule.h"

#include "AssetTypeActions_SsProject.h"
#include "SsImportSettings.h"

DEFINE_LOG_CATEGORY(LogSpriteStudioEd);
#define LOCTEXT_NAMESPACE "SpriteStudio6Ed"

class FSpriteStudio6Ed : public ISpriteStudio6Ed
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	TSharedPtr<FAssetTypeActions_SsProject> SspjAssetTypeActions;
};

IMPLEMENT_MODULE(FSpriteStudio6Ed, SpriteStudio6Ed)



void FSpriteStudio6Ed::StartupModule()
{
	SspjAssetTypeActions = MakeShareable(new FAssetTypeActions_SsProject);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().RegisterAssetTypeActions(SspjAssetTypeActions.ToSharedRef());

	Style = MakeShareable(new FSpriteStudio6EdStyle());

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.RegisterLogListing("SSPJ Import Log", LOCTEXT("SspjImportLog", "SSPJ Import Log"));


	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Editor", "Plugins", "SpriteStudio6",
			LOCTEXT("SsImportSettingsName", "Sprite Studio 6"),
			LOCTEXT("SsImportSettingsDescription", "Sprite Studio 6 Import Settings"),
			GetMutableDefault<USsImportSettings>()
			);
	}
}


void FSpriteStudio6Ed::ShutdownModule()
{
	if (SspjAssetTypeActions.IsValid())
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get().UnregisterAssetTypeActions(SspjAssetTypeActions.ToSharedRef());
		}
		SspjAssetTypeActions.Reset();
	}

	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.UnregisterLogListing("SSPJ Import Log");


	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Editor", "Plugins", "SpriteStudio6");
	}
}

#undef LOCTEXT_NAMESPACE
