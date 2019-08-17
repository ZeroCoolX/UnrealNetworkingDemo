// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicle.h"
#include "Components/InputComponent.h"

// Scalar value for converting Centimeters per second (CPS) to Meters per second (MPS)
const float CMS_TO_MS = 100.f;

// Sets default values
AVehicle::AVehicle()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AVehicle::BeginPlay()
{
	Super::BeginPlay();
	
}

void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AVehicle::ApplyThrottle);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehicle::ApplySteering);
}

void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateVelocity(DeltaTime);
	CalculateRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);
}


void AVehicle::CalculateVelocity(float deltaTime)
{
	// apply acceleration
	FVector force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	FVector acceleration = force / Mass;
	Velocity += (acceleration * deltaTime);
}

void AVehicle::CalculateRotation(float deltaTime)
{
	// The amount the user is pressing to turn * change in time * maximum degrees allowed to turn per second
	float rotationAngle = SteeringThrow * deltaTime * MaxTurnDegreesPerSecond;
	FQuat deltaRotation(FVector::UpVector, FMath::DegreesToRadians(rotationAngle));
	// Update the forward direction after rotation
	Velocity = deltaRotation.RotateVector(Velocity);
	AddActorWorldRotation(deltaRotation);
}

// translation = cm/s
void AVehicle::UpdateLocationFromVelocity(float deltaTime)
{
	// Calculate the change in space converting centimeters per second to meters per second
	FVector translation = Velocity * CMS_TO_MS *  deltaTime;

	FHitResult hit;
	AddActorWorldOffset(translation, true, &hit);
	if (hit.IsValidBlockingHit()) {
		Velocity = FVector::ZeroVector;
	}
}

