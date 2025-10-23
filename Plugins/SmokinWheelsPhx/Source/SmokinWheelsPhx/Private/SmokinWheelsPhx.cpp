// Copyright Epic Games, Inc. All Rights Reserved.

#include "SmokinWheelsPhx.h"
#include "SWPAsyncPhysicsManager.h"

#define LOCTEXT_NAMESPACE "FSmokinWheelsPhxModule"

void FSmokinWheelsPhxModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	OnPhysSceneInitHandle = FPhysicsDelegates::OnPhysSceneInit.AddRaw(this, &FSmokinWheelsPhxModule::PhysSceneInit);
	OnPhysSceneTermHandle = FPhysicsDelegates::OnPhysSceneTerm.AddRaw(this, &FSmokinWheelsPhxModule::PhysSceneTerm);
}

void FSmokinWheelsPhxModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FPhysicsDelegates::OnPhysSceneInit.Remove(OnPhysSceneInitHandle);
	FPhysicsDelegates::OnPhysSceneTerm.Remove(OnPhysSceneTermHandle);
}

void FSmokinWheelsPhxModule::PhysSceneInit(FPhysScene* Scene)
{
	new FSWPAsyncPhysicsManager(Scene);
}

void FSmokinWheelsPhxModule::PhysSceneTerm(FPhysScene* Scene)
{
	FSWPAsyncPhysicsManager* PhysManager = FSWPAsyncPhysicsManager::GetPhysicsManagerFromScene(Scene);

	if(PhysManager != nullptr)
	{
		PhysManager->DetachFromPhysScene(Scene);
		delete PhysManager;
		PhysManager = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSmokinWheelsPhxModule, SmokinWheelsPhx)