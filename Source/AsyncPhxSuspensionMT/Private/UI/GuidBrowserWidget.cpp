// Copyright (c) [2025] [Federico Grenoville]

#include "UI/GuidBrowserWidget.h"
#include "VehicleManager.h"
#include "Components/ListView.h"
#include "UI/GuidItem.h"
#include "UI/GuidItemWidget.h"

void UGuidBrowserWidget::SetVehicleManager(AVehicleManager* InMgr)
{
	VehicleManager = InMgr;
}

void UGuidBrowserWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (VehicleManager.IsValid())
	{
		VehicleManager->OnVehicleSpawned.AddDynamic(this, &UGuidBrowserWidget::HandleVehicleSpawned);
		VehicleManager->OnVehicleRemoved.AddDynamic(this, &UGuidBrowserWidget::HandleVehicleRemoved);
	}
	
	if (GuidList)
	{
		GuidList->OnEntryWidgetGenerated().AddUObject(this, &UGuidBrowserWidget::HandleEntryGenerated);
		GuidList->OnEntryWidgetReleased().AddUObject(this, &UGuidBrowserWidget::HandleEntryReleased);
	}
}

void UGuidBrowserWidget::NativeDestruct()
{
	if (VehicleManager.IsValid())
	{
		VehicleManager->OnVehicleSpawned.RemoveAll(this);
		VehicleManager->OnVehicleRemoved.RemoveAll(this);
	}
	
	if (GuidList)
	{
		GuidList->OnEntryWidgetGenerated().RemoveAll(this);
		GuidList->OnEntryWidgetReleased().RemoveAll(this);
	}
	
	Super::NativeDestruct();
}

void UGuidBrowserWidget::HandleVehicleSpawned(const FGuid VehicleId)
{
	if (!GuidList) return;
	if (ItemsById.Contains(VehicleId)) return;

	UGuidItem* Item = NewObject<UGuidItem>(this);
	Item->Guid = VehicleId;

	ItemsById.Add(VehicleId, Item);
	GuidList->AddItem(Item);
}

void UGuidBrowserWidget::HandleVehicleRemoved(const FGuid VehicleId)
{
	if (!GuidList) return;
	
	if (const TObjectPtr<UGuidItem>* Found = ItemsById.Find(VehicleId))
	{
		GuidList->RemoveItem(*Found);
		ItemsById.Remove(VehicleId);
	}
}

void UGuidBrowserWidget::HandleEntryGenerated(UUserWidget& EntryWidget)
{
	if (auto* Row = Cast<UGuidItemWidget>(&EntryWidget))
	{
		Row->OnActionRequested.RemoveAll(this);
		Row->OnActionRequested.AddUniqueDynamic(this, &ThisClass::HandleRowActionRequested);
	}
}

void UGuidBrowserWidget::HandleEntryReleased(UUserWidget& EntryWidget)
{
	if (auto* Row = Cast<UGuidItemWidget>(&EntryWidget))
		Row->OnActionRequested.RemoveAll(this);
}


void UGuidBrowserWidget::HandleRowActionRequested(const FGuid& Guid)
{
	if (VehicleManager.IsValid())
		VehicleManager->RemoveVehicleById(Guid);
}

