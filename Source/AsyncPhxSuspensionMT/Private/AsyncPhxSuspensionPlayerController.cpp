// Copyright (c) [2025] [Federico Grenoville]

#include "AsyncPhxSuspensionPlayerController.h"
#include "EngineUtils.h"
#include "SWPVehicle.h"
#include "Blueprint/UserWidget.h"
#include "VehicleManager.h"
#include "UI/HUDRoot.h"

void AAsyncPhxSuspensionPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Bind a simple click-to-select action.
	// NOTE: For rebindable input, consider an Enhanced Input Action instead of a raw key bind.
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ThisClass::SelectUnderCursor);
}

void AAsyncPhxSuspensionPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Only create UI and bind inputs on the locally controlled PC (client or standalone).
	if (!IsLocalController()) return;

	// Discover the VehicleManager present in the level.
	// NOTE: This grabs the first instance.
	for (TActorIterator<AVehicleManager> It(GetWorld()); It; ++It)
	{
		VehicleManager = *It;
		break;
	}

	// Instantiate the root HUD widget and wire dependencies.
	if (ensure(HUDRootClass))
	{
		HUDRoot = CreateWidget<UHUDRoot>(this, HUDRootClass);
		if (IsValid(HUDRoot))
		{
			HUDRoot->SetVehicleManager(VehicleManager.Get());
			HUDRoot->SetPlayerController(this);
			HUDRoot->AddToViewport();
		}
	}

	// Enable cursor + click/hover for UI + world picking.
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// Mixed input mode: allows UI interaction while keeping the game scene responsive.
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}

void AAsyncPhxSuspensionPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up the HUD.
	if (IsValid(HUDRoot))
	{
		HUDRoot->RemoveFromParent();
		HUDRoot = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void AAsyncPhxSuspensionPlayerController::SelectUnderCursor()
{
	FHitResult Hit;
	const bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	// Attempt to resolve a vehicle from the hit actor (top-level actor cast).
	ASWPVehicle* NewVehicle = bHit ? Cast<ASWPVehicle>(Hit.GetActor()) : nullptr;

	// Early out if selection didn't change.
	if (SelectedVehicle.Get() == NewVehicle) return;

	// TODO (optional): Visual feedback for old selection (outline, material param, Niagaras, etc.)
	/*
	if (const ASWPVehicle* OldVehicle = SelectedVehicle.Get())
	{
		TInlineComponentArray<UPrimitiveComponent*> Prims(OldVehicle);
		for (UPrimitiveComponent* C : Prims)
		{
			//if (C)
		}
	}
	*/

	SelectedVehicle = NewVehicle;

	// TODO (optional): Visual feedback for new selection.
	/*
	if (SelectedVehicle.IsValid())
	{
		if (ASWPVehicle* P = SelectedVehicle.Get())
		{
			TInlineComponentArray<UPrimitiveComponent*> Prims(P);
			for (UPrimitiveComponent* C : Prims)
			{
				//if (C)
			}
		}
	}
	*/

	// Notify listeners (e.g., HUD panels) that selection changed.
	OnSelectedVehicleChanged.Broadcast(SelectedVehicle.Get());
}
