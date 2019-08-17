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

	// the number of maximum degrees per second at max throttle
	UPROPERTY()
	float MaxTurnDegreesPerSecond = 90.f;

	// Meters per second
	FVector Velocity;	
	// Current speed and rotation based off user input
	float Throttle;
	float SteeringThrow;

private:
	inline void ApplyThrottle(float amount) { Throttle = amount; }
	inline void ApplySteering(float amount) { SteeringThrow = amount; }

	void CalculateVelocity(float deltaTime);
	void CalculateRotation(float deltaTime);
	void UpdateLocationFromVelocity(float deltaTime);
};
