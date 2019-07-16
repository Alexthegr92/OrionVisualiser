// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoundaryBox.generated.h"

class ARakNetRP;
UCLASS()
class RN4UE4_API ABoundaryBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoundaryBox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Raknet")
		ARakNetRP*        rakNetManager;

	UPROPERTY(EditAnywhere, Category = "Raknet")
		int       rank;

	UPROPERTY(EditDefaultsOnly, Category = "RakNet")
		UBoxComponent* BoxComponent;

	void SetBoundary();
private:
	bool	sent;
};
