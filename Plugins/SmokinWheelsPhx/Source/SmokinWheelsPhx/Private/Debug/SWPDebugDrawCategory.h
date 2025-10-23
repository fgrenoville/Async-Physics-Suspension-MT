// Copyright (c) [2025] [Federico Grenoville]

#pragma once

enum class ESWPDebugDrawCategory : uint8
{
	Suspension = 0,
	Engine = 1,
	Transmission = 2,
	Aero = 3,
	Tires = 4,
	Misc = 5
};

FORCEINLINE uint32 SWP_DebugCatBit(ESWPDebugDrawCategory Cat)
{
	return 1u << static_cast<uint8>(Cat);
}

FORCEINLINE bool SWP_IsCategoryEnabled(uint32 Mask, ESWPDebugDrawCategory Cat)
{
	return (Mask & SWP_DebugCatBit(Cat)) != 0;
}