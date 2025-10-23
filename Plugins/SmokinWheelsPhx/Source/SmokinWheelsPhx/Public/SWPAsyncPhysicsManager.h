// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "Configs/SWPSuspensionConfig.h"

class FSWPAsyncCallback;
class USWPSuspension;
class ASWPVehicle;

/**
 * FSWPAsyncPhysicsManager
 * 
 * Game-Thread (GT) manager that owns:
 *  - Per-physics-scene lifecycle (register/unregister of Chaos callbacks)
 *  - The GT<->PT data exchange (Producer input / Consumer output buffers)
 *  - A per-vehicle registry (WeakObjectPtrs on GT; simulation state lives on PT)
 *
 * Design notes:
 *  - One manager per FPhysScene (no global singleton). See SceneToPhysicsManagerMap.
 *  - GT writes Input buffers; PT writes Output buffers. No UObject deref on PT.
 *  - Vehicles are referenced on GT via TWeakObjectPtr (lifecycle-safe). PT uses
 *    POD/handles only (FGuid / Chaos::FUniqueIdx / rigid handles).
 *  - Debug drawing happens on GT in ScenePostTick(), consuming PT outputs.
 */
class SMOKINWHEELSPHX_API FSWPAsyncPhysicsManager
{
public:
	FSWPAsyncPhysicsManager(FPhysScene* InPhysScene);
	~FSWPAsyncPhysicsManager();

	void RegisterCallbacks();
	void UnregisterCallbacks();

	void DetachFromPhysScene(FPhysScene* InPhysScene);
	
	static FSWPAsyncPhysicsManager* GetPhysicsManagerFromScene(FPhysScene* PhysScene);
	
	static void OnPostWorldInitialization(UWorld* InWorld, const UWorld::InitializationValues);
	static void OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResource);

	void ScenePreTick(FPhysScene* InPhysScene, float DeltaTime);
	void ScenePostTick(FChaosScene* InChaosScene);

	FGuid AddVehicle(TWeakObjectPtr<ASWPVehicle> Vehicle);
	void RemoveVehicle(TWeakObjectPtr<ASWPVehicle> Vehicle);

	static Chaos::FUniqueIdx GetChaosUniqueIdx(const FBodyInstance* BI);
	static FSWPSuspensionConfig BuildSuspensionCfg(const USWPSuspension* Suspension);
	
private:
	static bool bInitialized;
	static TMap<FPhysScene*, FSWPAsyncPhysicsManager*> SceneToPhysicsManagerMap;
	
	FPhysScene_Chaos& PhysScene;
	
	FDelegateHandle OnPhysScenePreTickHandle;
	FDelegateHandle OnPhysScenePostTickHandle;

	static FDelegateHandle OnPostWorldInitializationHandle;
	static FDelegateHandle OnWorldCleanupHandle;

	FSWPAsyncCallback* AsyncObject;

	TArray<TWeakObjectPtr<ASWPVehicle>> Vehicles;
	TArray<FGuid> VehiclesToAdd;	
	TArray<FGuid> VehiclesToRemove;	
};
