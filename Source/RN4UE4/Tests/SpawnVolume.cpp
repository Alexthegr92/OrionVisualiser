// Fill out your copyright notice in the Description page of Project Settings.

#include "RN4UE4.h"
#include "SpawnVolume.h"
#include "Rand.h"
#include "EngineUtils.h"
#include "../RakNetRP.h"

ASpawnVolume::ASpawnVolume()
{
	PrimaryActorTick.bCanEverTick = true;
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
}

void ASpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	active = true;
	rand.GenerateNewSeed();
	ensureMsgf(rakNetManager, TEXT("Unexpected null rakNetManager!"));
}

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

bool ASpawnVolume::GetActive()
{
	return active;
}

void ASpawnVolume::SetActive(bool act)
{
	active = act;
}

FVector ASpawnVolume::GetRandomPointInBox()
{
	FVector pos = GetActorLocation();
	FVector extents = BoxComponent->GetUnscaledBoxExtent();
	pos.X = rand.RandRange(pos.X - extents.X, pos.X + extents.X);
	pos.Y = rand.RandRange(pos.Z - extents.Z, pos.Z + extents.Z);
	pos.Z = rand.RandRange(pos.Y - extents.Y, pos.Y + extents.Y);
	pos = pos / 50.0f;
	return pos;
}

FVector ASpawnVolume::GetRandomUnitVector()
{
	return  rand.GetUnitVector();
}

void ASpawnVolume::Reset()
{
	currentTime = 0.0f;
}

void ASpawnVolume::RandomSpawn()
{
	FVector pos = GetRandomPointInBox();
	FVector dir = GetRandomUnitVector();
	rakNetManager->RPrpcSpawn(pos, dir);
}
