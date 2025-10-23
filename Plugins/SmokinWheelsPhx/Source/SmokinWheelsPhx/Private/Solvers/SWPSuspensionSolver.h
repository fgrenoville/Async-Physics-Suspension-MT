// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "Configs/SWPSuspensionConfig.h"
#include "States/SWPSuspensionState.h"

struct FSWPSuspensionSolver
{
	static FORCEINLINE void Compute(UWorld* World,
									const TWeakObjectPtr<AActor> VehicleActor,
									const FTransform& VehicleAsyncTransformWorld,
									const FSWPSuspensionConfig& SuspensionConfig,
									FSWPSuspensionState& SuspensionState,
									FSWPVehicleOut& VehicleOut,
									float PhysicsDeltaTime)
	{
		// Compute suspension transform in world space.
		const FTransform AsyncWorld = SuspensionConfig.AttachLocal * VehicleAsyncTransformWorld;
		const FQuat AsyncRotation = AsyncWorld.GetRotation();

		FVector AsyncLocation = AsyncWorld.GetLocation();
		FVector AsyncUp = AsyncRotation.GetUpVector();

		// Setup trace parameters to detect ground under the wheel.
		FCollisionQueryParams TraceParams;
		TraceParams.MobilityType = EQueryMobilityType::Any;
		TraceParams.bReturnPhysicalMaterial = false;
		if (VehicleActor.IsValid())
			TraceParams.AddIgnoredActor(VehicleActor.Get());
		TraceParams.bTraceComplex = true;

		SuspensionState.ForceLocation = AsyncLocation;

		// Line trace along suspension axis (downwards).
		FVector StartTrace = AsyncLocation;
		FVector EndTrace = StartTrace - AsyncUp * (SuspensionConfig.TravelCm + SuspensionConfig.WheelRadiusCm);

		// Debug: draw suspension ray.
		VehicleOut.AddDebugDrawCommand(FSWPDebugDrawCommand::MakeLine(StartTrace, EndTrace,
			FColor::Magenta, 2.0f, 0.0f, ESWPDebugDrawCategory::Suspension));

		FHitResult HitResult;
		
		// --- Suspension engaged only if ground contact is found ---
		if (bool bHasContact = World->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams))
		{
			const FVector TraceVector = HitResult.ImpactPoint - StartTrace;

			// Distance along suspension axis.
			const float HitDistanceInSuspensionAxis = FVector::DotProduct(TraceVector, -AsyncUp);

			// Compression ratio ∈ [0,1]
			const float CompressionRatio = FMath::Clamp(1.0f - (HitDistanceInSuspensionAxis / (SuspensionConfig.TravelCm + SuspensionConfig.WheelRadiusCm)), 0.0f, 1.0f);
			const float CompressionVelocity = (CompressionRatio - SuspensionState.PreviousCompressionRatio) / PhysicsDeltaTime;
			
			const float SpringForce = CompressionRatio * SuspensionConfig.SpringStiffness;

			// Damping: separate coefficients for bump vs rebound.
			const float DampingForce = CompressionVelocity > 0.0f ?
				CompressionVelocity * SuspensionConfig.ShockBump :				// Compression
					 CompressionVelocity * SuspensionConfig.ShockRebound;		// Extension


			// Combine spring + damping, clamp to max.
			const float TotalForce = FMath::Clamp(SpringForce + DampingForce, 0.0f, SuspensionConfig.MaxForce);
			// Vertical suspension force.
			const FVector Fz = TotalForce * AsyncUp;
			// Project along contact normal to eliminate unwanted lateral/forward components.
			const FVector Fn = FMath::Max(0.0f, FVector::DotProduct(Fz, HitResult.ImpactNormal)) * HitResult.ImpactNormal;

			// Update suspension state for next iteration.
			SuspensionState.PreviousCompressionRatio = CompressionRatio;
			SuspensionState.SpringForce = SpringForce;
			SuspensionState.DampingForce = DampingForce;
			SuspensionState.Fz = Fn;

			// Debug: draw applied force vector.
			VehicleOut.AddDebugDrawCommand(FSWPDebugDrawCommand::MakeArrow(SuspensionState.ForceLocation,
				SuspensionState.ForceLocation + SuspensionState.Fz * 0.02f, 20.0f, FColor::Emerald,
				2.5f, 0, ESWPDebugDrawCategory::Engine));
		}
		else
		{
			// No ground contact --> reset to zero.
			SuspensionState.PreviousCompressionRatio = 0.0f;
			SuspensionState.SpringForce = 0.0f;
			SuspensionState.DampingForce = 0.0f;
			SuspensionState.Fz = FVector::ZeroVector;
		}
	}
};
