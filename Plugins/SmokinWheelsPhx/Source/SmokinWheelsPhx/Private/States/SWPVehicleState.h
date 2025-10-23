// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "SWPSuspensionState.h"

struct FSWPVehicleState
{
	FSWPSuspensionState FrontLeftSuspension;
	FSWPSuspensionState FrontRightSuspension;
	FSWPSuspensionState RearLeftSuspension;
	FSWPSuspensionState RearRightSuspension;
};
