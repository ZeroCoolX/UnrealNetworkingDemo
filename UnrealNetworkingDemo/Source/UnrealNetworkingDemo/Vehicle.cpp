// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicle.h"
#include "Components/InputComponent.h"

// Scalar value for converting Centimeters per second (CPS) to Meters per second (MPS)
const float CPS_TO_MPS = 100.f;

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

// Called every frame
void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	FVector acceleration = force / Mass;
	
	// apply acceleration
	Velocity += (acceleration * DeltaTime);

	// Calculate the change in space converting centimeters per second to meters per second
	FVector translationInCentimeters = Velocity * CPS_TO_MPS *  DeltaTime;

	AddActorWorldOffset(translationInCentimeters);
}

// Called to bind functionality to input
void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AVehicle::MoveForward);
}

void AVehicle::MoveForward(float amount)
{
	Throttle = amount;
}

