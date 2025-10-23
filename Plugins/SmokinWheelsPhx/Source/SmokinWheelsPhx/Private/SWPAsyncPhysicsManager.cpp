// Copyright (c) [2025] [Federico Grenoville]

#include "SWPAsyncPhysicsManager.h"
#include "PBDRigidsSolver.h"
#include "SWPAsyncCallback.h"
#include "SWPStat.h"
#include "SWPSuspension.h"
#include "SWPVehicle.h"
#include "Configs/SWPVehicleConfig.h"
#include "Debug/SWPDebugDrawCVars.h"
#include "Debug/SWPDebugDrawExec.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

// Show with 'stat SmokinWheelsPhx' in the UE console
DECLARE_CYCLE_STAT(TEXT("SmokinWheelsPhx:ScenePreTick"), STAT_SmokinWheelsPhx_ScenePreTick, STATGROUP_SmokinWheelsPhx);
DECLARE_CYCLE_STAT(TEXT("SmokinWheelsPhx:ScenePostTick"), STAT_SmokinWheelsPhx_ScenePostTick, STATGROUP_SmokinWheelsPhx);

FDelegateHandle FSWPAsyncPhysicsManager::OnPostWorldInitializationHandle;
FDelegateHandle FSWPAsyncPhysicsManager::OnWorldCleanupHandle;

bool FSWPAsyncPhysicsManager::bInitialized  = false;

// One manager per physics scene (no global singleton).
TMap<FPhysScene*, FSWPAsyncPhysicsManager*> FSWPAsyncPhysicsManager::SceneToPhysicsManagerMap;

FSWPAsyncPhysicsManager::FSWPAsyncPhysicsManager(FPhysScene* InPhysScene)
	: PhysScene(*InPhysScene), AsyncObject(nullptr)
{
	if(!bInitialized)
	{
		OnPostWorldInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddStatic(&FSWPAsyncPhysicsManager::OnPostWorldInitialization);
		OnWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddStatic(&FSWPAsyncPhysicsManager::OnWorldCleanup);

		bInitialized = true;
	}

	// Ensure a single manager per FPhysScene.
	ensure(FSWPAsyncPhysicsManager::SceneToPhysicsManagerMap.Find(InPhysScene) == nullptr);

	FSWPAsyncPhysicsManager::SceneToPhysicsManagerMap.Add(InPhysScene, this);
}

FSWPAsyncPhysicsManager::~FSWPAsyncPhysicsManager()
{
	// Proactively flush GT-side vehicle registry (WeakObjectPtrs).
	while (Vehicles.Num() > 0)
	{
		RemoveVehicle(Vehicles.Last());
	}

	// Detach GT delegates and PT sim-callback.
	UnregisterCallbacks();
}

// Called after a UWorld is initialized. We bind per-scene tick & sim callbacks here.
void FSWPAsyncPhysicsManager::OnPostWorldInitialization(UWorld* InWorld, const UWorld::InitializationValues)
{
	FSWPAsyncPhysicsManager* AsyncTickManager = FSWPAsyncPhysicsManager::GetPhysicsManagerFromScene(InWorld->GetPhysicsScene());

	if(AsyncTickManager != nullptr)
	{
		AsyncTickManager->RegisterCallbacks();
	}
}

// Called when a UWorld is being cleaned up. We unbind callbacks to avoid leaks/crashes.
void FSWPAsyncPhysicsManager::OnWorldCleanup(UWorld* InWorld, bool bSessionEnded, bool bCleanupResource)
{
	FSWPAsyncPhysicsManager* AsyncTickManager = FSWPAsyncPhysicsManager::GetPhysicsManagerFromScene(InWorld->GetPhysicsScene());

	if(AsyncTickManager != nullptr)
	{
		AsyncTickManager->UnregisterCallbacks();
	}
}

// Static lookup: map FPhysScene* -> manager (GT-only).
FSWPAsyncPhysicsManager* FSWPAsyncPhysicsManager::GetPhysicsManagerFromScene(FPhysScene* InPhysScene)
{
	FSWPAsyncPhysicsManager* Manager = nullptr;
	FSWPAsyncPhysicsManager** ManagerPtr = FSWPAsyncPhysicsManager::SceneToPhysicsManagerMap.Find(InPhysScene);

	if(ManagerPtr != nullptr)
	{
		Manager = *ManagerPtr;
	}

	return Manager;
}

// Bind GT ticks (pre/post) and register the PT sim-callback with the Chaos solver.
void FSWPAsyncPhysicsManager::RegisterCallbacks()
{
	// GT-side preparation/consumption hooks (called around each physics step).
	OnPhysScenePreTickHandle = PhysScene.OnPhysScenePreTick.AddRaw(this, &FSWPAsyncPhysicsManager::ScenePreTick);
	OnPhysScenePostTickHandle = PhysScene.OnPhysScenePostTick.AddRaw(this, &FSWPAsyncPhysicsManager::ScenePostTick);

	// PT-side callback object (lives on solver, executed in physics thread).
	if(AsyncObject == nullptr)
	{
		AsyncObject = PhysScene.GetSolver()->CreateAndRegisterSimCallbackObject_External<FSWPAsyncCallback>();
	}
}

// Unbind GT tick delegates and free the PT sim-callback on the solver.
void FSWPAsyncPhysicsManager::UnregisterCallbacks()
{
	if (OnPhysScenePreTickHandle.IsValid())
	{
		PhysScene.OnPhysScenePreTick.Remove(OnPhysScenePreTickHandle);
		OnPhysScenePreTickHandle.Reset();
	}

	if (OnPhysScenePostTickHandle.IsValid())
	{
		PhysScene.OnPhysScenePostTick.Remove(OnPhysScenePostTickHandle);
		OnPhysScenePostTickHandle.Reset();
	}
	
	if(AsyncObject != nullptr)
	{
		if (Chaos::FPhysicsSolver* Solver = PhysScene.GetSolver())
		{
			Solver->UnregisterAndFreeSimCallbackObject_External(AsyncObject);
		}
		
		AsyncObject = nullptr;
	}
}

// Remove manager entry for this scene (used on shutdown).
void FSWPAsyncPhysicsManager::DetachFromPhysScene(FPhysScene* InPhysScene)
{
	FSWPAsyncPhysicsManager::SceneToPhysicsManagerMap.Remove(InPhysScene);
}

// GT → PT: Build the input buffer for the next physics step.
// Runs on the Game Thread right before Chaos steps the scene.
void FSWPAsyncPhysicsManager::ScenePreTick(FPhysScene* InPhysScene, float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_SmokinWheelsPhx_ScenePreTick);
	
	if (!AsyncObject) return;
	
	UWorld* World = PhysScene.GetOwningWorld();
	if (!World) return;

	// Acquire a writable input packet for the PT callback (lock-free queue).
	FSWPAsyncCallbackInput* AsyncInput = AsyncObject->GetProducerInputData_External();
	AsyncInput->Reset();

	// NOTE: Passing UWorld to PT is a conscious compromise for now.
	// For a "PT-pure" step, prefer Chaos scene queries instead of UWorld line traces.
	AsyncInput->CurrentWorld = World;
	AsyncInput->VehicleConfigs.Reserve(Vehicles.Num());
	AsyncInput->VehiclesToAdd.Reserve(VehiclesToAdd.Num());
	AsyncInput->VehiclesToRemove.Reserve(VehiclesToRemove.Num());
	//AsyncInput->Timestamp = Timestamp;

	if (World)
	{
		// Snapshot per-vehicle config (POD only). No UObject deref will be done on PT.
		for (TWeakObjectPtr<ASWPVehicle> Vehicle : Vehicles)
		{
			if (!Vehicle.IsValid()) continue;
			
			FSWPVehicleConfig Config
			{
				Vehicle->GetGuid(),										// Gameplay ID (FGuid)
				GetChaosUniqueIdx(Vehicle->GetBodyInstance()),		// Solver particle ID (FUniqueIdx)
				Vehicle,												// Weak reference (GT-only)
				BuildSuspensionCfg(Vehicle->GetFrontLeftSuspension()),
				BuildSuspensionCfg(Vehicle->GetFrontRightSuspension()),
				BuildSuspensionCfg(Vehicle->GetRearLeftSuspension()),
				BuildSuspensionCfg(Vehicle->GetRearRightSuspension())
			};
			
			AsyncInput->VehicleConfigs.Add(Config);
		}
	}

	// Publish add/remove sets (will be applied by PT at the beginning of the step).
	for (FGuid IdToAdd : VehiclesToAdd)
	{
		AsyncInput->VehiclesToAdd.Add(IdToAdd);
	}
	VehiclesToAdd.Reset();
	
	for (FGuid IdToRemove : VehiclesToRemove)
	{
		AsyncInput->VehiclesToRemove.Add(IdToRemove);
	}
	VehiclesToRemove.Reset();

	// Optional frame counter increment (helps when skipping late outputs).
	//++Timestamp;
}

// GT ← PT: Consume all pending output packets produced by the PT callback.
// Called after the physics step; we can safely touch UWorld and draw debug.
void FSWPAsyncPhysicsManager::ScenePostTick(FChaosScene* InChaosScene)
{
	SCOPE_CYCLE_COUNTER(STAT_SmokinWheelsPhx_ScenePostTick);

	if (!AsyncObject) return;
	
	UWorld* World = PhysScene.GetOwningWorld();
	if (!World) return;
	
	// PT may produce more snapshots than GT consumes (PT usually runs faster).
	// Drain the queue to keep debug info fresh and prevent backlog growth.
	//while (Chaos::TSimCallbackOutputHandle<FGripRushAsyncCallbackOutput> OutH = AsyncObject->PopOutputData_External())
	while (true)
	{
		Chaos::TSimCallbackOutputHandle<FSWPAsyncCallbackOutput> OutH = AsyncObject->PopOutputData_External();
		if (!OutH) break;
		
		const FSWPAsyncCallbackOutput* Out =  OutH.Get();
		if (!Out) continue;

		// Debug draw (GT-only): visualize forces, traces, etc.
		for (const FSWPVehicleOut& VehicleOut : Out->VehicleOuts)
		{
			FSWPDebugDrawExec::DrawVehicle(World, VehicleOut, SWP_GetDebugDrawSettings());
		}
	}
}

// Register a new vehicle on GT. Returns its FGuid (used by both GT & PT).
FGuid FSWPAsyncPhysicsManager::AddVehicle(TWeakObjectPtr<ASWPVehicle> Vehicle)
{
	if (Vehicle.IsValid())
	{
		FGuid Guid = FGuid::NewGuid();
			
		Vehicles.Add(Vehicle);
		VehiclesToAdd.Add(Guid);
		
		return Guid;
	}

	return FGuid();		// Invalid
}

// Unregister vehicle on GT. We keep only the GUID for PT-side removal at next step.
void FSWPAsyncPhysicsManager::RemoveVehicle(TWeakObjectPtr<ASWPVehicle> Vehicle)
{
	if (Vehicle.IsValid())
	{
		Vehicles.Remove(Vehicle);
		VehiclesToRemove.Add(Vehicle->GetGuid());
	}
}

// Fetch Chaos particle UniqueIdx from a BodyInstance. Must be called on GT.
Chaos::FUniqueIdx FSWPAsyncPhysicsManager::GetChaosUniqueIdx(const FBodyInstance* BI)
{
	check(IsInGameThread());
	
	if (!BI)
		return Chaos::FUniqueIdx();

	const FPhysicsActorHandle& Handle = BI->ActorHandle;
	if (!Handle)
		return Chaos::FUniqueIdx();

	const Chaos::FRigidBodyHandle_External& GT = Handle->GetGameThreadAPI();
	return GT.UniqueIdx();
}

// Build a POD suspension config from a USWPSuspension component (GT).
// PT will consume this data without touching the component/UObject.
FSWPSuspensionConfig FSWPAsyncPhysicsManager::BuildSuspensionCfg(const USWPSuspension* Suspension)
{
	FSWPSuspensionConfig Config {};

	if (!Suspension) return Config;
	
	Config.AttachLocal = Suspension->GetRelativeTransform();
	
	Config.TravelCm = Suspension->TravelCm;
	Config.SpringStiffness = Suspension->SpringStiffness;
	Config.ShockBump = Suspension->ShockBump;
	Config.ShockRebound = Suspension->ShockRebound;
	Config.MaxForce = Suspension->MaxForce;
	Config.WheelRadiusCm = Suspension->WheelRadiusCm;
	return Config;
}
