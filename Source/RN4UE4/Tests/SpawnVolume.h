// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "RakNetRP.h"
#include "SpawnVolume.generated.h"

class ARakNetRP;

UCLASS()
class RN4UE4_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	bool		GetActive() const;
	void		SetActive(bool act);
	void		Reset();
	void		RandomSpawn();
	FVector		GetRandomPointInBox() const;
	FVector		GetRandomUnitVector() const;
	UPROPERTY(EditAnywhere, Category = "SpawnerBox")
		UBoxComponent* BoxComponent;
	UPROPERTY(EditAnywhere, Category = "Spawner")
		float			spawnTime;
private:
	bool			active;
	float			currentTime;
	FRandomStream			rand;

	ARakNetRP*		rakNetManager;	
};
