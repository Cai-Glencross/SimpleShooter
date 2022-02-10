// Fill out your copyright notice in the Description page of Project Settings.


#include "Gun.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "DrawDebugHelpers.h"



// Sets default values
AGun::AGun()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Mesh"));

	GunMesh->SetupAttachment(Root);

	MuzzleFlashParticle = CreateDefaultSubobject<UParticleSystem>(TEXT("Muzzle Particles"));
}

// Called when the game starts or when spawned
void AGun::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGun::PullTrigger() 
{
	UGameplayStatics::SpawnSoundAttached(MuzzleSound, GunMesh, TEXT("MuzzleFlashSocket"));
	UGameplayStatics::SpawnEmitterAttached(MuzzleFlashParticle, GunMesh, TEXT("MuzzleFlashSocket"));

	FHitResult BulletHitResult;
	FVector ShotDirection;
	bool bSuccess = GunTrace(BulletHitResult, ShotDirection);
	if (bSuccess) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), RicochetParticle, BulletHitResult.Location, ShotDirection.Rotation(), true);
		AActor* Target = BulletHitResult.GetActor();
		if (Target) {
			FPointDamageEvent DamageEvent(Damage, BulletHitResult, ShotDirection, nullptr);
			AController* OwnerController = GetOwnerController();
			Target->TakeDamage(Damage, DamageEvent, OwnerController, this);
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ImpactSound, BulletHitResult.Location);
		}
	}
}



bool AGun::GunTrace(FHitResult& OutHit, FVector& OutShotDirection) 
{
		
	AController* OwnerController = GetOwnerController();
	if (!OwnerController) return false;

	FVector ViewpointLocation;
	FRotator ViewpointRotation;
	OwnerController->GetPlayerViewPoint(ViewpointLocation, ViewpointRotation);
	OutShotDirection = -ViewpointRotation.Vector();

	FVector End = ViewpointLocation + (ViewpointRotation.Vector() * MaxRange);
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetOwner());

	return GetWorld()->LineTraceSingleByChannel(
		OutHit, 
		ViewpointLocation, 
		End, 
		ECollisionChannel::ECC_GameTraceChannel1, 
		Params
	);
	
}

AController* AGun::GetOwnerController() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return nullptr;
	AController* OwnerController = OwnerPawn->GetController();

	return OwnerController;
}

