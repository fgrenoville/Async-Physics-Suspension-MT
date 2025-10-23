// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "SWPSuspensionConfig.h"

struct FSWPVehicleConfig
{
	FGuid Guid;
	Chaos::FUniqueIdx PhysicsIdx;

	TWeakObjectPtr<AActor> VehicleActor;
		
	FSWPSuspensionConfig FrontLeftSuspension;
	FSWPSuspensionConfig FrontRightSuspension;
	FSWPSuspensionConfig RearLeftSuspension;
	FSWPSuspensionConfig RearRightSuspension;	
};
