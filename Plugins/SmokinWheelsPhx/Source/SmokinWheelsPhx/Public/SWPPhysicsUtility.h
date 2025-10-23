// Copyright (c) [2025] [Federico Grenoville]

#pragma once

class SMOKINWHEELSPHX_API FSWPPhysicsUtility
{
public:
	/**
	 * Apply a world-space force at a world-space location on a rigid body.
	 * - RigidHandle: Chaos rigid handle (PT-side, never dereference UObjects here).
	 * - Location:    world-space point where the force is applied (e.g., contact/suspension mount).
	 * - Force:       world-space force vector to apply.
	 */
	static void AddForceAtLocation(Chaos::FPBDRigidParticleHandle* RigidHandle, const FVector Location, const FVector Force);
};
