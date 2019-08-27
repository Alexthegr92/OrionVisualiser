// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RakNetTypes.h"
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SignalBoundariesToServer(const RakNet::SystemAddress address);
	void RPrpcSignalBoundaryBox(const TArray<FVector> pos, const TArray<FVector> size, const TArray<int> ranks, const RakNet::SystemAddress address);

	bool CheckServersNumber();

	bool CheckBoxesHaveDifferentRanks();

	UPROPERTY(EditAnywhere, Category = "Raknet")
		bool createCustomBoundariesBoxes;

private:
	ARakNetRP*		rakNetManager;
	bool boundariesSent = false;
};
