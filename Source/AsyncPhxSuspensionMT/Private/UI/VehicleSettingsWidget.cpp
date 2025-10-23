// Copyright (c) [2025] [Federico Grenoville]

#include "UI/VehicleSettingsWidget.h"
#include "AsyncPhxSuspensionPlayerController.h"
#include "SWPVehicle.h"
#include "VehicleManager.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "UI/SuspensionWidget.h"

void UVehicleSettingsWidget::SetVehicleManager(AVehicleManager* InMgr)
{
	VehicleManager = InMgr;
}

void UVehicleSettingsWidget::SetPlayerController(AAsyncPhxSuspensionPlayerController* InPC)
{
	PlayerController = InPC;
}

void UVehicleSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Grid) return;
	Grid->ClearChildren();
	
	FrontLeftSuspensionWidget = AddWidget(FText::FromString(TEXT("Front Left Suspension")), 0, 0);
	FrontRightSuspensionWidget = AddWidget(FText::FromString(TEXT("Front Right Suspension")), 0, 1);
	RearLeftSuspensionWidget = AddWidget(FText::FromString(TEXT("Rear Left Suspension")), 1, 0);
	RearRightSuspensionWidget = AddWidget(FText::FromString(TEXT("Rear Right Suspension")), 1, 1);

	if (VehicleManager.IsValid())
		VehicleManager->OnVehicleRemoved.AddDynamic(this, &ThisClass::HandleVehicleRemoved);
	
	if (PlayerController.IsValid())
		PlayerController->OnSelectedVehicleChanged.AddUniqueDynamic(this, &ThisClass::HandleSelectedVehicle);
	
	Grid->SetVisibility(ESlateVisibility::Collapsed);
}

void UVehicleSettingsWidget::NativeDestruct()
{
	if (VehicleManager.IsValid())
		VehicleManager->OnVehicleRemoved.RemoveAll(this);
	
	
	if (PlayerController.IsValid())
		PlayerController->OnSelectedVehicleChanged.RemoveAll(this);
	

	Super::NativeDestruct();
}

USuspensionWidget* UVehicleSettingsWidget::AddWidget(const FText& Label, int32 Row, int32 Col)
{
	if (!Grid) return nullptr;

	USuspensionWidget* Widget = CreateWidget<USuspensionWidget>(GetOwningPlayer(), SuspensionWidgetClass);
	if (!Widget) return nullptr;

	Widget->SetWidgetName(Label);
	
	if (UUniformGridSlot* SlotPosition = Grid->AddChildToUniformGrid(Widget, Row, Col))
	{
		SlotPosition->SetHorizontalAlignment(HAlign_Fill);
		SlotPosition->SetVerticalAlignment(VAlign_Fill);
	}

	return Widget;
}

void UVehicleSettingsWidget::HandleSelectedVehicle(ASWPVehicle* NewVehicle)
{
	if (IsValid(NewVehicle))
	{
		SelectedVehicle = NewVehicle;
		SelectedVehicleId = SelectedVehicle->GetGuid();
		
		FrontLeftSuspensionWidget->ReconfigureWidget(SelectedVehicle->GetFrontLeftSuspension());
		FrontRightSuspensionWidget->ReconfigureWidget(SelectedVehicle->GetFrontRightSuspension());
		RearLeftSuspensionWidget->ReconfigureWidget(SelectedVehicle->GetRearLeftSuspension());
		RearRightSuspensionWidget->ReconfigureWidget(SelectedVehicle->GetRearRightSuspension());

		Grid->SetVisibility(ESlateVisibility::Visible);
	}
	else
		Grid->SetVisibility(ESlateVisibility::Collapsed);
	
}

void UVehicleSettingsWidget::HandleVehicleRemoved(const FGuid VehicleId)
{
	if (VehicleId == SelectedVehicleId)
	{
		SelectedVehicle = nullptr;
		SelectedVehicleId.Invalidate();
		Grid->SetVisibility(ESlateVisibility::Collapsed);
	}
}
