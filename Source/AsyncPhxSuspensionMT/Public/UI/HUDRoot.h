// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDRoot.generated.h"

class AAsyncPhxSuspensionPlayerController;
class UVehicleSettingsWidget;
class UGuidBrowserWidget;
class USizeBox;
class AVehicleManager;

UCLASS(Abstract, Blueprintable)
class ASYNCPHXSUSPENSIONMT_API UHUDRoot : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AsyncPhxSuspensionMT")
	void SetVehicleManager(AVehicleManager* InMgr);
	UFUNCTION(BlueprintCallable, Category = "AsyncPhxSuspensionMT")
	void SetPlayerController(AAsyncPhxSuspensionPlayerController* InPC);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "AsyncPhxSuspensionMT|Refs")
	TSubclassOf<UGuidBrowserWidget> GuidBrowserWidgetClass = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "AsyncPhxSuspensionMT|Refs")
	TSubclassOf<UVehicleSettingsWidget> VehicleSettingsWidgetClass = nullptr;

	UPROPERTY(meta = (BindWidget))
	USizeBox* GuidBrowserSlot = nullptr;
	UPROPERTY(meta = (BindWidget))
	USizeBox* VehicleSettingsSlot = nullptr;

protected:
	virtual void NativeConstruct() override;
	
private:
	UPROPERTY()
	TWeakObjectPtr<AVehicleManager> VehicleManager = nullptr;
	UPROPERTY()
	TWeakObjectPtr<AAsyncPhxSuspensionPlayerController> PlayerController = nullptr;
	
	UPROPERTY()
	TObjectPtr<UGuidBrowserWidget> GuidBrowserWidget = nullptr;
	UPROPERTY()
	TObjectPtr<UVehicleSettingsWidget> VehicleSettingsWidget = nullptr;
};
