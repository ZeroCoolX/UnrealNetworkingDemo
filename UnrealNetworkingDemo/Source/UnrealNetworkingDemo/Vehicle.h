// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Vehicle.generated.h"

UCLASS()
class UNREALNETWORKINGDEMO_API AVehicle : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AVehicle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	// Mass in kg
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	// The maximum force applied to the car when the throttle is full down (N)
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000;	// 10000 / 1000 = 10 m/s

	// the minimum turning radius of the car turning circle at full lock (m/s)
	UPROPERTY()
	float MinTurnRadius = 10.f;

	// Aerodynamics (higher means more resistance) (in kg)
	// TODO: calculate this on the fly - MaxDrivingForce / DesiredSpeed^2 (speed = 60mph or 25 m/s)
	UPROPERTY()
	float DragCoefficient = 16.f;

	// Taken from wikipedia for std rolling resistance on wheels
	UPROPERTY()
	float RollingResistanceCoefficient = 0.015f;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ApplyThrottle(float amount);

	void ApplyThrottle(float amount);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ApplySteering(float amount);

	void ApplySteering(float amount);

	// Meters per second
	FVector Velocity;	
	// Current speed and rotation based off user input
	float Throttle;
	float SteeringThrow;

private:
	FVector GetAirResistance();
	FVector GetRollingResistance();

	void CalculateVelocity(float deltaTime);
	void CalculateRotation(float deltaTime);
	void UpdateLocationFromVelocity(float deltaTime);
};
