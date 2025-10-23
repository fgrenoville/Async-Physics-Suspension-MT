// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AsyncPhxSuspensionPlayerController.generated.h"

class ASWPVehicle;
class UHUDRoot;
class AVehicleManager;
class UGuidBrowserWidget;

/**
 * AAsyncPhxSuspensionPlayerController
 * 
 * Lightweight PlayerController that:
 *  - Binds a mouse-click selection action.
 *  - Locates the AVehicleManager at BeginPlay.
 *  - Creates and owns a HUD widget (UHUDRoot), wiring it to the manager and controller.
 *  - Exposes a simple "select vehicle under cursor" flow via a multicast delegate.
 *
 * Threading/Lifecycle:
 *  - All methods here are GT-only (UI, input, actor iteration).
 *  - HUD widget is created for local controllers only and cleaned up in EndPlay().
 *  - Selected vehicle is kept as a TWeakObjectPtr to avoid keeping destroyed actors alive.
 */
UCLASS(Abstract, Blueprintable)
class ASYNCPHXSUSPENSIONMT_API AAsyncPhxSuspensionPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedVehicleChanged, ASWPVehicle*, NewVehicle);
	UPROPERTY()
	FOnSelectedVehicleChanged OnSelectedVehicleChanged;
	
	virtual void SetupInputComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true), Category = "AsyncPhxSuspensionMT|Refs")
	TSubclassOf<UHUDRoot> HUDRootClass = nullptr;

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AsyncPhxSuspensionMT")
	AVehicleManager* GetVehicleManager() const { return VehicleManager.Get(); }

	UFUNCTION()
	void SelectUnderCursor();                 
	
private:
	UPROPERTY()
	TWeakObjectPtr<AVehicleManager> VehicleManager = nullptr;
	UPROPERTY()
	TWeakObjectPtr<ASWPVehicle> SelectedVehicle = nullptr;
	
	UPROPERTY()
	TObjectPtr<UHUDRoot> HUDRoot = nullptr;
};
