// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VehicleSettingsWidget.generated.h"

class AVehicleManager;
class USuspensionWidget;
class ASWPVehicle;
class USWPSuspension;
class AAsyncPhxSuspensionPlayerController;
class UTextBlock;
class USlider;

UCLASS(Abstract, Blueprintable)
class ASYNCPHXSUSPENSIONMT_API UVehicleSettingsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Widget class used to represent a single suspension (UI element for tuning). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AsyncPhxSuspensionMT|Refs")
	TSubclassOf<USuspensionWidget> SuspensionWidgetClass;

public:
	UFUNCTION(BlueprintCallable, Category = "AsyncPhxSuspensionMT")
	void SetVehicleManager(AVehicleManager* InMgr);
	UFUNCTION(BlueprintCallable, Category = "AsyncPhxSuspensionMT")
	void SetPlayerController(AAsyncPhxSuspensionPlayerController* InPC);

protected:
	/** Uniform grid container that holds the four child suspension widgets. */
	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* Grid;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TWeakObjectPtr<AVehicleManager> VehicleManager = nullptr;
	UPROPERTY()
	TWeakObjectPtr<AAsyncPhxSuspensionPlayerController> PlayerController = nullptr;
	UPROPERTY()
	TWeakObjectPtr<ASWPVehicle> SelectedVehicle = nullptr;

	UPROPERTY()
	FGuid SelectedVehicleId;
	
	UPROPERTY()
	TObjectPtr<USuspensionWidget> FrontLeftSuspensionWidget = nullptr;
	UPROPERTY()
	TObjectPtr<USuspensionWidget> FrontRightSuspensionWidget = nullptr;
	UPROPERTY()
	TObjectPtr<USuspensionWidget> RearLeftSuspensionWidget = nullptr;
	UPROPERTY()
	TObjectPtr<USuspensionWidget> RearRightSuspensionWidget = nullptr;
	
private:
	USuspensionWidget* AddWidget(const FText& Label, int32 Row, int32 Col);
	
	UFUNCTION()
	void HandleSelectedVehicle(ASWPVehicle* NewVehicle);
	UFUNCTION()
	void HandleVehicleRemoved(const FGuid VehicleId);
};
