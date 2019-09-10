// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "BoundaryBox.generated.h"

UCLASS()
class RN4UE4_API ABoundaryBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoundaryBox();
	
	int GetRank() const { return Rank; }
	UBoxComponent* GetBoxComponent() const { return BoxComponent; }

private:

	UPROPERTY(EditAnywhere, Category = "Raknet")
		int Rank;

	UPROPERTY(EditDefaultsOnly, Category = "RakNet")
		UBoxComponent* BoxComponent;
};
