// Copyright (c) [2025] [Federico Grenoville]

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SWPVehicle.generated.h"

class USWPSuspension;

UCLASS(Blueprintable)
class SMOKINWHEELSPHX_API ASWPVehicle : public APawn
{
	GENERATED_BODY()

public:
	ASWPVehicle();
	virtual void Tick(float DeltaTime) override;

	FORCEINLINE FGuid GetGuid() const { return Guid; }
	FORCEINLINE const FBodyInstance* GetBodyInstance() const
	{
		if (IsValid(BodyMeshComponent))
			return BodyMeshComponent->GetBodyInstance();
		else
			return  nullptr;
	}
		
	FORCEINLINE USWPSuspension* GetFrontLeftSuspension() const { return FrontLeftSuspension; }
	FORCEINLINE USWPSuspension* GetFrontRightSuspension() const { return FrontRightSuspension; }
	FORCEINLINE USWPSuspension* GetRearLeftSuspension() const { return RearLeftSuspension; }
	FORCEINLINE USWPSuspension* GetRearRightSuspension() const { return RearRightSuspension; }
	
	// virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FGuid Guid;

	UPROPERTY(EditDefaultsOnly, Category = "SmokinWheelsPhx|Chassis")
	float VehicleMass;
	
	/** Chassis mesh acting as the vehicle's rigid body (receives forces from suspensions). */
	UPROPERTY(VisibleAnywhere, Category = "SmokinWheelsPhx|Components")
	TObjectPtr<UStaticMeshComponent> BodyMeshComponent;	
	
	/** Front-left suspension component. */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "SmokinWheelsPhx|Components")
	TObjectPtr<USWPSuspension> FrontLeftSuspension;
	/** Front-right suspension component. */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "SmokinWheelsPhx|Components")
	TObjectPtr<USWPSuspension> FrontRightSuspension;
	/** Rear-left suspension component. */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "SmokinWheelsPhx|Components")
	TObjectPtr<USWPSuspension> RearLeftSuspension;
	/** Rear-right suspension component. */
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = "SmokinWheelsPhx|Components")
	TObjectPtr<USWPSuspension> RearRightSuspension;
};
