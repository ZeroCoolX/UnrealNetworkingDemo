// Fill out your copyright notice in the Description page of Project Settings.

#include "Vehicle.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"

// Scalar value for converting Centimeters per second (CPS) to Meters per second (MPS)
const float CMS_TO_MS = 100.f;


/*  Debugging only  */
FString GetEnumText(ENetRole role) {
	switch (role) {
	case ROLE_Authority:
		return "ROLE_Authority";
	case ROLE_SimulatedProxy:
		return "ROLE_SimulatedProxy";
	case ROLE_None:
		return "ROLE_None";
	case ROLE_AutonomousProxy:
		return "ROLE_AutonomousProxy";
	default:
		return "ERROR";
	}
}

// Sets default values
AVehicle::AVehicle()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AVehicle::BeginPlay()
{
	Super::BeginPlay();
}

void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Indicates the player controller is on this client
	if (IsLocallyControlled()) {
		FVehicleMove move;
		move.DeltaTime = DeltaTime;
		move.SteeringThrow = SteeringThrow;
		move.Throttle = Throttle;

		Server_SendMove(move);

		SimulateMove(move);
	}

	// TODO: Debugging only
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 100.f), GetEnumText(Role), this, FColor::White, DeltaTime);
}



/* Server Replication */

bool AVehicle::Server_SendMove_Validate(FVehicleMove move) { return true; }
void AVehicle::Server_SendMove_Implementation(FVehicleMove move) {
	SimulateMove(move);

	ServerState.LastMove = move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = Velocity;
}

void AVehicle::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVehicle, ServerState);
}

void AVehicle::OnRep_ServerState() {
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
}



void AVehicle::SimulateMove(FVehicleMove move) {
	CalculateVelocity(move.DeltaTime, move.Throttle);
	CalculateRotation(move.DeltaTime, move.SteeringThrow);
	UpdateLocationFromVelocity(move.DeltaTime);
}


/*  Movement control functions based off player input  */

void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AVehicle::ApplyLocalThrottle);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehicle::ApplyLocalSteering);
}

void AVehicle::ApplyLocalThrottle(float amount) {
	Throttle = amount;
}

void AVehicle::ApplyLocalSteering(float amount) {
	SteeringThrow = amount;
}


/*  Physics Simulations  */

void AVehicle::CalculateVelocity(float deltaTime, float throttle)
{
	// apply acceleration
	FVector force = GetActorForwardVector() * MaxDrivingForce * throttle;
	force += GetAirResistance();
	force += GetRollingResistance();

	FVector acceleration = force / Mass;
	Velocity += (acceleration * deltaTime);
}

FVector AVehicle::GetAirResistance()
{
	return -1.f * Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient;
}

FVector AVehicle::GetRollingResistance()
{
	float accelerationDueToGravity = -1.f * (GetWorld()->GetGravityZ() / CMS_TO_MS);
	float normalForce = Mass * accelerationDueToGravity;
	return -1.f * Velocity.GetSafeNormal() * RollingResistanceCoefficient * normalForce;
}

void AVehicle::CalculateRotation(float deltaTime, float steeringThrow)
{
	float deltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * deltaTime;
	// The smaller the steering turn, the bigger the radius
	float rotationAngle = (deltaLocation / MinTurnRadius) * steeringThrow;
	FQuat deltaRotation(FVector::UpVector, rotationAngle);

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

