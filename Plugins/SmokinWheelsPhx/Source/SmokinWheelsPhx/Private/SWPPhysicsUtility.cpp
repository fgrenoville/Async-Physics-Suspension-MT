// Copyright (c) [2025] [Federico Grenoville]

#include "SWPPhysicsUtility.h"

void FSWPPhysicsUtility::AddForceAtLocation(Chaos::FPBDRigidParticleHandle* RigidHandle,
												const FVector Location, const FVector Force)
{
	if (!RigidHandle) return;

	// World-space Center of Mass (not the pivot). Using CoM gives the correct torque arm.
	const Chaos::FVec3 CoM = RigidHandle->XCom();

	// r = Location - CoM (both in world space). τ = r × F.
	const Chaos::FVec3 Torque = Chaos::FVec3::CrossProduct(Location - CoM, Force);

	// Apply force and torque for this step.
	// The boolean flag uses Chaos' API semantics; 'false' here means we add a regular force/torque
	// to be integrated by the solver (not a mass-independent accel/impulse mode).
	RigidHandle->AddForce(Force, false);
	RigidHandle->AddTorque(Torque, false);
}
