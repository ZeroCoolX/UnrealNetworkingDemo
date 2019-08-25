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
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AVehicle::BeginPlay()
{
	Super::BeginPlay();
}

/*  Input Replication  */
void AVehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AVehicle::ApplyLocalThrottle);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVehicle::ApplyLocalSteering);
}

void AVehicle::ApplyLocalThrottle(float amount) {
	Throttle = amount;
	Server_ApplyThrottle(amount);
}

void AVehicle::ApplyLocalSteering(float amount) {
	SteeringThrow = amount;
	Server_ApplySteering(amount);
}

void AVehicle::Server_ApplyThrottle_Implementation(float amount) {
	Throttle = amount;
}
bool AVehicle::Server_ApplyThrottle_Validate(float amount) { return FMath::Abs(amount) <= 1; }

void AVehicle::Server_ApplySteering_Implementation(float amount) {
	SteeringThrow = amount;
}
bool AVehicle::Server_ApplySteering_Validate(float amount) {return FMath::Abs(amount) <= 1;}

void AVehicle::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AVehicle, ReplicatedTransform);
}

void AVehicle::OnRep_ReplicatedTransform() {
	SetActorTransform(ReplicatedTransform);
}

void AVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CalculateVelocity(DeltaTime);
	CalculateRotation(DeltaTime);
	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority()) {
		ReplicatedTransform = GetActorTransform();
	}

	// TODO: Debugging only
	DrawDebugString(GetWorld(), FVector(0.f, 0.f, 100.f), GetEnumText(Role), this, FColor::White, DeltaTime);
}


/*  Movement control functions based off player input  */
void AVehicle::CalculateVelocity(float deltaTime)
{
	// apply acceleration
	FVector force = GetActorForwardVector() * MaxDrivingForce * Throttle;
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

void AVehicle::CalculateRotation(float deltaTime)
{
	float deltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * deltaTime;
	// The smaller the steering turn, the bigger the radius
	float rotationAngle = (deltaLocation / MinTurnRadius) * SteeringThrow;
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

