// Copyright (c) [2025] [Federico Grenoville]

#pragma once

struct FSWPSuspensionState
{
	float PreviousCompressionRatio = 0.0f;
	
	float SpringForce = 0.0f;
	float DampingForce = 0.0f;

	float TotalForce = 0.0f;
	FVector Fz = FVector::ZeroVector;
	FVector ForceLocation = FVector::ZeroVector;
};
