// Copyright (c) [2025] [Federico Grenoville]

#pragma once

struct FSWPDebugDrawSettings
{
	bool bEnable = true;
	uint32 Mask = 0xFFFFFFFF;
	FString Cats = FString(TEXT(""));
	float DurationMultiplier = 1.0f;
	float ThicknessMultiplier = 1.0f;
};

const FSWPDebugDrawSettings& SWP_GetDebugDrawSettings();

void SWP_RegisterDebugDrawCVarsCallback();
void SWP_UnregisterDebugDrawCVarsCallback();

void SWP_UpdateDebugCatsFromCVars(IConsoleVariable* CVar);
