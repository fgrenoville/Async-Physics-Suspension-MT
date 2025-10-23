// Copyright (c) [2025] [Federico Grenoville]

#include "UI/HUDRoot.h"
#include "Components/SizeBox.h"
#include "VehicleManager.h"
#include "AsyncPhxSuspensionPlayerController.h"
#include "UI/GuidBrowserWidget.h"
#include "UI/VehicleSettingsWidget.h"

void UHUDRoot::SetVehicleManager(AVehicleManager* InMgr)
{
	VehicleManager = InMgr;
}

void UHUDRoot::SetPlayerController(AAsyncPhxSuspensionPlayerController* InPC)
{
	PlayerController = InPC;
}

void UHUDRoot::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(GuidBrowserWidgetClass) && GuidBrowserSlot)
	{
		GuidBrowserWidget = CreateWidget<UGuidBrowserWidget>(GetOwningPlayer(), GuidBrowserWidgetClass);
		if (IsValid(GuidBrowserWidget))
		{
			GuidBrowserWidget->SetVehicleManager(VehicleManager.Get());
			GuidBrowserSlot->SetContent(GuidBrowserWidget);
		}
	}

	if (IsValid(VehicleSettingsWidgetClass) && VehicleSettingsSlot)
	{
		VehicleSettingsWidget = CreateWidget<UVehicleSettingsWidget>(GetOwningPlayer(), VehicleSettingsWidgetClass);
		if (IsValid(VehicleSettingsWidget))
		{
			VehicleSettingsWidget->SetVehicleManager(VehicleManager.Get());
			VehicleSettingsWidget->SetPlayerController(PlayerController.Get());
			VehicleSettingsSlot->SetContent(VehicleSettingsWidget);
		}
	}
}
