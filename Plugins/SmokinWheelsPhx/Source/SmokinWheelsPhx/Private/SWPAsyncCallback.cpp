// Copyright (c) [2025] [Federico Grenoville]

#include "SWPAsyncCallback.h"
#include "PBDRigidsSolver.h"
#include "SWPPhysicsUtility.h"
#include "SWPStat.h"
#include "Solvers/SWPSuspensionSolver.h"

// Show with 'stat SmokinWheelsPhx' in the UE console
DECLARE_CYCLE_STAT(TEXT("SmokinWheelsPhx:OnPreSimulate_Internal"), STAT_SmokinWheelsPhx_OnPreSimulate_Internal, STATGROUP_SmokinWheelsPhx);
DECLARE_CYCLE_STAT(TEXT("SmokinWheelsPhx:ChaosSingleThread"), STAT_SmokinWheelsPhx_ChaosSingleThread, STATGROUP_SmokinWheelsPhx);
DECLARE_CYCLE_STAT(TEXT("SmokinWheelsPhx:ChaosParallelFor"), STAT_SmokinWheelsPhx_ChaosParallelFor, STATGROUP_SmokinWheelsPhx);

// Runtime toggle: force the per-vehicle step to run single-threaded.
static bool GSWP_ForceSingleThread = false;
FAutoConsoleVariableRef CVarSWP_ForceSingleThread(
	TEXT("swp.ForceSingleThread"),
	GSWP_ForceSingleThread,
	TEXT("If true, force OnPreSimulate method's single-thread execution (1/0)."),
	ECVF_Cheat
);

void FSWPAsyncCallback::OnPreSimulate_Internal()
{
	SCOPE_CYCLE_COUNTER(STAT_SmokinWheelsPhx_OnPreSimulate_Internal);

	// 1) Consume GT input snapshot.
	const FSWPAsyncCallbackInput* AsyncInput = GetConsumerInput_Internal();
	if (!AsyncInput) return;

	// NOTE (PT caveat): Passing UWorld into PT is a conscious compromise right now.
	// Accessing UWorld/UObject from PT is not thread-safe. Prefer Chaos scene queries
	// for ray/shape tests to keep the physics step PT-pure.
	UWorld* World = AsyncInput->CurrentWorld.Get();
	if (!World) return;

	float SimTime = GetDeltaTime_Internal();

	// 2) Apply adds/removes for per-vehicle PT state.
	// These FGuids were queued on GT.
	for (int32 i = 0; i < AsyncInput->VehiclesToAdd.Num(); ++i)
	{
		PhysicsDataVehicles.FindOrAdd(AsyncInput->VehiclesToAdd[i]);
	}
	for (int32 i = 0; i < AsyncInput->VehiclesToRemove.Num(); ++i)
	{
		PhysicsDataVehicles.Remove(AsyncInput->VehiclesToRemove[i]);	
	}

	const int32 NumVehicles = AsyncInput->VehicleConfigs.Num();
	if (NumVehicles == 0) return;
	ensureMsgf(NumVehicles == PhysicsDataVehicles.Num(), TEXT("Physics data mismatch !!"));

	// 3) Solver access & handle discovery.
	Chaos::FPhysicsSolverBase* ChaosBaseSolver = GetSolver();
	if (!ChaosBaseSolver) return;
	
	Chaos::FPhysicsSolver* ChaosSolver = static_cast<Chaos::FPhysicsSolver*>(ChaosBaseSolver);
	if (!ChaosSolver) return;

	// Current snapshot of particle handles. We scan it below to resolve rigid handles by FUniqueIdx.
	const auto& Handles = ChaosSolver->GetParticles().GetParticleHandles();
	const int32 NumHandles = static_cast<int32>(Handles.Size());
	
	// Build a compact array of PT per-vehicle pointers aligned with VehicleConfigs order.
	PhysicsSortedDataVehicles.SetNumUninitialized(NumVehicles);
	for (int32 i = 0; i < NumVehicles; ++i)
	{
		const FSWPVehicleConfig& Config = AsyncInput->VehicleConfigs[i];

		FSWPVehiclePhysicsData* PhysicsData = PhysicsDataVehicles.Find(Config.Guid);
		ensureMsgf(PhysicsData != nullptr, TEXT("Physics data not found !!"));
		if (!PhysicsData) continue;

		PhysicsData->PhysicsIdx = Config.PhysicsIdx;

		// Invalidate cached handle if the UniqueIdx changed (body was recreated/migrated).
		if (PhysicsData->PhysicsHandle && PhysicsData->PhysicsHandle->UniqueIdx() != PhysicsData->PhysicsIdx)
		{
			PhysicsData->PhysicsHandle = nullptr;
		}

		// Resolve rigid handle if missing (PT-safe).
		if (!PhysicsData->PhysicsHandle && PhysicsData->PhysicsIdx.IsValid())
		{
			for (int32 k = 0; k < NumHandles; ++k)
			{
				Chaos::FGeometryParticleHandle* P = Handles.Handle(k).Get();
				if (!P) continue;

				if (P->UniqueIdx() == PhysicsData->PhysicsIdx)
				{
					PhysicsData->PhysicsHandle = P->CastToRigidParticle();
					break;
				}
			}
		}
		
		PhysicsSortedDataVehicles[i] = PhysicsData;
	}

	// Raw pointers for tight inner loop access (no bounds checks in the lambda).
	FSWPVehiclePhysicsData** PhysicsData = PhysicsSortedDataVehicles.GetData();
	const FSWPVehicleConfig* Configs = AsyncInput->VehicleConfigs.GetData();

	// 4) Prepare output packet for GT.
	FSWPAsyncCallbackOutput& AsyncOutput = GetProducerOutputData_Internal();
	AsyncOutput.Reset();
	AsyncOutput.Timestamp = AsyncInput->Timestamp;			// optional versioning (helps skip stale)
	AsyncOutput.VehicleOuts.SetNum(NumVehicles);
	FSWPVehicleOut* Outs = AsyncOutput.VehicleOuts.GetData();

	// 5) Execute per-vehicle step: single-thread or parallel.
	if (GSWP_ForceSingleThread)
	{
		// Single-thread path: useful to compare against the parallel variant.
		SCOPE_CYCLE_COUNTER(STAT_SmokinWheelsPhx_ChaosSingleThread);
		{
			for (int k = 0; k < NumVehicles; ++k)
			{
				FSWPVehiclePhysicsData* VehiclePhysicsData = PhysicsData[k];
				ensureMsgf(VehiclePhysicsData != nullptr, TEXT("Physics data not found in Chaos::PhysicsParallelFor() !!"));
				if (!VehiclePhysicsData) return;

				const FSWPVehicleConfig& VehicleConfig = Configs[k];
				FSWPVehicleState& VehicleSimState = VehiclePhysicsData->SimState;
				
				
				Chaos::FPBDRigidParticleHandle* Chassis = VehiclePhysicsData->PhysicsHandle;
				ensureMsgf(Chassis != nullptr, TEXT("PhysicsHandle not found in Chaos::PhysicsParallelFor() !!"));
				if (!Chassis) return;

				FSWPVehicleOut& VehicleOut = Outs[k];
				VehicleOut.Guid = VehicleConfig.Guid;

				const FTransform ChassisTransformWorld = FTransform(Chassis->GetR(), Chassis->GetX());

				FSWPSuspensionSolver::Compute(World,
					VehicleConfig.VehicleActor,
					ChassisTransformWorld,
					VehicleConfig.FrontLeftSuspension,
					VehicleSimState.FrontLeftSuspension,
					VehicleOut,
					SimTime);
				FSWPSuspensionSolver::Compute(World,
					VehicleConfig.VehicleActor,
					ChassisTransformWorld,
					VehicleConfig.FrontRightSuspension,
					VehicleSimState.FrontRightSuspension,
					VehicleOut,
					SimTime);
				FSWPSuspensionSolver::Compute(World,
					VehicleConfig.VehicleActor,
					ChassisTransformWorld,
					VehicleConfig.RearLeftSuspension,
					VehicleSimState.RearLeftSuspension,
					VehicleOut,
					SimTime);
				FSWPSuspensionSolver::Compute(World,
					VehicleConfig.VehicleActor,
					ChassisTransformWorld,
					VehicleConfig.RearRightSuspension,
					VehicleSimState.RearRightSuspension,
					VehicleOut,
					SimTime);

				// PT-safe force application via Chaos API (no UObjects involved).
				FSWPPhysicsUtility::AddForceAtLocation(Chassis,
					VehicleSimState.FrontLeftSuspension.ForceLocation,
					VehicleSimState.FrontLeftSuspension.Fz * 100.0f);
				FSWPPhysicsUtility::AddForceAtLocation(Chassis,
					VehicleSimState.FrontRightSuspension.ForceLocation,
					VehicleSimState.FrontRightSuspension.Fz * 100.0f);
				FSWPPhysicsUtility::AddForceAtLocation(Chassis,
					VehicleSimState.RearLeftSuspension.ForceLocation,
					VehicleSimState.RearLeftSuspension.Fz * 100.0f);
				FSWPPhysicsUtility::AddForceAtLocation(Chassis,
					VehicleSimState.RearRightSuspension.ForceLocation,
					VehicleSimState.RearRightSuspension.Fz * 100.0f);
			}
		}
	}
	else
	{
		// Parallel path: one iteration per vehicle.
		// Uses Chaos' worker pool and returns only after all iterations complete.
		SCOPE_CYCLE_COUNTER(STAT_SmokinWheelsPhx_ChaosParallelFor);
		{
			Chaos::PhysicsParallelFor(NumVehicles, [World, SimTime, Configs, PhysicsData, Outs, this](int32 i)
			{
				FSWPVehiclePhysicsData* VehiclePhysicsData = PhysicsData[i];
				ensureMsgf(VehiclePhysicsData != nullptr, TEXT("Physics data not found in Chaos::PhysicsParallelFor() !!"));
				if (!VehiclePhysicsData) return;

				const FSWPVehicleConfig& VehicleConfig = Configs[i];
				FSWPVehicleState& VehicleSimState = VehiclePhysicsData->SimState;
				
				
				Chaos::FPBDRigidParticleHandle* Chassis = VehiclePhysicsData->PhysicsHandle;
				ensureMsgf(Chassis != nullptr, TEXT("PhysicsHandle not found in Chaos::PhysicsParallelFor() !!"));
				if (!Chassis) return;

				FSWPVehicleOut& VehicleOut = Outs[i];
				VehicleOut.Guid = VehicleConfig.Guid;

				const FTransform ChassisTransformWorld = FTransform(Chassis->GetR(), Chassis->GetX());

				FSWPSuspensionSolver::Compute(World,
					VehicleConfig.VehicleActor,
					ChassisTransformWorld,
					VehicleConfig.FrontLeftSuspension,
					VehicleSimState.FrontLeftSuspension,
					VehicleOut,
					SimTime);
				FSWPSuspensionSolver::Compute(World,
					VehicleConfig.VehicleActor,
					ChassisTransformWorld,
					VehicleConfig.FrontRightSuspension,
					VehicleSimState.FrontRightSuspension,
					VehicleOut,
					SimTime);
				FSWPSuspensionSolver::Compute(World,
					VehicleConfig.VehicleActor,
					ChassisTransformWorld,
					VehicleConfig.RearLeftSuspension,
					VehicleSimState.RearLeftSuspension,
					VehicleOut,
					SimTime);
				FSWPSuspensionSolver::Compute(World,
					VehicleConfig.VehicleActor,
					ChassisTransformWorld,
					VehicleConfig.RearRightSuspension,
					VehicleSimState.RearRightSuspension,
					VehicleOut,
					SimTime);

				// PT-safe force application via Chaos API (no UObjects involved).
				FSWPPhysicsUtility::AddForceAtLocation(Chassis,
					VehicleSimState.FrontLeftSuspension.ForceLocation,
					VehicleSimState.FrontLeftSuspension.Fz * 100.0f);
				FSWPPhysicsUtility::AddForceAtLocation(Chassis,
					VehicleSimState.FrontRightSuspension.ForceLocation,
					VehicleSimState.FrontRightSuspension.Fz * 100.0f);
				FSWPPhysicsUtility::AddForceAtLocation(Chassis,
					VehicleSimState.RearLeftSuspension.ForceLocation,
					VehicleSimState.RearLeftSuspension.Fz * 100.0f);
				FSWPPhysicsUtility::AddForceAtLocation(Chassis,
					VehicleSimState.RearRightSuspension.ForceLocation,
					VehicleSimState.RearRightSuspension.Fz * 100.0f);
				
			}, false);
		}
	}
}
