// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "RakNetTypes.h"
#include "BoundaryManager.generated.h"

class ARakNetRP;

UCLASS()
class RN4UE4_API ABoundaryManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ABoundaryManager();
	
	void Tick(float DeltaTime) override;

private:
	void SignalBoundariesToServer(RakNet::SystemAddress address) const;
	void RPrpcSignalBoundaryBox(const TArray<FVector>& pos, const TArray<FVector>& size, const TArray<int>& ranks, RakNet::SystemAddress address) const;

	bool CheckServersNumber() const;

	bool CheckBoxesHaveDifferentRanks() const;

	UPROPERTY(EditAnywhere, Category = "Raknet")
		bool CreateCustomBoundariesBoxes;
	
	ARakNetRP*		RakNetManager;
	bool BoundariesChecked = false;
};
