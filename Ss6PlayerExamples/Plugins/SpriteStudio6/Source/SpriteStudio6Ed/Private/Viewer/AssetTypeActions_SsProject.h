#pragma once

#include "AssetTypeActions_Base.h"

class FAssetTypeActions_SsProject : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_SsProject", "SsProject"); }
	virtual UClass* GetSupportedClass() const override;
	virtual FColor GetTypeColor() const override { return SSPJ_COLOR; }
	virtual uint32 GetCategories() override { return EAssetTypeCategories::UI; }
	virtual void OpenAssetEditor( const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>() ) override;

	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	void ExecuteReimport(TArray<TWeakObjectPtr<class USsProject>> Objects);

};
