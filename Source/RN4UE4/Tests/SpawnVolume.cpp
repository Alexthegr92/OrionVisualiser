// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "SpawnVolume.h"
#include "Rand.h"
#include "EngineUtils.h"
#include "../RakNetRP.h"

// Sets default values
ASpawnVolume::ASpawnVolume()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	active = true;
	rand.GenerateNewSeed();
	ensureMsgf(rakNetManager, TEXT("Unexpected null rakNetManager!"));
}

// Called every frame
void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ensure(rakNetManager))
	{
		if (active && rakNetManager->getAllServersChecked())
		{
			currentTime += DeltaTime;
			if (currentTime >= spawnTime)
			{
				RandomSpawn();
				currentTime = 0.0f;
			}
		}
	}
}

void ASpawnVolume::SetUp()
{

}

ARakNetRP* ASpawnVolume::getRakNetManager()
{
	return rakNetManager;
}

void ASpawnVolume::setRakNetManager(ARakNetRP* raknet)
{
	rakNetManager = raknet;
}

bool ASpawnVolume::getActive()
{
	return active;
}

void ASpawnVolume::setActive(bool act)
{
	active = act;
}

FVector ASpawnVolume::getRandomPointInBox()
{
	FVector pos = GetActorLocation();
	FVector extents = BoxComponent->GetUnscaledBoxExtent();
	pos.X = rand.RandRange(pos.X - extents.X, pos.X + extents.X);
	pos.Y = rand.RandRange(pos.Z - extents.Z, pos.Z + extents.Z);
	pos.Z = rand.RandRange(pos.Y - extents.Y, pos.Y + extents.Y);
	pos = pos / 50.0f;
	return pos;
}

FVector ASpawnVolume::getRandomUnitVector()
{
	return  rand.GetUnitVector();
}

void ASpawnVolume::reset()
{
	currentTime = 0.0f;
}

void ASpawnVolume::RandomSpawn()
{
	FVector pos = getRandomPointInBox();
	FVector dir = getRandomUnitVector();
	rakNetManager->RPrpcSpawn(pos, dir);
}
