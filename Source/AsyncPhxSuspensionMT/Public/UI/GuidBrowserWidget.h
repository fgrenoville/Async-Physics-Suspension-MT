// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GuidBrowserWidget.generated.h"

class UListView;
class UGuidItem;
class AVehicleManager;

UCLASS(Abstract, Blueprintable)
class ASYNCPHXSUSPENSIONMT_API UGuidBrowserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AsyncPhxSuspensionMT")
	void SetVehicleManager(AVehicleManager* InMgr);

protected:
	UPROPERTY(meta = (BindWidget))
	UListView* GuidList = nullptr;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TWeakObjectPtr<AVehicleManager> VehicleManager = nullptr;
	UPROPERTY()
	TMap<FGuid, TObjectPtr<UGuidItem>> ItemsById;
	
private:
	UFUNCTION()
	void HandleVehicleSpawned(const FGuid VehicleId);
	UFUNCTION()
	void HandleVehicleRemoved(const FGuid VehicleId);

	void HandleEntryGenerated(UUserWidget& EntryWidget);
	void HandleEntryReleased(UUserWidget& EntryWidget);
	
	UFUNCTION()
	void HandleRowActionRequested(const FGuid& Guid);
};
