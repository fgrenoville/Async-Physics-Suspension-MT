// Copyright (c) [2025] [Federico Grenoville]

#include "SWPDebugDrawCVars.h"
#include "SWPDebugDrawCategory.h"
#include "HAL/IConsoleManager.h"

FSWPDebugDrawSettings GSWP_DebugDrawSettings;
static IConsoleVariable* GCatsVar = nullptr;

FAutoConsoleVariableRef CVarSWP_DebugDrawEnable(TEXT("swp.DebugDraw.Enable"), GSWP_DebugDrawSettings.bEnable,
	TEXT("Enable/Disable SWP debug draw (1/0)."), ECVF_Cheat);
FAutoConsoleVariableRef CVarSWP_DebugDrawCats(TEXT("swp.DebugDraw.Cats"), GSWP_DebugDrawSettings.Cats,
	TEXT("Category list: Suspension, Engine, Transmission, Aero, Tires, Misc."), ECVF_Cheat);
FAutoConsoleVariableRef CVarSWP_DebugDrawDurationScale(TEXT("swp.DebugDraw.DurationScale"),
	GSWP_DebugDrawSettings.DurationMultiplier, TEXT("Scale for debug line duration."), ECVF_Cheat);
FAutoConsoleVariableRef CVarSWP_DebugDrawThicknessScale(TEXT("swp.DebugDraw.ThicknessScale"),
	GSWP_DebugDrawSettings.ThicknessMultiplier, TEXT("Scale for debug line thickness."), ECVF_Cheat);

static uint32 SWP_MaskFromCatsString(const FString& Csv)
{
	TArray<FString> Toks;
	Csv.ParseIntoArray(Toks, TEXT(","), true);
	uint32 M = 0;
	
	for (FString& T : Toks)
	{
		T.TrimStartAndEndInline(); T.ToLowerInline();
		if (T == TEXT("suspension"))		M |= SWP_DebugCatBit(ESWPDebugDrawCategory::Suspension);
		else if (T == TEXT("engine"))		M |= SWP_DebugCatBit(ESWPDebugDrawCategory::Engine);
		else if (T == TEXT("transmission")) M |= SWP_DebugCatBit(ESWPDebugDrawCategory::Transmission);
		else if (T == TEXT("aero"))			M |= SWP_DebugCatBit(ESWPDebugDrawCategory::Aero);
		else if (T == TEXT("tires"))		M |= SWP_DebugCatBit(ESWPDebugDrawCategory::Tires);
		else if (T == TEXT("misc"))			M |= SWP_DebugCatBit(ESWPDebugDrawCategory::Misc);
	}
	
	return M;
}

const FSWPDebugDrawSettings& SWP_GetDebugDrawSettings()
{
	return GSWP_DebugDrawSettings;
}

void SWP_RegisterDebugDrawCVarsCallback()
{
#if !UE_BUILD_SHIPPING
	GCatsVar = IConsoleManager::Get().FindConsoleVariable(TEXT("swp.DebugDraw.Cats"));
	if (GCatsVar)
	{
		GCatsVar->SetOnChangedCallback(FConsoleVariableDelegate::CreateStatic(&SWP_UpdateDebugCatsFromCVars));
	}
	SWP_UpdateDebugCatsFromCVars(nullptr);
#endif
}

void SWP_UnregisterDebugDrawCVarsCallback()
{
#if !UE_BUILD_SHIPPING
	if (GCatsVar)
	{
		GCatsVar->SetOnChangedCallback(FConsoleVariableDelegate());
		GCatsVar = nullptr;
	}
#endif
}

void SWP_UpdateDebugCatsFromCVars(IConsoleVariable* CVar)
{
	if (!GSWP_DebugDrawSettings.Cats.IsEmpty())
		GSWP_DebugDrawSettings.Mask = SWP_MaskFromCatsString(GSWP_DebugDrawSettings.Cats);
	else
		GSWP_DebugDrawSettings.Mask = 0xFFFFFFFF;
}

