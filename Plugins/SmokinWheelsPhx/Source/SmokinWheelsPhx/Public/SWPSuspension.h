// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SWPSuspension.generated.h"

UCLASS(ClassGroup = (SmokinWheelPhx))
class SMOKINWHEELSPHX_API USWPSuspension : public USceneComponent
{
	GENERATED_BODY()

public:
	/** Max distance (cm) for the downward line trace from the TopLink. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SmokinWheelsPhx|Suspension")
	float TravelCm;
	/** Spring constant (stiffness) for Hooke's law. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SmokinWheelsPhx|Suspension")
	float SpringStiffness;
	/** Damping coefficient when the spring is compressing (bump). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SmokinWheelsPhx|Suspension")
	float ShockBump;
	/** Damping coefficient when the spring is extending (rebound). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SmokinWheelsPhx|Suspension")
	float ShockRebound;
	/** Absolute cap on the force produced by this suspension (safety against spikes). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SmokinWheelsPhx|Suspension")
	float MaxForce;

	/**
	 * Visual/physical wheel radius (cm).
	 * Added to TravelCm to build the trace length so wheel visuals remain decoupled
	 * from suspension travel.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SmokinWheelsPhx|Suspension")
	float WheelRadiusCm;

public:
	USWPSuspension();
	// Tick unused here. All physics happens in PT.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;
protected:
	virtual void BeginPlay() override;
};
