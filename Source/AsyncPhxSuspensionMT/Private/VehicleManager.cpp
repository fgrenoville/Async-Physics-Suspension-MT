// Copyright (c) [2025] [Federico Grenoville]

#include "AsyncPhxSuspensionMT/Public/VehicleManager.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "SWPVehicle.h"
#include "Kismet/GameplayStatics.h"

AVehicleManager::AVehicleManager()
{
	PrimaryActorTick.bCanEverTick = false;		// Manager does not need per-frame Tick by default.

	// Grid spawn defaults
	SpawnNumMaxRows = 5;
	SpawnForwardSpacing = 500.0f;
	SpawnRightSpacing = 400.0f;
	SpawnStartingAt = (FTransform(FRotator(0, 0, 0)));
	SpawnIndex = 0;

	VehicleDeathFX = nullptr;
	ScaleFactorFX = FVector::OneVector;
}

void AVehicleManager::BeginPlay()
{
	Super::BeginPlay();

	// Cache the world as a weak pointer (PIE-safe; avoids dangling references on teardown).
	CurrentWorld = GetWorld();

	// Pre-warm a small Niagara pool so the first death FX does not hitch on allocation.
	PrimeNiagaraPool(VehicleDeathFX, 4);
}

void AVehicleManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Note: tick is unused here.
}

// Spawns a vehicle at a specific transform. Optionally possesses it with PlayerController 0.
// Returns the vehicle's FGuid (gameplay identifier) or an invalid FGuid on failure.
FGuid AVehicleManager::SpawnVehicleAtTransform(const FTransform& SpawnTransform, const bool bPossess)
{
	if (!CurrentWorld.IsValid()) return FGuid();
	if (!IsValid(SWPVehicleClass)) return FGuid();

	FActorSpawnParameters SpawnParameters;
	// Try to place, but never fail to spawn (useful for dense grids).
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ASWPVehicle* NewVehicle = CurrentWorld->SpawnActor<ASWPVehicle>(SWPVehicleClass, SpawnTransform, SpawnParameters);
	if (!NewVehicle) return FGuid();

	// Register bookkeeping with FGuid (the vehicle owns/knows its own GUID).
	const FGuid NewVehicleGuid = NewVehicle->GetGuid();
	SpawnedVehicles.Add(NewVehicleGuid, NewVehicle);
	OnVehicleSpawned.Broadcast(NewVehicleGuid);

	// Optional immediate possession.
	if (bPossess)
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(CurrentWorld.Get(), 0))
		{
			PC->Possess(NewVehicle);
		}
	}	

	// Keep the map coherent even if someone calls Destroy() externally.
	NewVehicle->OnDestroyed.AddUniqueDynamic(this, &AVehicleManager::OnVehicleDestroyed);
	
	return NewVehicleGuid;
}

// Spawns at the next grid slot using the configured grid parameters.
// Blueprint-friendly method.
FGuid AVehicleManager::SpawnVehicle(const bool bPossess)
{
	const FTransform SpawnSpot = GetNewSpawnLocation(SpawnIndex, SpawnNumMaxRows, SpawnForwardSpacing,
		SpawnRightSpacing, SpawnStartingAt);

	SpawnIndex++;
	
	return SpawnVehicleAtTransform(SpawnSpot, bPossess);
}

// Removes a vehicle by its GUID. This triggers the normal Actor destruction path,
// which will in turn fire OnDestroyed and clean bookkeeping + FX.
void AVehicleManager::RemoveVehicleById(const FGuid VehicleId)
{
	const TWeakObjectPtr<ASWPVehicle>* VehiclePtr = SpawnedVehicles.Find(VehicleId);
	if (!VehiclePtr) return;

	ASWPVehicle* Vehicle = VehiclePtr->Get();
	if (!Vehicle) return;

	// Cleanly release player control if any.
	if (AController* Controller = Vehicle->GetController())
	{
		Controller->UnPossess();
	}
	
	Vehicle->Destroy();
}

// Computes a grid-based spawn transform.
// Index is mapped to (Row,Col) = (Index / NumMaxRows, Index % NumMaxRows).
// Forward/Right vectors come from 'Origin' rotation; scale is always 1.
FTransform AVehicleManager::GetNewSpawnLocation(int32 Index, int32 NumMaxRows, float ForwardSpacing, float RightSpacing,
	const FTransform& Origin)
{
	const int32 Col = Index % NumMaxRows;
	const int32 Row = Index / NumMaxRows;

	const FVector Fwd = Origin.GetRotation().GetForwardVector();
	const FVector Right = Origin.GetRotation().GetRightVector();

	const FVector Loc = Origin.GetLocation()
				+ Fwd   * (Col * ForwardSpacing)
				+ Right * (Row * RightSpacing);
	
	return FTransform(Origin.GetRotation(), Loc, FVector::OneVector);
}

// Pre-warms Niagara components into the engine pool so subsequent spawns are allocation-free.
// Uses ManualRelease so we can immediately return the component to the pool.
void AVehicleManager::PrimeNiagaraPool(UNiagaraSystem* System, const int32 Count) const
{
	if (System->IsValid() && CurrentWorld.IsValid())
	{
		for (int i = 0; i < Count; ++i)
		{
			if (UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
					CurrentWorld.Get(), System, FVector::ZeroVector, FRotator::ZeroRotator,
					FVector::OneVector, false, false, ENCPoolMethod::ManualRelease))
			{
				Comp->ReleaseToPool();
			}
		}
	}
}

// Called when a spawned vehicle gets destroyed (either via RemoveVehicleById or externally).
// Keeps the GUID map in sync and plays an optional "death" Niagara FX aligned to the ground.
void AVehicleManager::OnVehicleDestroyed(AActor* DestroyedActor)
{
	if (!DestroyedActor) return;

	const ASWPVehicle* Vehicle = Cast<ASWPVehicle>(DestroyedActor);
	if (!Vehicle) return;

	// Notify listeners and drop weak reference from the map.
	OnVehicleRemoved.Broadcast(Vehicle->GetGuid());
	SpawnedVehicles.Remove(Vehicle->GetGuid());

	// Optionally spawn a VFX at the impact/ground location with the normal-aligned rotation.
	if (VehicleDeathFX->IsValid() && CurrentWorld.IsValid())
	{
		FVector  Loc = Vehicle->GetActorLocation();
		FRotator Rot = Vehicle->GetActorRotation();

		FHitResult Hit;
		const FVector Start = Loc + FVector(0,0,50.0f);
		const FVector End   = Loc - FVector(0,0,2000.0f);
		FCollisionQueryParams Params(SCENE_QUERY_STAT(DeathFXTrace), false, const_cast<ASWPVehicle*>(Vehicle));
		if (CurrentWorld->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
		{
			Loc = Hit.Location;
			Rot = FRotationMatrix::MakeFromZ(Hit.Normal).Rotator();
		}

		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			CurrentWorld.Get(),
			VehicleDeathFX,
			Loc,
			Rot,
			ScaleFactorFX,
			false,
			true, ENCPoolMethod::AutoRelease	// fire-and-forget; Niagara pool manages lifetime.
		);
	}
}
