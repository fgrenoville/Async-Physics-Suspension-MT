// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VehicleManager.generated.h"

class UNiagaraSystem;
class ASWPVehicle;

/**
 * AVehicleManager
 * 
 * Game-Thread utility actor that owns vehicle spawning/destruction, lightweight bookkeeping,
 * optional possession, and a small Niagara FX. Vehicles are identified by FGuid
 * (consistent with the GT ↔ PT pattern used in the physics plugin).
 *
 * Threading/Lifecycle notes:
 * - All methods here are GT-only (spawn, possession, delegates, Niagara). Do not call from PT.
 * - Vehicles are tracked via TWeakObjectPtr to avoid keeping destroyed actors alive.
 * - Destruction path: we listen to ASWPVehicle::OnDestroyed to keep the map in sync and to play FX.
 */
UCLASS()
class ASYNCPHXSUSPENSIONMT_API AVehicleManager : public AActor
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleSpawned, const FGuid, VehicleId);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVehicleRemoved, const FGuid, VehicleId);

	UPROPERTY(BlueprintAssignable)
	FOnVehicleSpawned OnVehicleSpawned;
	UPROPERTY(BlueprintAssignable)
	FOnVehicleRemoved OnVehicleRemoved;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AsyncPhxSuspensionMT|Refs")
	TSubclassOf<ASWPVehicle> SWPVehicleClass;

	// Optional VFX shown on vehicle destruction.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT|VFX")
	UNiagaraSystem* VehicleDeathFX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AsyncPhxSuspensionMT|VFX")
	FVector ScaleFactorFX;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AsyncPhxSuspensionMT|Spawn Config")
	int32 SpawnNumMaxRows;
	// cm along "forward" of SpawnStartingAt.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AsyncPhxSuspensionMT|Spawn Config")
	float SpawnForwardSpacing;
	// cm along "right" of SpawnStartingAt.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AsyncPhxSuspensionMT|Spawn Config")
	float SpawnRightSpacing;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AsyncPhxSuspensionMT|Spawn Config")
	FTransform SpawnStartingAt;

public:
	AVehicleManager();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "AsyncPhxSuspensionMT")
	FGuid SpawnVehicleAtTransform(const FTransform& SpawnTransform, const bool bPossess = false);
	UFUNCTION(BlueprintCallable, Category = "AsyncPhxSuspensionMT")
	FGuid SpawnVehicle(const bool bPossess = false);

	UFUNCTION(BlueprintCallable, Category = "AsyncPhxSuspensionMT")
	void RemoveVehicleById(const FGuid VehicleId);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AsyncPhxSuspensionMT")
	void GetAllVehicleIds(TArray<FGuid>& OutKeys) const
	{
		return SpawnedVehicles.GenerateKeyArray(OutKeys);
	}
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TWeakObjectPtr<UWorld> CurrentWorld;
	
	UPROPERTY()
	TMap<FGuid, TWeakObjectPtr<ASWPVehicle>> SpawnedVehicles;

	int32 SpawnIndex;
	
private:
	FTransform GetNewSpawnLocation(int32 Index, int32 NumMaxRows, float ForwardSpacing, float RightSpacing,
		const FTransform& Origin);

	void PrimeNiagaraPool(UNiagaraSystem* System, const int32 Count) const;
	
	UFUNCTION()
	void OnVehicleDestroyed(AActor* DestroyedActor);
};
