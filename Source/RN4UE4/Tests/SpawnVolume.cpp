// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnVolume.h"
#include "Rand.h"
#include "EngineUtils.h"
#include "../RakNetRP.h"
#include "RN4UE4GameInstance.h"

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
}

void ASpawnVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!active) return;

	if (rakNetManager == nullptr)
	{
		URN4UE4GameInstance* GameInstance = static_cast<URN4UE4GameInstance*>(GetGameInstance());
		ensureMsgf(GameInstance != nullptr, TEXT("RakNetRP - GameInstance is not of type URN4UE4GameInstance"));
		rakNetManager = GameInstance->GetRakNetManager();
	}

	if (ensure(rakNetManager) && rakNetManager->GetAllServersChecked())
	{
		currentTime += DeltaTime;
		if (currentTime >= spawnTime)
		{
			RandomSpawn();
			currentTime = 0.0f;
		}
	}
}

bool ASpawnVolume::GetActive() const
{
	return active;
}

void ASpawnVolume::SetActive(bool act)
{
	active = act;
}

FVector ASpawnVolume::GetRandomPointInBox() const
{
	FVector extents = BoxComponent->GetUnscaledBoxExtent();
	FVector randomPointInBox = FVector(
		rand.RandRange(-extents.X, extents.X),
		rand.RandRange(-extents.Y, extents.Y),
		rand.RandRange(-extents.Z, extents.Z)
	);

	return BoxComponent->GetComponentTransform().TransformPosition(randomPointInBox);
}

FVector ASpawnVolume::GetRandomUnitVector() const
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

	// Convert to Samples space
	pos = pos / 50.0f;	
	float tempY = pos.Y;
	pos.Y = pos.Z;
	pos.Z = tempY;

	// Random direction. N.B. Does not get converted to Samples Space, but doesn't need to be
	FVector dir = GetRandomUnitVector();
	rakNetManager->RPrpcSpawn(pos, dir);
}
