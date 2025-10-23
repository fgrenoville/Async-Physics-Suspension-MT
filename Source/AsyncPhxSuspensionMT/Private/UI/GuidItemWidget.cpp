// Copyright (c) [2025] [Federico Grenoville]

#include "UI/GuidItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "UI/GuidItem.h"

void UGuidItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	if (DestroyButton)
	{
		DestroyButton->OnClicked.RemoveAll(this);
		DestroyButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleDestroyButtonClicked);
	}
	
	if (const UGuidItem* Data = Cast<UGuidItem>(ListItemObject))
	{
		Guid = Data->Guid;
		
		if (GuidText)
			GuidText->SetText(FText::FromString(Data->Guid.ToString()));
	}
	else
	{
		Guid.Invalidate();

		if (GuidText)
			GuidText->SetText(FText::GetEmpty());
	}
}

void UGuidItemWidget::NativeOnEntryReleased()
{
	IUserObjectListEntry::NativeOnEntryReleased();

	if (DestroyButton)
		DestroyButton->OnClicked.RemoveAll(this);
	
	OnActionRequested.Clear();
	Guid.Invalidate();
}

void UGuidItemWidget::HandleDestroyButtonClicked()
{
	OnActionRequested.Broadcast(Guid);
}
