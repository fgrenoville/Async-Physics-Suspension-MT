// Copyright (c) [2025] [Federico Grenoville]

#include "SWPSuspension.h"

USWPSuspension::USWPSuspension()
{
	PrimaryComponentTick.bCanEverTick = true;

	// --- Default tuning parameters (can be adjusted via UI) ---
	TravelCm = 80.0f;
	SpringStiffness = 35000.0f;
	ShockBump = 4300.0f;
	ShockRebound = 3000.0f;
	MaxForce = 7500.0f;
	
	WheelRadiusCm = 30.0f;
}

void USWPSuspension::BeginPlay()
{
	Super::BeginPlay();
}

void USWPSuspension::TickComponent(float DeltaTime, ELevelTick TickType,
										FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Note: game-thread tick is unused here. All physics happens in PT.
}

