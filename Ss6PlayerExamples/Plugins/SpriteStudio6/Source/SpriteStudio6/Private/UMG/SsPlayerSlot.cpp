#include "SpriteStudio6PrivatePCH.h"
#include "SsPlayerSlot.h"


// コンストラクタ 
USsPlayerSlot::USsPlayerSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

USsPlayerSlot::~USsPlayerSlot()
{
	if(Slot)
	{
		Slot->WidgetSlot = nullptr;
	}
}

void USsPlayerSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	Slot = nullptr;
}

void USsPlayerSlot::BuildSlot(TSharedRef<SSsPlayerWidget> SsPlayerWidget)
{
	Slot = &SsPlayerWidget->AddSlot()
		[
			Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
		];
	Slot->WidgetSlot = this;

	SynchronizeProperties();
}

void USsPlayerSlot::SetPartIndex(int32 InPartIndex)
{
	if(Slot)
	{
		Slot->PartIndex(InPartIndex);
	}
}

#if WITH_EDITOR
// プロパティ編集イベント 
void USsPlayerSlot::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.Property)	// Undo/Redo時にNULLのケースがある 
	{
		FString PropertyName = PropertyChangedEvent.Property->GetNameCPP();
		if(0 == PropertyName.Compare(TEXT("PartName")))
		{
			if(Parent)
			{
				Parent->SynchronizeProperties();
			}
		}
	}
}
#endif

