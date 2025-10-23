// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "Configs/SWPVehicleConfig.h"
#include "Outs/SWPVehicleOut.h"
#include "States/SWPVehicleState.h"

struct SMOKINWHEELSPHX_API FSWPAsyncCallbackInput : public Chaos::FSimCallbackInput
{
	int32 Timestamp = INDEX_NONE;
	TWeakObjectPtr<UWorld> CurrentWorld;
	
	TArray<FSWPVehicleConfig> VehicleConfigs;
	TArray<FGuid> VehiclesToAdd;	
	TArray<FGuid> VehiclesToRemove;	
	
	void Reset()
	{
		CurrentWorld.Reset();
		VehicleConfigs.Reset();

		VehiclesToAdd.Reset();
		VehiclesToRemove.Reset();
	}
};

struct SMOKINWHEELSPHX_API FSWPAsyncCallbackOutput : public Chaos::FSimCallbackOutput
{
	int32 Timestamp = INDEX_NONE;

	TArray<FSWPVehicleOut> VehicleOuts;
	
	void Reset()
	{
		VehicleOuts.Reset();
	}
};

struct SMOKINWHEELSPHX_API FSWPVehiclePhysicsData
{
	FSWPVehicleState SimState;

	Chaos::FUniqueIdx PhysicsIdx;
	Chaos::FPBDRigidParticleHandle* PhysicsHandle = nullptr;
};

/**
 * FSWPAsyncCallback (PT side)
 * 
 * Chaos TSimCallbackObject implementation that runs on the Physics Thread (PT).
 * Responsibilities:
 *  - Consume per-step input produced on GT (FSimCallbackInput).
 *  - Maintain per-vehicle PT state (PhysicsDataVehicles).
 *  - Run per-vehicle simulation (suspension compute + force application).
 *  - Produce per-step output for GT (FSimCallbackOutput).
 *
 * Threading contract:
 *  - This callback executes on the PT. Do NOT dereference UObjects here.
 *  - Only consume POD/config/handles prepared by GT; only write to PT-owned state
 *    and to the output packet that GT will read in ScenePostTick().
 */
class SMOKINWHEELSPHX_API FSWPAsyncCallback : public Chaos::TSimCallbackObject<FSWPAsyncCallbackInput, FSWPAsyncCallbackOutput>
{
	TMap<FGuid, FSWPVehiclePhysicsData> PhysicsDataVehicles;
	TArray<FSWPVehiclePhysicsData*> PhysicsSortedDataVehicles;
	
	virtual void OnPreSimulate_Internal() override;
};
