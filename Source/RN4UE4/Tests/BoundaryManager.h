// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoundaryManager.generated.h"

class ABoundaryBox;
class ARakNetRP;
UCLASS()
class RN4UE4_API ABoundaryManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoundaryManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SignalBoundariesToServer();

	bool CheckServersNumber();

	bool CheckBoxesHaveDifferentRanks();

	UPROPERTY(EditAnywhere, Category = "Raknet")
		ARakNetRP*		rakNetManager;
	UPROPERTY(EditAnywhere, Category = "Raknet")
		bool multiAuras;
	UPROPERTY(EditAnywhere, Category = "Raknet")
		float errorTolerance;
	UPROPERTY(EditAnywhere, Category = "Raknet")
		bool createCustomBoundariesBoxes;

private:
	bool boundariesSent = false;
};
