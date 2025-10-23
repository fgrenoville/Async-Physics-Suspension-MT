// Copyright (c) [2025] [Federico Grenoville]

#include "SWPVehicle.h"
#include "SWPAsyncPhysicsManager.h"
#include "SWPSuspension.h"

ASWPVehicle::ASWPVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	Guid = FGuid();
	VehicleMass = 1'500.0f;
	
	// --- Vehicle composition: chassis + four suspensions ---
	BodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = BodyMeshComponent;

	FrontLeftSuspension = CreateDefaultSubobject<USWPSuspension>("FrontLeftSuspension");
	FrontLeftSuspension->SetupAttachment(BodyMeshComponent);

	FrontRightSuspension = CreateDefaultSubobject<USWPSuspension>("FrontRightSuspension");
	FrontRightSuspension->SetupAttachment(BodyMeshComponent);

	RearLeftSuspension = CreateDefaultSubobject<USWPSuspension>("RearLeftSuspension");
	RearLeftSuspension->SetupAttachment(BodyMeshComponent);

	RearRightSuspension = CreateDefaultSubobject<USWPSuspension>("RearRightSuspension");
	RearRightSuspension->SetupAttachment(BodyMeshComponent);
}

void ASWPVehicle::BeginPlay()
{
	Super::BeginPlay();

	if (GetWorld())
	{
		if (FPhysScene* PhysScene = GetWorld()->GetPhysicsScene())
		{
			if (FSWPAsyncPhysicsManager* PhysManager = FSWPAsyncPhysicsManager::GetPhysicsManagerFromScene(PhysScene))
				Guid = PhysManager->AddVehicle(this);
		}
	}

	// --- Chassis rigid body setup (mass & simulation flags) ---
	BodyMeshComponent->SetMassOverrideInKg(NAME_None, VehicleMass);
	BodyMeshComponent->SetSimulatePhysics(true);
	BodyMeshComponent->SetEnableGravity(true);
}

void ASWPVehicle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GetWorld())
	{
		if (FPhysScene* PhysScene = GetWorld()->GetPhysicsScene())
		{
			if (FSWPAsyncPhysicsManager* PhysManager = FSWPAsyncPhysicsManager::GetPhysicsManagerFromScene(PhysScene))
				PhysManager->RemoveVehicle(this);
		}
	}
	
	Super::EndPlay(EndPlayReason);
}

void ASWPVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Ensure the rigid body stays awake during the simulation.
	BodyMeshComponent->WakeRigidBody();
}

// Called to bind functionality to input
// void ASWPVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
// {
// 	Super::SetupPlayerInputComponent(PlayerInputComponent);
// }

