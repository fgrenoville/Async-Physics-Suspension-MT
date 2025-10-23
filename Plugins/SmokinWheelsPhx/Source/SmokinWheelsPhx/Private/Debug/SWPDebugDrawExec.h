// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "SWPDebugDrawCommand.h"
#include "DrawDebugHelpers.h"

struct FSWPDebugDrawExec
{
	static void Draw(UWorld* World, const FSWPDebugDrawCommand& DebugDrawCommand, const float Duration, const float Thickness)
	{
#if !UE_BUILD_SHIPPING
		if (!World) return;
		
		switch (DebugDrawCommand.Shape)
		{
		case ESWPDebugDrawShape::Line:
			DrawDebugLine(World, DebugDrawCommand.P0, DebugDrawCommand.P1, DebugDrawCommand.Color,
				DebugDrawCommand.bPersistent, Duration, 0, Thickness);
			break;
			
		case ESWPDebugDrawShape::Arrow:
			DrawDebugDirectionalArrow(World, DebugDrawCommand.P0, DebugDrawCommand.P1,
				DebugDrawCommand.ArrowSize, DebugDrawCommand.Color, DebugDrawCommand.bPersistent,
				Duration, 0, Thickness);
			break;
			
		case ESWPDebugDrawShape::Point:
			DrawDebugPoint(World, DebugDrawCommand.P0, DebugDrawCommand.Radius, DebugDrawCommand.Color,
				DebugDrawCommand.bPersistent, Duration);
			break;
			
		case ESWPDebugDrawShape::Sphere:
			DrawDebugSphere(World, DebugDrawCommand.P0, DebugDrawCommand.Radius, 16, DebugDrawCommand.Color,
				DebugDrawCommand.bPersistent, Duration, 0, Thickness);
			break;
			
		case ESWPDebugDrawShape::Box:
			DrawDebugBox(World, DebugDrawCommand.P0, DebugDrawCommand.Extents,
				DebugDrawCommand.Rot.Quaternion(), DebugDrawCommand.Color, DebugDrawCommand.bPersistent, Duration, 0, Thickness);
			break;
		}
#endif
	}

	static void DrawVehicle(UWorld* World, const FSWPVehicleOut& VehicleOut, const FSWPDebugDrawSettings& DebugDrawSettings)
	{
#if !UE_BUILD_SHIPPING
		if (!World) return;
		if (!DebugDrawSettings.bEnable) return;
		
		for (const auto& DebugDrawCommand : VehicleOut.DebugDrawCommands)
		{
			if (!SWP_IsCategoryEnabled(DebugDrawSettings.Mask, DebugDrawCommand.Category))
				continue;
		
			const float Duration = DebugDrawCommand.Duration * DebugDrawSettings.DurationMultiplier;
			const float Thickness = FMath::Max(0.0f, DebugDrawCommand.Thickness * DebugDrawSettings.ThicknessMultiplier);
			
			Draw(World, DebugDrawCommand, Duration, Thickness);
		}
#endif
	}
};

