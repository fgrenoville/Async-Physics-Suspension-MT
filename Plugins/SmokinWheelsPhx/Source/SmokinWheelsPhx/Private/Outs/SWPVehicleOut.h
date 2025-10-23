// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "Debug/SWPDebugDrawCommand.h"

struct FSWPVehicleOut
{
	FGuid Guid;

	static constexpr int32 MaxDebugPerVehicle = 64;
	TArray<FSWPDebugDrawCommand, TInlineAllocator<MaxDebugPerVehicle>> DebugDrawCommands;

	FORCEINLINE void ClearDebugDrawCommands()
	{
#if !UE_BUILD_SHIPPING
		DebugDrawCommands.Reset();
#endif
	}

	FORCEINLINE void AddDebugDrawCommand(const FSWPDebugDrawCommand& Cmd)
	{
#if !UE_BUILD_SHIPPING
		if (DebugDrawCommands.Num() < MaxDebugPerVehicle)
			DebugDrawCommands.Emplace(Cmd);
#endif
	}
};
